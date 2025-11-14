//
// Created by dunca on 01/11/2025.
//

#ifndef DCC_AST_H
#define DCC_AST_H

#include <string>
#include <memory>
#include <iostream>

// Holds the structure for the classes that make up the abstract syntax tree
namespace Ast {

	// Forward declarations of all classes
	// Serves as an index of classes for easier reading
	// Inherits directly from Ast
	class Ast;
	class Program;
	class Function;
	class Statement;
	class Constant;
	class Identifier;

	// represents integers, inherits from constant
	class IntConstant;

	class ReturnStatement;

	class Ast {
	public:
		virtual std::string identify() const = 0;

		friend std::ostream& operator<<(std::ostream& out, const Ast& ast) {
			out << ast.identify();
			return out;
		};

		virtual ~Ast() = default;
	};

	// Base class to derive from
	// m_type is used to work out which derived class to dynamic_cast to
	class Constant : public Ast {
		std::string m_type;
	public:
		Constant(std::string& type)
			: m_type{type}
		{};

		const std::string& type() const { return m_type; }
		std::string getType() const { return m_type; }
	};

	class IntConstant : public Constant {
		int m_value{};
	public:
		explicit IntConstant(std::string type,const int& value)
			: Constant{type}
			, m_value{value} {};

		std::string identify() const override {
			return "Constant(" + std::to_string(m_value) + ")";
		};

		int value() const { return m_value; }
	};

	class Identifier : public Ast {
		const std::string m_name;
	public:
		explicit Identifier(const std::string& name): m_name{name} {};
		std::string identify() const override {
			return '"' + m_name + '"';
		};

		std::string name() const { return m_name; }
	};

	// Base class to inherit statements from
	// m_type allows the derived type to be identified
	class Statement : public Ast {
		std::string m_type;
	public:
		explicit Statement(const std::string& type): m_type{type} {};

		const std::string& type() const { return m_type; }
		std::string getType() const { return m_type; }
	};

	// Class for simple statements such as return 5
	// The keyword used will be from a limited list
	class KeywordStatement : public Statement {
		std::unique_ptr<Constant> m_constant{};
	public:
		KeywordStatement() = delete;
		KeywordStatement(const std::string& keyword, std::unique_ptr<Constant>&& constant)
			: Statement{keyword}
			, m_constant{std::move(constant)}
		{}

		[[nodiscard]] std::string identify() const override {
			return this->type() + "(\n\t\t\t" + m_constant->identify() + "\t\t\n)";
		}

		const Constant& constant() const { return *m_constant; }
	};

	class Function : public Ast {
		std::unique_ptr<Identifier> m_identifier;
		std::unique_ptr<Statement> m_statement;
	public:
		Function() = delete;
		Function(std::unique_ptr<Identifier>&& identifier, std::unique_ptr<Statement>&& statement)
		: m_identifier{std::move(identifier)}
		, m_statement{std::move(statement)} {}

		[[nodiscard]] std::string identify() const override {
			return "Function(\n\t\tname = " + m_identifier->identify() + "\n\t\tbody = " + m_statement->identify() + "\n\t)";
		}

		const Identifier& identifier() const { return *m_identifier; }
		Statement& statement() const { return *m_statement; }
	};

	// Holds an abstract syntax tree for a whole program
	class Program : public Ast {
		std::unique_ptr<Function> m_function;
	public:
		Program() = default;
		explicit Program(std::unique_ptr<Function>&& function): m_function{std::move(function)} {}

		[[nodiscard]] std::string identify() const override {
			return "Program(\n\t" + m_function->identify() + "\n)";
		}

		const Function& function() const { return *m_function; }
	};
}
#endif //DCC_AST_H