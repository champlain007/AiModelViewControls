#pragma once
#include "core/IPayloadFormatter.hpp"
#include <string>

class UserMarkdownFormatter : public IPayloadFormatter {
public:
    std::string format(const std::string& payload) override;
};
