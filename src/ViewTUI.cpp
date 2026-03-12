#include "ViewTUI.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <thread>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <chrono>

using namespace ftxui;

AgenticDashboardView::AgenticDashboardView(AgenticPipelineModel& model) : m_model(model) {}

Element AgenticDashboardView::renderOrchestratorView() {
    static std::string nodesJson = "[]";
    static auto lastFetch = std::chrono::steady_clock::now();
    
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastFetch).count() >= 2) {
        httplib::Client cli("http://127.0.0.1:9000");
        if (auto res = cli.Get("/api/orchestrator/nodes")) {
            nodesJson = res->body;
        }
        lastFetch = now;
    }

    try {
        nlohmann::json data = nlohmann::json::parse(nodesJson);
        Elements nodeElements;
        for (auto& n : data) {
            bool online = n.value("status", "OFFLINE") == "ONLINE";
            nodeElements.push_back(hbox({
                text(online ? " ● " : " ○ ") | color(online ? Color::Green : Color::Red),
                text(n.value("id", "Unknown")) | bold,
                filler(),
                text(n.value("url", "")) | dim,
                filler(),
                text("[" + n.value("state", "N/A") + "]") | color(Color::Cyan)
            }));
        }
        return vbox({
            text(" AgenticMVC Global Orchestrator ") | bold | center | bgcolor(Color::Blue),
            vbox(std::move(nodeElements)) | frame | border | flex
        });
    } catch (...) {
        return text("Error parsing orchestrator telemetry") | color(Color::Red);
    }
}

void AgenticDashboardView::run() {
    auto screen = ScreenInteractive::Fullscreen();
    int tabSelected = 0;
    std::vector<std::string> tabEntries = {"Agent Switchboard", "Global Orchestrator"};
    auto tabToggle = Menu(&tabEntries, &tabSelected, MenuOption::Horizontal());

    auto switchboard = Renderer([&] {
        auto state = m_model.getState();
        auto messages = m_model.getRecentMessages(20);
        auto alerts = m_model.getRecentAlerts(10);

        Color stateColor = Color::Green;
        std::string stateName = state->getName();
        std::string stateText = "STATUS: AGENTIC_" + stateName;
        
        if (stateName == "WARNING") stateColor = Color::Yellow;
        else if (stateName == "LOCKDOWN") stateColor = Color::Red;

        Elements alertElements;
        for (auto it = alerts.rbegin(); it != alerts.rend(); ++it) {
            alertElements.push_back(vbox({
                text("!! " + it->type + " !!") | color(Color::Red),
                text(" Originating Agent: " + it->agentId) | dim,
                separator() | color(Color::GrayDark)
            }));
        }

        Elements msgElements;
        for (auto it = messages.rbegin(); it != messages.rend(); ++it) {
            msgElements.push_back(hbox({
                text("[" + it->senderId + "] ") | color(Color::Yellow),
                text(it->content)
            }));
        }

        return vbox({
            hbox({
                text(" AgenticMVC Pipeline Switchboard ") | bold,
                filler(),
                text(" " + stateText + " ") | bold | bgcolor(stateColor) | color(Color::Black)
            }) | bgcolor(Color::Blue),
            separator(),
            hbox({
                vbox({ text(" AGENT SECURITY ALERTS ") | underlined | hcenter, vbox(alertElements) | flex | border | color(Color::Red) }) | size(WIDTH, EQUAL, 35),
                vbox({ text(" AGENTIC PIPELINE TRAFFIC ") | underlined | hcenter, vbox(msgElements) }) | flex_grow | border
            }) | flex
        });
    });

    auto orchestratorView = Renderer([&] { return renderOrchestratorView(); });

    auto mainContainer = Container::Tab({
        switchboard,
        orchestratorView
    }, &tabSelected);

    auto mainRenderer = Renderer(mainContainer, [&] {
        return vbox({
            tabToggle->Render(),
            separator(),
            mainContainer->Render() | flex
        });
    });

    auto finalComponent = CatchEvent(mainRenderer, [&](Event event) {
        if (event == Event::Character('1')) { tabSelected = 0; return true; }
        if (event == Event::Character('2')) { tabSelected = 1; return true; }
        return mainRenderer->OnEvent(event);
    });

    std::thread refresher([&] { 
        while (true) { 
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
            screen.Post(Event::Custom); 
        } 
    });
    refresher.detach();
    screen.Loop(finalComponent);
}
