//
// Created by dunca on 27/10/2025.
//

#ifndef DCC_LEXER_H
#define DCC_LEXER_H

#include <regex>
#include <string>
#include <filesystem>
#include "tokens.h"

namespace Lexer {
    static const std::regex initialWhitespace("^\\s+");

    // Reduce size of line after regex match
    void shrinkLine(std::string& line, std::smatch& match);

    // Iterate over file token by token, generating a vector of tokens as it goes.
    std::vector<Token::Token> lexFile(const std::filesystem::path& inputFile);
}
#endif //DCC_LEXER_H