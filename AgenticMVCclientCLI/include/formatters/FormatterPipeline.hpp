#pragma once
#include "core/IPayloadFormatter.hpp"
#include <vector>
#include <memory>

class FormatterPipeline {
public:
    void addFormatter(std::unique_ptr<IPayloadFormatter> formatter);
    std::string runFormatters(const std::string& payload);
private:
    std::vector<std::unique_ptr<IPayloadFormatter>> m_formatters;
};
