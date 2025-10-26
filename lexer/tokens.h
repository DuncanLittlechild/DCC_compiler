//
// Created by dunca on 26/10/2025.
//

#ifndef DCC_TOKENS_H
#define DCC_TOKENS_H
#include <regex>
#include <variant>
#include <string>
#include <array>

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
    struct Constant { std::string value; };

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

    template <typename T>
    inline Token tokenFactory(const T& type) {
        return Token{type};
    }

    template <typename T>
    inline Token tokenFactory(const T& type, const std::smatch& val) {
        T tmp {val[0]};
        return Token{tmp};
    }

    using regexLookup = std::pair<std::regex, std::function<Token(std::smatch)>>;
    // Keywords must be lower down the array than patterns for this to work
    static const std::array<regexLookup, 10> patterns {
        {
            {std::regex("^[a-zA-Z_]\\w*\\b"), [](const auto& m) { return tokenFactory(Identifier{}, m); }},
            {std::regex("^[0-9]+\\b"),        [](const auto& m) { return tokenFactory(Constant{}, m); }},
            {std::regex("^int\\b"),           [](const auto&)   { return tokenFactory(Int{}); }},
            {std::regex("^void\\b"),          [](const auto&)   { return tokenFactory(Void{}); }},
            {std::regex("^return\\b"),        [](const auto&)   { return tokenFactory(Return{});}},
            {std::regex("^\\("),              [](const auto&)   { return tokenFactory(OpenParen{}); }},
            {std::regex("^\\)"),              [](const auto&)   { return tokenFactory(CloseParen{});}},
            {std::regex("^{"),                [](const auto&)   { return tokenFactory(OpenBrace{}); }},
            {std::regex("^}"),                [](const auto&)   { return tokenFactory(CloseBrace{});}},
            {std::regex("^;"),                [](const auto&)   { return tokenFactory(Semicolon{});}}
        }};
}
#endif //DCC_TOKENS_H