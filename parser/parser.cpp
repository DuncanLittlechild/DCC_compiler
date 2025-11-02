//
// Created by dunca on 01/11/2025.
//

#include "ast.h"
#include "../lexer/tokens.h"

// Implements recursive descent parsing
namespace Parser {

	//class to iterate over the vector of tokens
	class VectorAndIterator {
	using VectorPointer = std::unique_ptr<std::vector<Token::Token>>;
	private:
		VectorPointer m_vectorRef;
		int m_index {0};
	public:
		explicit VectorAndIterator(VectorPointer vec) : m_vectorRef(std::move(vec)) {};

		int index() const { return m_index; }
		void setIndex(int index) { m_index = index; }

		VectorAndIterator& operator++() {
			if (m_index < m_vectorRef->size()) {
				++m_index;
			} else {
				throw std::out_of_range("VectorAndIterator::operator++ going out of range");
			}
			return *this;
		}

		VectorAndIterator& operator--() {
			if (m_index > 0) {
				--m_index;
			} else {
				throw std::out_of_range("VectorAndIterator::operator-- going out of range");
			}
			return *this;
		}

		VectorAndIterator& operator+=(int add) {
			if (m_index + add < m_vectorRef->size()) {
				m_index += add;
			} else {
				throw std::out_of_range("VectorAndIterator::operator+= going out of range");
			}
			return *this;
		}

		VectorAndIterator& operator-=(int sub) {
			if (m_index - sub > 0) {
				m_index -= sub;
			} else {
				throw std::out_of_range("VectorAndIterator::operator-= going out of range");
			}
			m_index -= sub;
			return *this;
		}

		Token::Token& operator[](const int index) const {
			return (*m_vectorRef)[index];
		}

		Token::Token& takeCurrent() {
			Token::Token& tmp{(*m_vectorRef)[m_index]};
			++m_index;
			return tmp;
		}
	};

	Token::Token& expect(auto&& expected, VectorAndIterator& tokens) {
		Token::Token& actual {tokens.takeCurrent()};
		if (actual != expected) {
			std::string error = "Parser::expect found unexpected token " + Visitor::getStructName(actual) +
								" at index " + std::to_string(tokens.index());
			throw std::invalid_argument(error);
		}
		return actual;
	}

	// Parse Integer values and return a pointer
	std::unique_ptr<Ast::IntConstant> parseInt (Token::Token& token) {
		// Get the value stored in the token
		int tokenValue {std::get<Token::Constant>(token.type).value};

		// Make it a unique pointer and return it
		return std::make_unique<Ast::IntConstant>(tokenValue);
	}

	auto parseExp(VectorAndIterator& tokens) {
		//Check that token contains Token::Constant - if not, throw an error
		// If it does, return the current token
		auto& constantToken {expect(Token::Token{Token::Constant{}}, tokens)};

		// Return a unique pointer to an IntConstant Object
		return parseInt(constantToken);
	}

	auto parseStatement(VectorAndIterator& tokens) {
		// Check that token contains Token::Return - if not, throw an error
		expect(Token::Token{Token::Return{}}, tokens);

		// Get the return value
		auto returnValue {parseExp(tokens)};

		// Check the statement ends with a semicolon token
		expect(Token::Token{Token::Semicolon{}}, tokens);

		// Make a unique pointer to a returnKeyword object
		auto returnPtr {std::make_unique<Ast::ReturnKeyword>(std::move(returnValue))};

		// return a unique pointer to a ReturnKeyword object
		return returnPtr;
	}

	auto parseIdentifier(VectorAndIterator& tokens) {
		// Check that the token is an identifier
		auto& id {expect(Token::Token{Token::Identifier{}}, tokens)};

		// Get the identifier string
		std::string& identifier {std::get<Token::Identifier>(id.type).name};

		// Make a unique pointer to an identifier object
		auto idPtr {std::make_unique<Ast::Identifier>(identifier)};

		//return a pointer to an identifier object
		return idPtr;
	}

	auto parseFunction(VectorAndIterator& tokens) {
		// Check return value
		expect(Token::Token{Token::Int{}}, tokens);

		// Check Identifier
		auto identifier {parseIdentifier(tokens)};

		expect(Token::Token{Token::OpenParen{}}, tokens);
		expect(Token::Token{Token::Void{}}, tokens);
		expect(Token::Token{Token::CloseParen{}}, tokens);
		expect(Token::Token{Token::OpenBrace{}}, tokens);

		// Get a unique pointer to the statement body
		auto statementBody {parseStatement(tokens)};

		expect(Token::Token{Token::CloseBrace{}}, tokens);

		auto function {std::make_unique<Ast::Function>(identifier, statementBody)};

		return Ast::Function {identifier, parseStatement(tokens)};
	}

	Ast::Program parseProgram(VectorAndIterator& tokens) {
		return Ast::Program {parseFunction(tokens)};
	}
}