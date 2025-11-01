//
// Created by dunca on 01/11/2025.
//

#ifndef DCC_AST_H
#define DCC_AST_H

#include <string>
 // Holds the structure for the classes that make up the abstract syntax tree
namespace Ast {

	// Forward declarations of all classes
	// Serves as an index of classes for easier reading
	class Program;
	class Function;
	class Statement;
	class ReturnKeyword;

	// Base class
     class Ast {
     };

	class Program : public Ast {
		Function m_function;
	public:
		Program() = delete;
		explicit Program(Function function): m_function{function} {}
	};

	class Function : public Ast {
		std::string m_identifier{};
		Statement m_statement;
	public:
		Function() = delete;
		Function(std::string identifier, Statement statement): m_identifier{identifier}, m_statement{statement} {}
	};

	class Statement : public Ast {
		ReturnKeyword m_return;
	public:
		Statement() = delete;
		Statement(ReturnKeyword ret): m_return{ret} {}
	};

	class ReturnKeyword : public Ast {
		int m_value{};
	public:
		ReturnKeyword() = delete;
		explicit ReturnKeyword(int value): m_value{value}{};
	};

}
#endif //DCC_AST_H