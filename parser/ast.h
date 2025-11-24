//
// Created by dunca on 01/11/2025.
//

#ifndef DCC_AST_H
#define DCC_AST_H

#include <string>
#include <memory>
#include <variant>
#include <iostream>

// Holds the structure for the classes that make up the abstract syntax tree
namespace Ast {
	class Ast {
	public:
		virtual ~Ast() = default;
	};

	/////////////////
	/// Operators ///
	/////////////////

	// Represents and stores the data for unary operators
	// Inherits the m_operator member from it's parent
	class UnaryOperator : public Ast {
		const std::string& m_unop;
	public:
		UnaryOperator() = delete;
		UnaryOperator(const std::string& unop)
			: m_unop {unop}
		{}

		const std::string& unop() const { return m_unop; }
	};

	//////////////////
	/// Constants ///
	/////////////////
	// Leaf integer constant class
	class IntConstant : public Ast {
		int m_value{};
	public:
		explicit IntConstant(const int& value)
			: m_value{value} {};

		int value() const { return m_value; }
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
	};


	////////////////////
	/// Expressions ///
	///////////////////
	class ConstantExpression;
	class UnopExpression;

	// variant to allow polymorphic expressions
	using Expression =	std::variant<
							ConstantExpression,
							UnopExpression
						>;

	// An expression that holds a particular constant
	class ConstantExpression : public Ast {
		std::unique_ptr<IntConstant> m_constant;
	public:
		explicit ConstantExpression(std::unique_ptr<IntConstant>&& constant)
			: m_constant{std::move(constant)}
		{}

		IntConstant& constant() const { return *m_constant;}
	};

	// A unary operator and another expression
	// As unary operators can be chained, this can be nested an arbitrary number of times
	class UnopExpression : public Ast {
		std::unique_ptr<UnaryOperator> m_unop{};
		std::unique_ptr<Expression> m_expression{};
	public:
		UnopExpression(std::unique_ptr<UnaryOperator>&& unop, std::unique_ptr<Expression>&& expression)
			: m_unop{std::move(unop)}
			, m_expression{std::move(expression)}
		{}

		UnaryOperator& unop() const { return *m_unop; }
		Expression& expression() const { return *m_expression; }
	};

	///////////////////
	/// Statements ///
	//////////////////
	class KeywordStatement;

	// Base class to inherit statements from
	using Statement = std::variant<
						KeywordStatement
					>;
	// Class for simple statements such as return 5
	// The keyword used will be taken from those in the Tokens file
	class KeywordStatement : public Ast {
		const std::string& m_keyword;
		std::unique_ptr<Expression> m_expression{};
	public:
		KeywordStatement() = delete;
		KeywordStatement(const std::string& keyword, std::unique_ptr<Expression>&& expression)
			: m_keyword{keyword}
			, m_expression{std::move(expression)}
		{}

		const std::string& keyword() const { return m_keyword; }
		Expression& expression() const { return *m_expression; }
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
		Statement& statement() const { return *m_statement; }
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

		Function& function() const { return *m_function; }
	};


	//////////////////////////////
	///// Visitors and Enums /////
	//////////////////////////////

	// Enum used to identify the type of each node
	enum NodeType {
		ProgramT,
		FunctionT,
		ConstantExpressionT,
		UnopExpressionT,
		IdentifierT,
		IntConstantT,
		KeywordStatementT,
		UnaryOperatorT,
		maxNodeType
	};

	// Allows iterating over the different types of node
	constexpr std::array<NodeType, maxNodeType> nodeTypes {ProgramT, FunctionT,
		ConstantExpressionT, IdentifierT, IntConstantT, KeywordStatementT, UnaryOperatorT};
	static_assert(std::size(nodeTypes) == maxNodeType && "Ast::nodeTypes does not match Ast::nodeTypes");

	// Allows getting the strings associated with a particular enum
	constexpr std::array<std::string_view, maxNodeType> nodeTypeStrings { "Program", "Function",
		"ConstantExpression", "UnopExpression", "Identifier", "IntConstant", "KeywordStatement", "UnaryOperator"};
	static_assert(std::size(nodeTypeStrings) == maxNodeType && "Ast::nodeTypeString does not match Ast::maxNodeType");

	///// Parsing /////
	struct GetStatementType {
		NodeType operator()(KeywordStatement& statement) { return KeywordStatementT; }
	};

	using AstNode =
		std::variant<
			Program,
			Function,
			ConstantExpression,
			UnopExpression,
			Identifier,
			IntConstant,
			KeywordStatement,
			UnaryOperator
	>;

	struct PrettyPrinter {
		void operator()(Program& program) const {
			(*this)(program.function());
		}
		void operator()(Function& function) const {
			std::cout << "Function: " << function.identifier().name() << "\n";
			std::cout << "\t";
			Statement& statement {function.statement()};
			NodeType type {std::visit(GetStatementType{}, statement)};
			if (type == KeywordStatementT) {
				(*this)(std::get<KeywordStatement>(statement));
			}
		}
		void operator()(KeywordStatement& statement) const {
			std::cout <<"Not implemented";
		}
	};
}
#endif //DCC_AST_H