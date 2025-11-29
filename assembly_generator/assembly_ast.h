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
#include <cmath>

namespace AAst {
	enum NodeType {
		AstT,
		ProgramT,
		FunctionT,
		IdentifierT,
		MovInstructionT,
		RetInstructionT,
		ImmOperandT,
		RegisterOperandT,
		max_NodeType
	};

	constexpr std::array<std::string, max_NodeType> nodeTypeStrings {"Ast", "Program", "Function", "Identifier",
		"MovInstruction", "RetInstruction", "ImmOperand", "RegisterOperand"};
	static_assert(std::size(nodeTypeStrings) == max_NodeType && "nodeTypeStrings is a different length to NodeType");

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
	constexpr std::array<std::string, max_register_count> registerStrings{"eax","r10d"};
	static_assert(std::size(registerStrings) == max_register_count
		&& "Register enum and registerStrings are different sizes");

	///////////////////////
	/// Unary Operators ///
	///////////////////////
	enum Unop {
		NegUnop,
		NotUnop,
		max_unop_count
	};

	constexpr std::array<std::string, max_unop_count> unopStrings {"negl", "notl"};
	static_assert(std::size(unopStrings) == max_unop_count
		&& "Unop enum and unopStrings are different sizes");

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
		RegisterOperand(Register reg)
			: m_register{std::move(reg)}
		{}

		//Returns a const reference to the destination address
		const Register& reg() const { return m_register; }
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
	// Only ever stores negative numbers
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
		Operand m_toMove;
		Operand m_destination;
	public:
		MovInstruction(Operand toMove, Operand destination)
			: m_toMove{std::move(toMove)}
			, m_destination{std::move(destination)}
		{}

		Operand& toMove() { return m_toMove; }
		Operand& destination() { return m_destination; }

		void setToMove(Operand toMove) { m_toMove = std::move(toMove); }
		void setDestination(Operand destination) { m_destination = std::move(destination); }
	};

	// Class to represent a unary operator and the value it acts on
	class UnopInstruction : public Ast {
		Unop m_unop;
		Operand m_operand;
	public:
		UnopInstruction() = delete;
		UnopInstruction(Unop unop, Operand operand)
			: m_unop{std::move(unop)}
			, m_operand{std::move(operand)}
		{}

		Unop& unop() { return m_unop; }
		Operand& operand() { return m_operand; }

		void setOperand(Operand operand) { m_operand = std::move(operand); }
	};

	// Class to represent how much to increment the stack pointer by
	// Should only have one instance of this at the start of a function's instruction list
	class StackallocInstruction : public Ast {
		int m_stackSize;
	public:
		StackallocInstruction() = delete;
		StackallocInstruction(int stackSize)
			: m_stackSize{abs(stackSize)}
		{}

		const int stackSize() const { return m_stackSize; }
	};

	// Empty class to represent return
	class RetInstruction : public Ast {};

	using Instruction =
		std::variant<
			MovInstruction,
			UnopInstruction,
			StackallocInstruction,
			RetInstruction
		>;

	////////////////
	/// Function ///
	////////////////
	using InstructionList = std::vector<std::unique_ptr<Instruction>>;

	// Container for pointer to identifier and pointer to list of pointers to instructions
	class Function : public Ast {
		std::string m_identifier;
		InstructionList m_instructions;
	public:
		Function(const std::string& identifier, InstructionList&& instructions)
			: m_identifier{identifier}
			, m_instructions{std::move(instructions)}
		{}

		const std::string& identifier() const { return m_identifier; }
		InstructionList& instructions() { return m_instructions; }
		const InstructionList& instructions() const { return m_instructions; }

		void setInstructions(InstructionList&& instructions) { m_instructions = std::move(instructions); }
	};

	///////////////
	/// Program ///
	///////////////

	// Container for pointer to function
	class Program : public Ast {
		std::unique_ptr<Function> m_function;
	public:
		Program(std::unique_ptr<Function>&& a_function)
			: m_function{std::move(a_function)}
		{}

		Function& function() { return *m_function; }
	};
}
#endif //DCC_ASSEMBLY_AST_H