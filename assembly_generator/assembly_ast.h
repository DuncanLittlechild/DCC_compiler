//
// Created by dunca on 02/11/2025.
//

#ifndef DCC_ASSEMBLY_AST_H
#define DCC_ASSEMBLY_AST_H
#include <string>
#include <memory>
#include <vector>

namespace AAst {

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
	public:
		virtual std::string identify() const = 0;
		virtual ~Ast() = default;
		friend std::ostream& operator<<(std::ostream& out, const Ast& ast) {
			out << ast.identify();
			return out;
		}
	};

	// Base class from which to derive types of Operand
	class Operand : public Ast {};

	// Container for an int
	class ImmOperand : public Operand {
		int m_value;
	public:
		ImmOperand(int value)
			: m_value{value}
		{}

		std::string identify() const override {
			return std::to_string(m_value);
		}

		int value() const { return m_value; }
	};

	// Container for a register name - initially blank
	class RegisterOperand : public Operand {
		// Name/address of the register
		std::string m_destination;
	public:
		RegisterOperand(const std::string& destination)
			: m_destination{destination}
		{}

		std::string identify() const override {
			return "RegisterOperand";
		}

		//Returns a const reference to the destination address
		const std::string& destination() const { return m_destination; }
		//Returns a copy of the destination address
		std::string getDestination() const { return m_destination; }
	};

	// Base class from which to derive types of instruction
	class Instruction : public Ast {};

	// Container for two pointers to operands
	class MovInstruction : public Instruction {
		std::unique_ptr<Operand> m_toMove;
		std::unique_ptr<RegisterOperand> m_destination;
	public:
		MovInstruction(std::unique_ptr<Operand>&& toMove, std::unique_ptr<RegisterOperand>&& destination)
			: m_toMove{std::move(toMove)}
			, m_destination{std::move(destination)}
		{}

		std::string identify() const override {
			return "MovInstruction";
		}

		const Operand& toMove() { return *m_toMove; }
		const Operand& destination() { return *m_destination; }
	};

	// Empty class to represent return
	class RetInstruction : public Instruction {
	public:
		std::string identify() const override {
			return "RetInstruction";
		}
	};


	// Container for std::string
	class Identifier : public Ast {
		std::string m_identifier;
	public:
		Identifier(std::string&& identifier)
			: m_identifier{std::move(identifier)}
		{}

		std::string identify() const override {
			return m_identifier;
		}

		const std::string& identifier() const { return m_identifier; }
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

		std::string identify() const override {
			return "Function";
		}

		const Identifier& identifier() { return *m_identifier; }

		const std::vector<std::unique_ptr<Instruction>>& instructions() const { return m_instructions; }
	};

	// Container for pointer to function
	class Program : public Ast {
		std::unique_ptr<Function> m_function;
	public:
		Program(std::unique_ptr<Function>&& a_function)
			: m_function{std::move(a_function)}
		{}

		std::string identify() const override {
			return "Program";
		}

		const Function& function() const { return *m_function; }
	};
}
#endif //DCC_ASSEMBLY_AST_H