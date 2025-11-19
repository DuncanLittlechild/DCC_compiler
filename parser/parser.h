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

	std::unique_ptr<Ast::UnaryOperator> parseUnaryOperator (auto& constantToken);

	// Construct the unary operator constant
	// This can be nested an arbitrary number of times
	std::unique_ptr<Ast::OperatorConstant> parseUnaryOperatorConstant(auto& currentTokenName, VectorAndIterator& tokens);

	// Parse Integer values and return a pointer
	std::unique_ptr<Ast::IntConstant> parseIntConstant (Token::Token& token);

	// Helper to work out what type of cosntatn token to create
	std::unique_ptr<Ast::Constant> parseConstant(VectorAndIterator& tokens);

	std::unique_ptr<Ast::KeywordStatement> parseKeywordStatement (const std::string& keyword, VectorAndIterator& tokens);

	// Statements are complete lines that come before semicolons in C
	// Helper function to select the correct type of statement
	std::unique_ptr<Ast::Statement> parseStatement(VectorAndIterator& tokens);

	std::unique_ptr<Ast::Function> parseFunction(VectorAndIterator& tokens);

	Ast::Program parseProgram(std::vector<Token::Token>& t);
}

#endif //DCC_PARSER_H
