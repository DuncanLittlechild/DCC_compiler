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

	class Ast {
	public:
		virtual std::string identify() const {
			return "Ast";
		}

		friend std::ostream& operator<<(std::ostream& out, const Ast& ast) {
			out << ast.identify();
			return out;
		};

		virtual ~Ast() = default;
	};


	class Constant : public Ast {};

	class IntConstant : public Constant {
		int m_value{};
	public:
		explicit IntConstant(const int& value): m_value{value} {};
		std::string identify() const override { return "Constant(" + std::to_string(m_value) + ")"; };
	};

	class Identifier : public Ast {
		std::string m_name;
	public:
		explicit Identifier(const std::string& name): m_name{name} {};
		std::string identify() const override {
			return '"' + m_name + '"';
		};
	};

	class Statement : public Ast {
		std::string m_keyword;
		std::unique_ptr<Constant> m_constant{};
	public:
		Statement() = delete;
		Statement(const std::string& keyword, std::unique_ptr<Constant>&& constant)
			: m_keyword{keyword}
			, m_constant{std::move(constant)}
		{}
		std::string identify() const override {
			return "body = " + m_keyword;
		}
	};

	class Function : public Ast {
		std::unique_ptr<Identifier> m_identifier;
		std::unique_ptr<Statement> m_statement;
	public:
		Function() = delete;
		Function(std::unique_ptr<Identifier>&& identifier, std::unique_ptr<Statement>&& statement)
		: m_identifier{std::move(identifier)}
		, m_statement{std::move(statement)} {}
		std::string identify() const override {
			return "Function(\nname = " + m_identifier->identify() + ")";
		}
	};

	class Program : public Ast {
		std::unique_ptr<Function> m_function;
	public:
		Program() = delete;
		explicit Program(std::unique_ptr<Function>&& function): m_function{std::move(function)} {}
	};
}
#endif //DCC_AST_H