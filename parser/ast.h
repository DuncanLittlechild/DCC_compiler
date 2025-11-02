//
// Created by dunca on 01/11/2025.
//

#ifndef DCC_AST_H
#define DCC_AST_H

#include <string>
#include <memory>

// Holds the structure for the classes that make up the abstract syntax tree
namespace Ast {

	// Forward declarations of all classes
	// Serves as an index of classes for easier reading
	// Inherits directly from Ast
	class Ast;
	class Program;
	class Function;
	class Statement;
	class Keyword;
	class Constant;
	class Identifier;

	// represents return statements. Inherits from Keyword
	class ReturnKeyword;

	// represents integers, inherits from constant
	class IntConstant;

	class Ast {
	};

	class Keyword : public Ast {};

	class Constant : public Ast {};

	class Identifier : public Ast {
		std::string m_name;
	public:
		explicit Identifier(const std::string& name): m_name{name} {};
	};

	class IntConstant : public Constant {
		int m_value{};
	public:
		explicit IntConstant(const int& value): m_value{value} {};
	};

	class ReturnKeyword : public Keyword {
		std::unique_ptr<Constant> m_constant{};
	public:
		ReturnKeyword() = delete;
		explicit ReturnKeyword(std::unique_ptr<Constant> constant): m_constant{std::move(constant)}{};
	};

	class Statement : public Ast {
		std::unique_ptr<Keyword> m_keyword;
	public:
		Statement() = delete;
		Statement(std::unique_ptr<Keyword> keyword): m_keyword{std::move(keyword)} {}
	};

	class Function : public Ast {
		std::unique_ptr<Identifier> m_identifier;
		std::unique_ptr<Statement> m_statement;
	public:
		Function() = delete;
		Function(std::unique_ptr<Identifier> identifier, std::unique_ptr<Statement> statement)
		: m_identifier{std::move(identifier)}
		, m_statement{std::move(statement)} {}
	};

	class Program : public Ast {
		std::unique_ptr<Function> m_function;
	public:
		Program() = delete;
		explicit Program(std::unique_ptr<Function> function): m_function{std::move(function)} {}
	};
}
#endif //DCC_AST_H