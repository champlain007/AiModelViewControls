#pragma once
#include "Model.hpp"
#include <ftxui/dom/elements.hpp>

class AgenticDashboardView {
public:
    AgenticDashboardView(AgenticPipelineModel& model);
    void run(); // Main TUI event loop
    ftxui::Element renderOrchestratorView();

private:
    AgenticPipelineModel& m_model;
};
