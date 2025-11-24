//
// Created by dunca on 02/11/2025.
//

#ifndef DCC_ASSEMBLY_AST_H
#define DCC_ASSEMBLY_AST_H
#include <string>
#include <memory>
#include <vector>
#include <array>
#include <variant>

namespace AAst {

	enum NodeType {
		AstT,
		ProgramT,
		FunctionT,
		IdentifierT,
		InstructionT,
		OperandT,
		MovInstructionT,
		RetInstructionT,
		ImmOperandT,
		RegisterOperandT,
		NodeTypeLength
	};

	constexpr std::array<std::string, NodeTypeLength> nodeTypeStrings {"Ast", "Program", "Function", "Identifier",
		"MovInstruction", "RetInstruction", "Immediate", "RegisterOperand"};
	static_assert(std::size(nodeTypeStrings) == NodeTypeLength && "nodeTypeStrings is a different length to NodeType");

	class Ast;
	class Program;
	class Function;
	class Identifier;

	class MovInstruction;
	class RetInstruction;
	class UnopInstruction;
	class StackallocInstruction;

	class ImmOperand;
	class RegisterOperand;
	class UnopOperand;
	class PseudoOperand;
	class StackOperand;

	// identify to get the string
	// <<operator as friend
	// virtual destructor
	class Ast {
	public:
		virtual ~Ast() = default;
	};

	///////////////////////
	/// Unary Operators ///
	///////////////////////
	class NegUnop : public Ast {};

	class NotUnop : public Ast {};

	using Unop =
		std::variant<
			NegUnop,
			NotUnop
		>;

	////////////////
	/// Operands ///
	////////////////
	using Operand =
		std::variant <
			ImmOperand,
			RegisterOperand,
			UnopOperand,
			PseudoOperand,
			StackOperand
		>;

	// Container for an int
	class ImmOperand : public Ast {
		int m_value;
	public:
		ImmOperand(int value)
			: m_value{value}
		{}

		int value() const { return m_value; }
	};

	// Container for a register name - initially blank
	class RegisterOperand : public Ast {
		// Name/address of the register
		std::string m_destination;
	public:
		RegisterOperand(const std::string& destination)
			: m_destination{destination}
		{}

		//Returns a const reference to the destination address
		const std::string& destination() const { return m_destination; }
	};

	class PseudoOperand : public Ast {
		std::string m_identifier;
	public:
		PseudoOperand() = delete;
		PseudoOperand(const std::string& identifier)
			: m_identifier{identifier}
		{}
	};

	class StackOperand : public Ast {
		int m_value;
	public:
		StackOperand() = delete;
		StackOperand(int value)
			: m_value{value}
		{}
	};

	class UnopOperand : public Ast {
		Unop m_unop;
		std::unique_ptr<Operand> m_operand;
	public:
		UnopOperand() = delete;
		UnopOperand(Unop& unop, std::unique_ptr<Operand>&& operand)
			: m_unop{unop}
			, m_operand{std::move(operand)}
		{}
	};

	///////////////////
	/// Instruction ///
	///////////////////

	// Container for two pointers to operands
	class MovInstruction : public Ast {
		std::unique_ptr<Operand> m_toMove;
		std::unique_ptr<RegisterOperand> m_destination;
	public:
		MovInstruction(std::unique_ptr<Operand>&& toMove, std::unique_ptr<RegisterOperand>&& destination)
			: m_toMove{std::move(toMove)}
			, m_destination{std::move(destination)}
		{}

		const Operand& toMove() { return *m_toMove; }
		const Operand& destination() { return *m_destination; }

		const NodeType type() const override { return MovInstructionT; }
	};

	// Empty class to represent return
	class RetInstruction : public Ast {
	public:
		const NodeType type() const override { return RetInstructionT; }
	};

	using Instruction =
		std::variant<
			MovInstruction,
			RetInstruction
		>;


	// Container for std::string
	class Identifier : public Ast {
		std::string m_identifier;
	public:
		Identifier(std::string&& identifier)
			: m_identifier{std::move(identifier)}
		{}

		const std::string& identifier() const { return m_identifier; }

		const NodeType type() const override { return IdentifierT; }
	};

	// Container for pointer to identifier and pointer to list of pointers to instructions
	class Function : public Ast {
		std::unique_ptr<Identifier> m_identifier;
		std::vector<std::unique_ptr<Instruction>> m_instructions;
	public:
		Function(std::unique_ptr<Identifier>&& identifier, std::vector<std::unique_ptr<Instruction>>&& instructions)
			: m_identifier{std::move(identifier)}
			, m_instructions{std::move(instructions)}
		{}

		const Identifier& identifier() const { return *m_identifier; }

		const std::vector<std::unique_ptr<Instruction>>& instructions() const { return m_instructions; }

		const NodeType type() const override { return FunctionT; }
	};

	// Container for pointer to function
	class Program : public Ast {
		std::unique_ptr<Function> m_function;
	public:
		Program(std::unique_ptr<Function>&& a_function)
			: m_function{std::move(a_function)}
		{}

		const Function& function() const { return *m_function; }

		const NodeType type() const override { return ProgramT; }
	};
}
#endif //DCC_ASSEMBLY_AST_H