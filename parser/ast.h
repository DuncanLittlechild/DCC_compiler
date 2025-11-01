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

	// represents return statements. Inherits from Keyword
	class ReturnKeyword;


	class Ast {
	};

	class Keyword : public Ast {};


	class ReturnKeyword : public Keyword {
		int m_value{};
	public:
		ReturnKeyword() = delete;
		explicit ReturnKeyword(int value): m_value{value}{};
	};

	class Statement : public Ast {
		std::unique_ptr<ReturnKeyword> m_return;
	public:
		Statement() = delete;
		Statement(std::unique_ptr<ReturnKeyword> ret): m_return{std::move(ret)} {}
	};

	class Function : public Ast {
		std::string m_identifier{};
		std::unique_ptr<Statement> m_statement;
	public:
		Function() = delete;
		Function(const std::string& identifier, std::unique_ptr<Statement> statement)
		: m_identifier{identifier}
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