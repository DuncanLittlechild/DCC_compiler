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

#include "../helpers/overload.h"

namespace Token {
    // Precedences for binary operators
    constexpr int ADDPRECEDENCE      {45};
    constexpr int SUBTRACTPRECEDENCE {45};
    constexpr int MULTIPLYPRECEDENCE {50};
    constexpr int DIVIDEPRECEDENCE   {50};
    constexpr int MODULOPRECEDENCE   {50};

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
    inline bool isKeyword(const std::string& keyword) {
        return std::find(keywordStringPtrs.begin(), keywordStringPtrs.end(), &keyword) != keywordStringPtrs.end();
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

    // Binary operators
    // Mixin class to give the precedence member
    struct Binop { const int precedence; };
    struct Add      : Base, Binop { Add()      : Binop{ADDPRECEDENCE} {}};
    struct Divide   : Base, Binop { Divide()   : Binop{DIVIDEPRECEDENCE} {}};
    struct Multiply : Base, Binop { Multiply() : Binop{MULTIPLYPRECEDENCE} {}};
    struct Modulo   : Base, Binop { Modulo()   : Binop{MODULOPRECEDENCE}{}};

    static constexpr std::string addString     {"+"};
    static constexpr std::string divideString  {"/"};
    static constexpr std::string multiplyString{"*"};
    static constexpr std::string moduloString  {"%"};


    // Unary operators
    // Note that negate can also be a binary operator, depending on context
    struct Negate : Base, Binop { Negate() : Binop{SUBTRACTPRECEDENCE}{}};
    struct Decrement : Base {};
    struct Bitwisenot : Base {};

    static constexpr std::string negateString {"-"};
    static constexpr std::string decrementString {"--"};
    static constexpr std::string bitwisenotString {"~"};

    constexpr std::array<const std::string*, 2> unaryOperatorStringPtrs {&negateString, &bitwisenotString};
    inline bool isUnop(const std::string& unop){
        return std::find(unaryOperatorStringPtrs.begin(), unaryOperatorStringPtrs.end(), &unop) != unaryOperatorStringPtrs.end();
    }

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
            Return, Int, Void,
            OpenParen, CloseParen, OpenBrace, CloseBrace, Semicolon,
            Add, Multiply, Divide, Modulo,
            Negate, Decrement, Bitwisenot,
            Identifier, Constant
        > type;

        friend bool operator==(const Token& lhs, const Token& rhs) {
            return lhs.type == rhs.type;
        }
    };

    // Alias to shorten or statements
    template<typename T>
    constexpr bool isBinopT =
                    std::is_same_v<T, Add>
                    || std::is_same_v<T, Negate>
                    || std::is_same_v<T, Divide>
                    || std::is_same_v<T, Multiply>
                    || std::is_same_v<T, Modulo>;

    inline bool isBinop(const Token& tok) {
        return std::visit([](auto& arg) -> bool {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (isBinopT<T>) {
                return true;
            } else {
                return false;
            }
        }, tok.type);
    }

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
    static const std::array<regexLookup, 17> patterns {
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
            {std::regex("^~"),                [](const auto&)   { return tokenFactory(Bitwisenot{}); }},
            {std::regex("^\\+"),                [](const auto&)   { return tokenFactory(Add{}); }},
            {std::regex("^/"),                [](const auto&)   { return tokenFactory(Divide{}); }},
            {std::regex("^\\*"),                [](const auto&)   { return tokenFactory(Multiply{}); }},
            {std::regex("^%"),                [](const auto&)   { return tokenFactory(Modulo{}); }}
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
    inline const std::string& getTokenName(const Token::Token& token) {
        return std::visit(Overloaded{
            [](const Token::Return& ret) -> const std::string&     { return Token::returnString; },
            [](const Token::Void& ret) -> const std::string&       { return Token::voidString; },
            [](const Token::Int& ret) -> const std::string&        { return Token::intString; },
            [](const Token::OpenParen& ret) -> const std::string&  { return Token::openParenString; },
            [](const Token::CloseParen& ret) -> const std::string& { return Token::closeParenString; },
            [](const Token::OpenBrace& ret) -> const std::string&  { return Token::openBraceString; },
            [](const Token::CloseBrace& ret) -> const std::string& { return Token::closeBraceString;},
            [](const Token::Semicolon& ret) -> const std::string&  { return Token::semicolonString; },
            [](const Token::Identifier& ret) -> const std::string& { return Token::identifierString; },
            [](const Token::Constant& ret) -> const std::string&   { return Token::constantString; },
            [](const Token::Decrement& ret) -> const std::string&  { return Token::decrementString; },
            [](const Token::Negate& ret) -> const std::string&     { return Token::negateString; },
            [](const Token::Bitwisenot& ret) -> const std::string& { return Token::bitwisenotString; },
            [](const Token::Add& ret) -> const std::string&        { return Token::addString; },
            [](const Token::Divide& ret) -> const std::string&     { return Token::divideString; },
            [](const Token::Multiply& ret) -> const std::string&   { return Token::multiplyString; },
            [](const Token::Modulo& ret) -> const std::string&     { return Token::moduloString; },
        }, token.type);
    }

}

#endif //DCC_TOKENS_H


