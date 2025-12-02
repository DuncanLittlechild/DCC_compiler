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

		const Token::Token& peekCurrent() const {
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
		if (Visitor::getTokenName(actual) != expected) {
			std::string error = "Parser::expect found unexpected token " + Visitor::getTokenName(actual) +
								" at index " + std::to_string(tokens.index());
			throw std::invalid_argument(error);
		}
		return actual;
	}


	std::unique_ptr<Ast::Identifier> parseIdentifier(VectorAndIterator& tokens) {
		// Check that the token is an identifier
		auto& id {expect(Token::identifierString, tokens)};

		// Get the identifier string
		std::string& identifier {std::get<Token::Identifier>(id.type).name};

		//return a pointer to an identifier object
		return std::make_unique<Ast::Identifier>(identifier);
	}

	Ast::BinaryOperator parseBinaryOperator(VectorAndIterator& tokens) {
		auto& currentTokenName {Visitor::getTokenName(tokens.takeCurrent())};
		return Ast::BinaryOperator{currentTokenName};
	}

	Ast::UnaryOperator parseUnaryOperator (VectorAndIterator& tokens) {
		auto& currentTokenName {Visitor::getTokenName(tokens.takeCurrent())};
		return Ast::UnaryOperator{currentTokenName};
	}

	// Parse Integer values and return a pointer
	std::unique_ptr<Ast::IntConstant> parseIntConstant (Token::Token& token) {
		// Get the value stored in the token
		int tokenValue {std::get<Token::Constant>(token.type).value};

		// Make it a unique pointer and return it
		return std::make_unique<Ast::IntConstant>(tokenValue);
	}

	Ast::ExpressionPtr parseConstantExpression(VectorAndIterator& tokens) {
		auto& currentToken{tokens.takeCurrent()};
		return std::make_unique<Ast::ConstantExpression>(parseIntConstant(currentToken));
	}

	// Construct the unary operator constant
	// This can be nested an arbitrary number of times
	Ast::ExpressionPtr parseUnaryOperatorExpression(VectorAndIterator& tokens) {
		auto unop {parseUnaryOperator(tokens)};
		auto constant{parseFactor(tokens)};
		return std::make_unique<Ast::UnopExpression>(unop, std::move(constant));
	}

	// Helper to work out what type of expression token to create
	Ast::ExpressionPtr parseFactor(VectorAndIterator& tokens) {
		Ast::ExpressionPtr expressionNode;

		auto& currentToken {tokens.peekCurrent()};
		auto& currentTokenName {Visitor::getTokenName(currentToken)};

		// Go over the current token and choose the appropriate constant to generate
		if (currentTokenName == Token::openParenString) {
			++tokens;
			expressionNode = parseExpression(tokens, 0);
			expect(Token::closeParenString, tokens);
		} else if (currentTokenName == Token::constantString) {
			expressionNode = parseConstantExpression(tokens);
		} else if (Token::isUnop(currentTokenName)){
			expressionNode = Ast::ExpressionPtr{parseUnaryOperatorExpression(tokens)};
		} else {
			throw std::invalid_argument(currentTokenName + "is not a recognised constant");
		}

		// Return a unique pointer to a constant Object
		return expressionNode;
	}

	// Parse to create left-associative binary operations
	// If there is another operation, the previous complete node becomes the left node of a new BinopExpression
	Ast::ExpressionPtr parseExpression(VectorAndIterator& tokens, int minPrecedence) {
		auto getPrecedence = [](auto& tok) -> int {
			using T = std::decay_t<decltype(tok)>;
			if constexpr (Token::isBinopT<T>) {
				return tok.precedence;
			}
			else {
				throw std::invalid_argument("getPrecedence should only be called on binary operators");
			}
		};

		auto leftNode {parseFactor(tokens)};
		auto* nextTokenPtr {&tokens.peekCurrent()};
		int nextTokenPrecedence {getPrecedence(*nextTokenPtr)};
		while (Token::isBinop(*nextTokenPtr) && nextTokenPrecedence >= minPrecedence) {
			auto binop {parseBinaryOperator(tokens)};
			auto rightNode {parseExpression(tokens, nextTokenPrecedence + 1)};
			leftNode = std::make_unique<Ast::BinopExpression> (std::move(leftNode), binop, std::move(rightNode));
			nextTokenPtr = &tokens.peekCurrent();
			nextTokenPrecedence = getPrecedence(*nextTokenPtr);
		}
		return leftNode;
	}

	Ast::Statement parseKeywordStatement (const std::string& keyword, VectorAndIterator& tokens) {
		// Get the return value
		auto value {parseExpression(tokens, 0)};

		return Ast::KeywordStatement{keyword, std::move(value)};
	}

	// Statements are complete lines that come before semicolons in C
	// Helper function to select the correct type of statement
	std::unique_ptr<Ast::Statement> parseStatement(VectorAndIterator& tokens) {
		std::unique_ptr<Ast::Statement> statementNode;
		
		Token::Token& currentToken {tokens.takeCurrent()};
		auto& currentTokenName {Visitor::getTokenName(currentToken)};
		
		// Determine the subfunciton to pass the current token to
		if (Token::isKeyword(currentTokenName)) {
			statementNode = std::make_unique<Ast::Statement>(parseKeywordStatement(currentTokenName, tokens));
		} else {
			throw std::invalid_argument(currentTokenName + "is not a recognised keyword");
		}

		// Check the statement ends with a semicolon token
		expect(Token::semicolonString, tokens);

		// return a unique pointer to a statement object
		return statementNode;
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
