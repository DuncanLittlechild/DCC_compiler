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

	// Enum used to identify the type of each node
	enum NodeType {
		AstT,
		ProgramT,
		FunctionT,
		StatementT,
		ExpressionT,
		ConstantExpressionT,
		OperatorExpressionT,
		ConstantT,
		IdentifierT,
		IntConstantT,
		KeywordStatementT,
		OperatorT,
		UnaryOperatorT,
		maxNodeType
	};

	// Allows iterating over the different types of node
	constexpr std::array<NodeType, maxNodeType> nodeTypes {AstT, ProgramT, FunctionT, StatementT, ExpressionT,
		ConstantExpressionT, ConstantT, IdentifierT, IntConstantT, KeywordStatementT, OperatorT, UnaryOperatorT};
	static_assert(std::size(nodeTypes) == maxNodeType && "Ast::nodeTypes does not match Ast::nodeTypes");

	// Allows getting the strings associated with a particular enum
	constexpr std::array<std::string_view, maxNodeType> nodeTypeStrings {"Ast", " Program", "Function", "Statement", "Expression"
		"ConstantExpression", "OperatorExpression", "Constant", "Identifier", "IntConstant", "KeywordStatement", "Operator", "UnaryOperator"};
	static_assert(std::size(nodeTypeStrings) == maxNodeType && "Ast::nodeTypeString does not match Ast::maxNodeType");

	class Ast {
	public:
		virtual ~Ast() = default;

		virtual const std::string_view identify() {
			return nodeTypeStrings[this->type()];
		}

		// Returns the nodeType of the class
		virtual const NodeType type() const = 0;
	};

	/////////////////
	/// Operators ///
	/////////////////

	// Base class to represent unary operators
	class Operator : public Ast {
		const std::string& m_operator;
	public:
		Operator() = delete;
		Operator(const std::string& op)
			: m_operator{op}
		{}

		const NodeType type() const override { return OperatorT; }
	};

	// Represents and stores the data for unary operators
	// Inherits the m_operator member from it's parent
	class UnaryOperator : public Operator {
	public:
		UnaryOperator() = delete;
		UnaryOperator(const std::string& op)
			: Operator{op}
		{}

		const NodeType type() const override { return UnaryOperatorT; }
	};

	//////////////////
	/// Constants ///
	/////////////////

	// Base class to derive constants from
	// These are basically just wrappers for constant values
	// Note that constant here means expressions that can be collated to make a single output, eg 5 + 6 is a constant
	class Constant : public Ast {
	public:
		const NodeType type() const override { return ConstantT; }
	};

	// Leaf integer constant class
	class IntConstant : public Constant {
		int m_value{};
	public:
		explicit IntConstant(const int& value)
			: m_value{value} {};

		int value() const { return m_value; }

		const NodeType type() const override { return IntConstantT; }
	};


	///////////////////
	/// Identifier ///
	//////////////////

	// The string used to identify a funciton or a variable
	class Identifier : public Ast {
		const std::string m_name;
	public:
		explicit Identifier(const std::string& name)
			: m_name{name}
		{};

		const std::string& name() const { return m_name; }

		const NodeType type() const override { return IdentifierT; }
	};


	////////////////////
	/// Expressions ///
	///////////////////

	// Class to hold expressions
	class Expression : public Ast {
	public:
		const NodeType type() const override { return ExpressionT; }
	};

	// An expression that holds a particular constant
	class ConstantExpression : public Expression {
		std::unique_ptr<Constant> m_constant;
	public:
		explicit ConstantExpression(std::unique_ptr<Constant>&& constant)
			: m_constant{std::move(constant)}
		{}

		const Constant& constant() const { return *m_constant;}

		const NodeType type() const override { return ConstantExpressionT; }
	};

	// A unary operator and another expression
	// As unary operators can be chained, this can be nested an arbitrary number of times
	class OperatorExpression : public Expression {
		std::unique_ptr<Operator> m_unop{};
		std::unique_ptr<Expression> m_expression{};
	public:
		OperatorExpression(std::unique_ptr<Operator>&& unop, std::unique_ptr<Expression>&& expression)
			: m_unop{std::move(unop)}
			, m_expression{std::move(expression)}
		{}

		const Operator& unop() const { return *m_unop; }
		const Expression& expression() const { return *m_expression; }

		const NodeType type() const override { return OperatorExpressionT; }
	};


	///////////////////
	/// Statements ///
	//////////////////

	// Base class to inherit statements from
	class Statement : public Ast {
	public:
		const NodeType type() const override { return StatementT; }
	};

	// Class for simple statements such as return 5
	// The keyword used will be taken from those in the Tokens file
	class KeywordStatement : public Statement {
		const std::string& m_keyword;
		std::unique_ptr<Expression> m_expression{};
	public:
		KeywordStatement() = delete;
		KeywordStatement(const std::string& keyword, std::unique_ptr<Expression>&& expression)
			: m_keyword{keyword}
			, m_expression{std::move(expression)}
		{}

		const std::string& keyword() const { return m_keyword; }
		const Expression& expression() const { return *m_expression; }

		const NodeType type() const override { return KeywordStatementT; }
	};


	//////////////////
	/// Functions ///
	/////////////////

	// The identifier string and main statement of a function
	class Function : public Ast {
		std::unique_ptr<Identifier> m_identifier;
		std::unique_ptr<Statement> m_statement;
	public:
		Function() = delete;
		Function(std::unique_ptr<Identifier>&& identifier, std::unique_ptr<Statement>&& statement)
		: m_identifier{std::move(identifier)}
		, m_statement{std::move(statement)} {}

		const Identifier& identifier() const { return *m_identifier; }
		const Statement& statement() const { return *m_statement; }

		const NodeType type() const override { return FunctionT; }
	};


	/////////////////
	/// Programs ///
	////////////////

	// Holds an abstract syntax tree for a whole program
	class Program : public Ast {
		std::unique_ptr<Function> m_function;
	public:
		Program() = default;
		explicit Program(std::unique_ptr<Function>&& function)
			: m_function{std::move(function)}
		{}

		const Function& function() const { return *m_function; }

		const NodeType type() const override { return ProgramT; }
	};
}
#endif //DCC_AST_H