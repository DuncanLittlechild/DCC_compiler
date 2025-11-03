//
// Created by dunca on 02/11/2025.
//

#ifndef DCC_ASSEMBLY_AST_H
#define DCC_ASSEMBLY_AST_H
#include "assembly_ast.h"
#include "assembly_ast.h"

namespace AssemblyAst {

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

	};

	// Container for pointer to function
	class Program : public Ast {

	};

	// Container for pointer to identifier and pointer to list of pointers to instructions
	class Function : public Ast {

	};

	// Container for std::string
	class Identifier : public Ast {

	};

	// Base class from which to derive types of instruction
	class Instruction : public Ast {

	};

	// Base class from which to derive types of Operand
	class Operand : public Ast {

	};

	// Container for two pointers to operands
	class MovInstruction : public Instruction {

	};

	// Empty class to represent return
	class RetInstruction : public Instruction {

	};

	// Container for an int
	class ImmOperand : public Operand {

	};

	// Container for a register name - initially blank
	class RegisterOperand : public Operand {

	};

}
#endif //DCC_ASSEMBLY_AST_H