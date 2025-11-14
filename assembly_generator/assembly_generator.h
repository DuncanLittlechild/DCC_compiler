//
// Created by duncan on 11/14/25.
//

#ifndef DCC_ASSEMBLY_GENERATOR_H
#define DCC_ASSEMBLY_GENERATOR_H
#include "assembly_ast.h"
#include "../parser/ast.h"

namespace AssemblyGenerator {
    std::unique_ptr<AAst::RegisterOperand> parseRegisterOperand(const Ast::Constant& constant);

    std::unique_ptr<AAst::ImmOperand> parseImmOperand(const int value);

    // works out the operand to return, then creates and returns it
    std::unique_ptr<AAst::Operand> parseOperand(const Ast::Constant& constant);

    std::unique_ptr<AAst::RetInstruction> parseRetInstruction();

    // Works out what is being moved, then where it needs to be moved to
    // Constructs a pointer using those two values
    std::unique_ptr<AAst::MovInstruction> parseMovInstruction(const Ast::Constant& constant);

    // Helper to construct the instruction list one at a time
    // Also checks the type of the statement, as single statements can produce multiple instructions
    std::vector<std::unique_ptr<AAst::Instruction>> parseInstructionList(Ast::Statement& statement);

    // Parses identifiers and returns a copy of the string
    std::unique_ptr<AAst::Identifier> parseIdentifier(const Ast::Identifier& identifier);

    // Parses functions by looking at the identifier and the accompanying list of instructions
    std::unique_ptr<AAst::Function> parseFunction(const Ast::Function& function);

    // Root function to parse a C Ast and return an assembly Ast
    AAst::Program parseProgram(Ast::Program& program);
}
#endif //DCC_ASSEMBLY_GENERATOR_H