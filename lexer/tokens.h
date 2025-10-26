//
// Created by dunca on 26/10/2025.
//

#ifndef DCC_TOKENS_H
#define DCC_TOKENS_H
#include <variant>
#include <string>

namespace Token {
    // Empty structs
    // Keywords
    struct Return {};

    // Keyword Types
    struct Int {};
    struct Void{};

    // Punctuation
    struct OpenParen {};
    struct CloseParen {};
    struct OpenBrace {};
    struct CloseBrace {};
    struct Semicolon {};

    // Non-empty structs
    struct Identifier { std::string name; };
    struct Constant { int value; };

    // Main token struct
    // Wrapper for a std::variant containing token types
    struct Token {
        std::variant<
            Return,
            Int, Void,
            OpenParen, CloseParen, OpenBrace, CloseBrace, Semicolon,
            Identifier, Constant
        > type;
    };
}
#endif //DCC_TOKENS_H