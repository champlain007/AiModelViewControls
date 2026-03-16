#include "formatters/UserMarkdownFormatter.hpp"
#include <regex>

std::string UserMarkdownFormatter::format(const std::string& payload) {
    std::string processed = payload;
    
    // Safely translate user's '*' highlighting to explicit XML tags for the model
    std::regex starRe("\\*([^\\*]+)\\*");
    processed = std::regex_replace(processed, starRe, "<user_emphasis>$1</user_emphasis>");
    
    // Safely translate user's '/' highlighting to explicit XML tags
    std::regex slashRe("/([^/]+)/");
    processed = std::regex_replace(processed, slashRe, "<user_highlight>$1</user_highlight>");
    
    return processed;
}
