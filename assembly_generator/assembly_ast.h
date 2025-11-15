//
// Created by dunca on 02/11/2025.
//

#ifndef DCC_ASSEMBLY_AST_H
#define DCC_ASSEMBLY_AST_H
#include <string>
#include <memory>
#include <vector>
#include <array>

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
	class Instruction;
	class Operand;

	class MovInstruction;
	class RetInstruction;

	class ImmOperand;
	class RegisterOperand;

	// identify to get the string
	// <<operator as friend
	// virtual destructor
	class Ast {
		NodeType m_nodeType {AstT};
	public:
		virtual ~Ast() = default;

		friend std::ostream& operator<<(std::ostream& out, const Ast& ast) {
			out << ast.identify();
			return out;
		}

		virtual const std::string& identify() const {
			return nodeTypeStrings[m_nodeType];
		}

		virtual const NodeType& type() const {
			return m_nodeType;
		}
	};

	// Base class from which to derive types of Operand
	class Operand : public Ast {
		NodeType m_nodeType {OperandT};
	public:
		virtual const std::string opString() const = 0;
	};

	// Container for an int
	class ImmOperand : public Operand {
		int m_value;

		NodeType m_nodeType {ImmOperandT};
	public:
		ImmOperand(int value)
			: m_value{value}
		{}

		int value() const { return m_value; }

		const std::string opString() const override { return "$" + std::to_string(m_value); }
	};

	// Container for a register name - initially blank
	class RegisterOperand : public Operand {
		// Name/address of the register
		std::string m_destination;

		NodeType m_nodeType {RegisterOperandT};
	public:
		RegisterOperand(const std::string& destination)
			: m_destination{destination}
		{}

		//Returns a const reference to the destination address
		const std::string& destination() const { return m_destination; }
		//Returns a copy of the destination address
		std::string getDestination() const { return m_destination; }

		const std::string opString() const override { return m_destination; }
	};

	// Base class from which to derive types of instruction
	class Instruction : public Ast {
		NodeType m_nodeType {InstructionT};
	};

	// Container for two pointers to operands
	class MovInstruction : public Instruction {
		std::unique_ptr<Operand> m_toMove;
		std::unique_ptr<RegisterOperand> m_destination;

		NodeType m_nodeType {MovInstructionT};
	public:
		MovInstruction(std::unique_ptr<Operand>&& toMove, std::unique_ptr<RegisterOperand>&& destination)
			: m_toMove{std::move(toMove)}
			, m_destination{std::move(destination)}
		{}

		const Operand& toMove() { return *m_toMove; }
		const Operand& destination() { return *m_destination; }
	};

	// Empty class to represent return
	class RetInstruction : public Instruction {
		NodeType m_nodeType {RetInstructionT};
	};


	// Container for std::string
	class Identifier : public Ast {
		std::string m_identifier;

		NodeType m_nodeType {IdentifierT};
	public:
		Identifier(std::string&& identifier)
			: m_identifier{std::move(identifier)}
		{}

		const std::string& identifier() const { return m_identifier; }
	};

	// Container for pointer to identifier and pointer to list of pointers to instructions
	class Function : public Ast {
		std::unique_ptr<Identifier> m_identifier;
		std::vector<std::unique_ptr<Instruction>> m_instructions;

		NodeType m_nodeType {FunctionT};
	public:
		Function(std::unique_ptr<Identifier>&& identifier, std::vector<std::unique_ptr<Instruction>>&& instructions)
			: m_identifier{std::move(identifier)}
			, m_instructions{std::move(instructions)}
		{}

		const Identifier& identifier() const { return *m_identifier; }

		const std::vector<std::unique_ptr<Instruction>>& instructions() const { return m_instructions; }
	};

	// Container for pointer to function
	class Program : public Ast {
		std::unique_ptr<Function> m_function;
		NodeType m_nodeType {ProgramT};
	public:
		Program(std::unique_ptr<Function>&& a_function)
			: m_function{std::move(a_function)}
		{}

		const Function& function() const { return *m_function; }
	};
}
#endif //DCC_ASSEMBLY_AST_H