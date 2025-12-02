//
// Created by dunca on 02/11/2025.
//

#ifndef DCC_PARSER_H
#define DCC_PARSER_H

#include "../lexer/tokens.h"
#include "ast.h"
namespace Parser {

	//class to iterate over the vector of tokens
	class VectorAndIterator;

	Token::Token& expect(auto& expected, VectorAndIterator& tokens);

	std::unique_ptr<Ast::Identifier> parseIdentifier(VectorAndIterator& tokens);

	Ast::BinaryOperator parseBinaryOperator(VectorAndIterator& tokens);

	Ast::UnaryOperator parseUnaryOperator (VectorAndIterator& tokens);

	// Parse Integer values and return a pointer
	std::unique_ptr<Ast::IntConstant> parseIntConstant (Token::Token& token);

	Ast::ExpressionPtr parseConstantExpression(VectorAndIterator& tokens);

	// Construct the unary operator constant
	// This can be nested an arbitrary number of times
	Ast::ExpressionPtr parseUnaryOperatorExpression(VectorAndIterator& tokens);

	// Helper to work out what type of expression token to create
	Ast::ExpressionPtr parseFactor(VectorAndIterator& tokens);

	// Parse to create left-associative binary operations
	// If there is another operation, the previous complete node becomes the left node of a new BinopExpression
	Ast::ExpressionPtr parseExpression(VectorAndIterator& tokens, int minPrecedence);

	Ast::Statement parseKeywordStatement (const std::string& keyword, VectorAndIterator& tokens);

	// Statements are complete lines that come before semicolons in C
	// Helper function to select the correct type of statement
	std::unique_ptr<Ast::Statement> parseStatement(VectorAndIterator& tokens);

	std::unique_ptr<Ast::Function> parseFunction(VectorAndIterator& tokens);

	Ast::Program parseProgram(std::vector<Token::Token>& t);
}

#endif //DCC_PARSER_H
