#include "formatters/FormatterPipeline.hpp"

void FormatterPipeline::addFormatter(std::unique_ptr<IPayloadFormatter> formatter) {
    m_formatters.push_back(std::move(formatter));
}

std::string FormatterPipeline::runFormatters(const std::string& payload) {
    std::string result = payload;
    for (auto& formatter : m_formatters) {
        result = formatter->format(result);
    }
    return result;
}
