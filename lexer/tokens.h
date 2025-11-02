//
// Created by dunca on 26/10/2025.
//

#ifndef DCC_TOKENS_H
#define DCC_TOKENS_H
#include <regex>
#include <variant>
#include <string>
#include <array>
#include <typeinfo>

namespace Token {
    // Empty structs
    struct Base {
        virtual bool operator==(const Base& rhs) const {
            return typeid(*this) == typeid(rhs);
        }

        virtual bool operator!=(const Base& rhs) const {
            return !(*this == rhs);
        }

        virtual ~Base() = default;
    };

    // Keywords
    struct Return : Base {};

    // Keyword Types
    struct Int : Base {};

    struct Void : Base {};

    // Punctuation
    struct OpenParen : Base {};

    struct CloseParen : Base {};

    struct OpenBrace : Base {};

    struct CloseBrace : Base {};

    struct Semicolon : Base {};

    // Non-empty structs
    // Function/ variable identifier
    struct Identifier : Base {
        std::string name{};

        Identifier() = default;
        explicit Identifier(const std::string& name) : name{name} {}
    };

    // Integer constant
    struct Constant : Base {
        int value{};

        Constant() = default;
        explicit Constant(int value): value{value}{};
    };

    // Main token struct
    // Wrapper for a std::variant containing token types
    struct Token {
        std::variant<
            Return,
            Int, Void,
            OpenParen, CloseParen, OpenBrace, CloseBrace, Semicolon,
            Identifier, Constant
        > type;

        friend bool operator==(const Token& lhs, const Token& rhs) {
            return lhs.type == rhs.type;
        }
    };

    template <typename T>
    inline Token tokenFactory(const T& type) {
        return Token{type};
    }

    inline Token tokenFactory(const Identifier& type, const std::smatch& val) {
        Identifier tmp {val[0]};
        return Token{tmp};
    }

    inline Token tokenFactory(const Constant& type, const std::smatch& val) {
        Constant tmp {std::stoi(val[0])};
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
            {std::regex("^\\{"),              [](const auto&)   { return tokenFactory(OpenBrace{}); }},
            {std::regex("^\\}"),              [](const auto&)   { return tokenFactory(CloseBrace{});}},
            {std::regex("^;"),                [](const auto&)   { return tokenFactory(Semicolon{});}}
        }};
}

// Visitors for std::variant
namespace Visitor {
    // Overloaded helper to expose the operator() of lambdas
    template<class... Ts>
    struct Overloaded : Ts... {
        using Ts::operator()...;
    };
    template<class... Ts>
    Overloaded(Ts...) -> Overloaded<Ts...>;

    inline std::string getStructName(Token::Token& token) {
        return std::visit(Overloaded{
            [](Token::Return& ret)     { return "Return";},
            [](Token::Void& ret)       { return "Void";},
            [](Token::Int& ret)        { return "Int";},
            [](Token::OpenParen& ret)  { return "OpenParen";},
            [](Token::CloseParen& ret) { return "CloseParen";},
            [](Token::OpenBrace& ret)  { return "OpenBrace";},
            [](Token::CloseBrace& ret) { return "CloseBrace";},
            [](Token::Semicolon& ret)  { return "Semicolon";},
            [](Token::Identifier& ret) { return "Identifier";},
            [](Token::Constant& ret)   { return "Constant";}
        }, token.type);
    }

}
#endif //DCC_TOKENS_H