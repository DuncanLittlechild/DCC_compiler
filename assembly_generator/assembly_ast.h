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

	// identify to get the string
	// <<operator as friend
	// virtual destructor
	class Ast {
	public:
		virtual ~Ast() = default;
	};

	/////////////////
	/// Registers ///
	/////////////////
	enum Register {
		AX,
		R10,
		max_register_count
	};
	constexpr std::array<std::string, max_register_count> registerStrings{"ax","r10"};
	static_assert(std::size(registerStrings) == max_register_count
		&& "Register enum and registerStrings are different sizes");

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

	// Forward declared because some Operands need nesting
	class ImmOperand;
	class RegisterOperand;
	class PseudoOperand;
	class StackOperand;

	using Operand =
		std::variant <
			ImmOperand,
			RegisterOperand,
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
		Register m_register;
	public:
		RegisterOperand() = delete;
		RegisterOperand(Register& reg)
			: m_register{reg}
		{}

		//Returns a const reference to the destination address
		const Register reg() const { return m_register; }
	};

	// Placeholder for an address relative to the base pointer
	class PseudoOperand : public Ast {
		std::string m_pseudoAddress;
	public:
		PseudoOperand() = delete;
		PseudoOperand(const std::string& pseudoAddress)
			: m_pseudoAddress{pseudoAddress}
		{}

		const std::string& pseudoAddress() const { return m_pseudoAddress; }
	};

	// Operand to show the offset of an address from the base pointer
	class StackOperand : public Ast {
		int m_value;
	public:
		StackOperand() = delete;
		StackOperand(int value)
			: m_value{value}
		{}

		const int value() const { return m_value; }
	};

	///////////////////
	/// Instruction ///
	///////////////////

	// Container for two pointers to operands
	class MovInstruction : public Ast {
		std::unique_ptr<Operand> m_toMove;
		std::unique_ptr<Operand> m_destination;
	public:
		MovInstruction(std::unique_ptr<Operand>&& toMove, std::unique_ptr<Operand>&& destination)
			: m_toMove{std::move(toMove)}
			, m_destination{std::move(destination)}
		{}

		const Operand& toMove() { return *m_toMove; }
		const Operand& destination() { return *m_destination; }
	};

	// Empty class to represent return
	class RetInstruction : public Ast {
		std::unique_ptr<Operand> m_returnValue;
	public:
		RetInstruction() = delete;
		RetInstruction(std::unique_ptr<Operand>&& returnValue)
			: m_returnValue{std::move(returnValue)}
		{}

		const Operand& returnValue() const { return *m_returnValue; }
	};

	// Class to represent a unary operator and the value it acts on
	class UnopInstruction : public Ast {
		std::unique_ptr<Unop> m_unop;
		std::unique_ptr<Operand> m_operand;
	public:
		UnopInstruction() = delete;
		UnopInstruction(std::unique_ptr<Unop>&& unop, std::unique_ptr<Operand>&& operand)
			: m_unop{std::move(unop)}
			, m_operand{std::move(operand)}
		{}

		const Unop& unop() const { return *m_unop; }
		const Operand& operand() const { return *m_operand; }
	};

	// Class to represent how much to increment the stack pointer by
	// Should only have one instance of this at the start of a function's instruction list
	class StackallocInstruction : public Ast {
		int m_stackSize;
	public:
		StackallocInstruction() = delete;
		StackallocInstruction(int stackSize)
			: m_stackSize{stackSize}
		{}

		const int stackSize() const { return m_stackSize; }
	};

	using Instruction =
		std::variant<
			MovInstruction,
			RetInstruction,
			UnopInstruction,
			StackallocInstruction
		>;



	// Container for pointer to identifier and pointer to list of pointers to instructions
	class Function : public Ast {
		std::string m_identifier;
		std::vector<std::unique_ptr<Instruction>> m_instructions;
	public:
		Function(const std::string& identifier, std::vector<std::unique_ptr<Instruction>>&& instructions)
			: m_identifier{identifier}
			, m_instructions{std::move(instructions)}
		{}

		const std::string& identifier() const { return m_identifier; }
		const std::vector<std::unique_ptr<Instruction>>& instructions() const { return m_instructions; }
	};

	// Container for pointer to function
	class Program : public Ast {
		std::unique_ptr<Function> m_function;
	public:
		Program(std::unique_ptr<Function>&& a_function)
			: m_function{std::move(a_function)}
		{}

		const Function& function() const { return *m_function; }
	};
}
#endif //DCC_ASSEMBLY_AST_H