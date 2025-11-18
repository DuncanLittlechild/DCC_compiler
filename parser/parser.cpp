//
// Created by dunca on 01/11/2025.
//

#include "parser.h"
#include <type_traits>

// Implements recursive descent parsing
namespace Parser {

	//class to iterate over the vector of tokens
	class VectorAndIterator {
	private:
		std::vector<Token::Token>& m_vectorRef;
		int m_index {0};
	public:
		explicit VectorAndIterator(std::vector<Token::Token>& vec) : m_vectorRef(vec) {};

		int index() const { return m_index; }
		void setIndex(int index) { m_index = index; }

		const std::vector<Token::Token>& vectorRef() const { return m_vectorRef; }

		int size() const { return static_cast<int>(std::ssize(m_vectorRef)); }

		VectorAndIterator& operator++() {
			if (m_index < m_vectorRef.size()) {
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
			if (m_index + add < m_vectorRef.size()) {
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
			return m_vectorRef[index];
		}

		Token::Token& peekCurrent() const {
			return m_vectorRef[m_index];
		}

		Token::Token& takeCurrent() {
			Token::Token& tmp{m_vectorRef[m_index]};
			++m_index;
			return tmp;
		}
	};

	Token::Token& expect(auto& expected, VectorAndIterator& tokens) {
		Token::Token& actual {tokens.takeCurrent()};
		if (Visitor::getStructName(actual) != expected) {
			std::string error = "Parser::expect found unexpected token " + Visitor::getStructName(actual) +
								" at index " + std::to_string(tokens.index());
			throw std::invalid_argument(error);
		}
		return actual;
	}

	std::unique_ptr<Ast::UnaryOperator> parseUnaryOperator (auto& constantToken) {

	}

	std::unique_ptr<Ast::OperatorConstant> parseOperatorConstant(auto& constantToken, VectorAndIterator& tokens) {
		auto unop {parseUnaryOperator(constantToken)};
	}

	// Parse Integer values and return a pointer
	std::unique_ptr<Ast::IntConstant> parseIntConstant (Token::Token& token) {
		// Get the value stored in the token
		int tokenValue {std::get<Token::Constant>(token.type).value};

		// Make it a unique pointer and return it
		return std::make_unique<Ast::IntConstant>(tokenValue);
	}

	std::unique_ptr<Ast::Constant> parseConstant(VectorAndIterator& tokens) {
		auto& tmp {tokens.takeCurrent()};
		if (std::is_same< == Token::OpenParen) {

		}
		//Check that token contains Token::Constant - if not, throw an error
		// If it does, return the current token
		auto& constantToken {expect(Token::constantString, tokens)};

		// Work out what Constant subtype the constantToken is
		if (Visitor::getStructName(constantToken) == Token::negateString) {
			return parseOperatorConstant(constantToken, tokens);
		};

		// Return a unique pointer to an IntConstant Object
		return parseIntConstant(constantToken);
	}

	std::unique_ptr<Ast::KeywordStatement> parseKeywordStatement (const std::string& keyword, VectorAndIterator& tokens) {
		// Get the return value
		auto returnValuePtr {parseConstant(tokens)};

		// Check the statement ends with a semicolon token
		expect(Token::semicolonString, tokens);

		return std::make_unique<Ast::KeywordStatement>(keyword, std::move(returnValuePtr));
	}

	// Helper function to select the correct type of statement
	std::unique_ptr<Ast::Statement> parseStatement(VectorAndIterator& tokens) {
		std::unique_ptr<Ast::Statement> tmp;
		auto& currentStructName {Visitor::getStructName(tokens.takeCurrent())};
		// If the name of the struct is in keywordStringPtrs, then parse as a keywordStatement
		if (Token::isKeyword(currentStructName)) {
			tmp = parseKeywordStatement(currentStructName, tokens);
		} else {
			throw std::invalid_argument(currentStructName + "is not a recognised keyword");
		}

		// return a unique pointer to a statement object
		return tmp;
	}

	std::unique_ptr<Ast::Identifier> parseIdentifier(VectorAndIterator& tokens) {
		// Check that the token is an identifier
		auto& id {expect(Token::identifierString, tokens)};

		// Get the identifier string
		std::string& identifier {std::get<Token::Identifier>(id.type).name};

		//return a pointer to an identifier object
		return std::make_unique<Ast::Identifier>(identifier);
	}

	std::unique_ptr<Ast::Function> parseFunction(VectorAndIterator& tokens) {
		// Check return value
		expect(Token::intString, tokens);

		// Check Identifier
		auto identifier {parseIdentifier(tokens)};

		expect(Token::openParenString, tokens);
		expect(Token::voidString, tokens);
		expect(Token::closeParenString, tokens);
		expect(Token::openBraceString, tokens);

		// Get a unique pointer to the statement body
		auto statementBody {parseStatement(tokens)};

		expect(Token::closeBraceString, tokens);

		return std::make_unique<Ast::Function>(std::move(identifier), std::move(statementBody));
	}

	Ast::Program parseProgram(std::vector<Token::Token>& t) {
		VectorAndIterator tokens {t};
		Ast::Program tmp {parseFunction(tokens)};
		if (tokens.index() != (tokens.size())) {
			int remaining {tokens.size() - tokens.index()};
			throw std::out_of_range("Tokens remaining in tokens vector. Quantity: " + std::to_string(remaining));
		}
		return tmp;
	}
}