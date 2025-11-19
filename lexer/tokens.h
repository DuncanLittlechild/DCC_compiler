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
    static constexpr std::string returnString {"return"};
    struct Int : Base {};
    static constexpr std::string intString {"int"};
    struct Void : Base {};
    static constexpr std::string voidString {"void"};
    // Array of keyword types to iterate over
    // Update when add new keyword
    constexpr std::array<const std::string*, 3> keywordStringPtrs {&returnString, &intString, &voidString};
    // Helper to identify if something is a keyword
    bool isKeyword(const std::string& keyword) {
        return !(std::find(keywordStringPtrs.begin(), keywordStringPtrs.end(), &keyword) == keywordStringPtrs.end());
    }

    // Punctuation
    struct OpenParen : Base {};
    static constexpr std::string openParenString {"("};
    struct CloseParen : Base {};
    static constexpr std::string closeParenString {")"};
    struct OpenBrace : Base {};
    static constexpr std::string openBraceString {"{"};
    struct CloseBrace : Base {};
    static constexpr std::string closeBraceString {"}"};
    struct Semicolon : Base {};
    static constexpr std::string semicolonString {";"};

    // Unary operators
    struct Negate : Base {};
    static constexpr std::string negateString {"-"};
    struct Decrement : Base {};
    static constexpr std::string decrementString {"--"};
    struct Bitwisenot : Base {};
    static constexpr std::string bitwisenotString {"~"};

    // Non-empty structs
    // Function/ variable identifier
    struct Identifier : Base {
        std::string name{};

        Identifier() = default;
        explicit Identifier(const std::string& name) : name{name} {}
    };
    static constexpr std::string identifierString {"identifier"};

    // Integer constant
    struct Constant : Base {
        int value{};

        Constant() = default;
        explicit Constant(int value): value{value}{};
    };
    static constexpr std::string constantString {"constant"};

    // Main token struct
    // Wrapper for a std::variant containing token types
    struct Token {
        std::variant<
            Return,
            Int, Void,
            OpenParen, CloseParen, OpenBrace, CloseBrace, Semicolon,
            Negate, Decrement, Bitwisenot,
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
    // Update when add new token
    static const std::array<regexLookup, 12> patterns {
        {
            {std::regex("^[a-zA-Z_]\\w*\\b"), [](const auto& m) { return tokenFactory(Identifier{}, m); }},
            {std::regex("^[0-9]+\\b"),        [](const auto& m) { return tokenFactory(Constant{}, m); }},
            {std::regex("^int\\b"),           [](const auto&)   { return tokenFactory(Int{}); }},
            {std::regex("^void\\b"),          [](const auto&)   { return tokenFactory(Void{}); }},
            {std::regex("^return\\b"),        [](const auto&)   { return tokenFactory(Return{}); }},
            {std::regex("^\\("),              [](const auto&)   { return tokenFactory(OpenParen{}); }},
            {std::regex("^\\)"),              [](const auto&)   { return tokenFactory(CloseParen{}); }},
            {std::regex("^\\{"),              [](const auto&)   { return tokenFactory(OpenBrace{}); }},
            {std::regex("^\\}"),              [](const auto&)   { return tokenFactory(CloseBrace{}); }},
            {std::regex("^;"),                [](const auto&)   { return tokenFactory(Semicolon{}); }},
            {std::regex("^--"),               [](const auto&)   { return tokenFactory(Decrement{}); }},
            {std::regex("^-"),                [](const auto&)   { return tokenFactory(Negate{}); }},
            {std::regex("^~"),                [](const auto&)   { return tokenFactory(Bitwisenot{}); }}
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

    // function to get the name of the struct held in a Token::Token std::variant
    // Update when add new token
    inline const std::string& getStructName(Token::Token& token) {
        return std::visit(Overloaded{
            [](Token::Return& ret) -> const std::string&     { return Token::returnString; },
            [](Token::Void& ret) -> const std::string&       { return Token::voidString; },
            [](Token::Int& ret) -> const std::string&        { return Token::intString; },
            [](Token::OpenParen& ret) -> const std::string&  { return Token::openParenString; },
            [](Token::CloseParen& ret) -> const std::string& { return Token::closeParenString; },
            [](Token::OpenBrace& ret) -> const std::string&  { return Token::openBraceString; },
            [](Token::CloseBrace& ret) -> const std::string& { return Token::closeBraceString;},
            [](Token::Semicolon& ret) -> const std::string&  { return Token::semicolonString; },
            [](Token::Identifier& ret) -> const std::string& { return Token::identifierString; },
            [](Token::Constant& ret) -> const std::string&   { return Token::constantString; },
            [](Token::Decrement& ret) -> const std::string&  { return Token::decrementString; },
            [](Token::Negate& ret) -> const std::string&     { return Token::negateString; },
            [](Token::Bitwisenot& ret) -> const std::string& { return Token::bitwisenotString; }
        }, token.type);
    }

}

#endif //DCC_TOKENS_H
