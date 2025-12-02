//
// Created by duncan on 11/26/25.
//

#ifndef DCC_TACKY_GENERATOR_H
#define DCC_TACKY_GENERATOR_H
#include "tacky.h"
#include "../parser/ast.h"

namespace TkyGen {
    using InstructionList = std::vector<std::unique_ptr<Tky::Instruction>>;

    std::string createTempName();

    Tky::Unop parseUnop(Ast::UnaryOperator& unop);

    Tky::ConstantValue parseConstantValue(Ast::IntConstant& constant);

    Tky::ReturnInstruction parseReturnInstruction(Tky::Value& value);

    // Recursively parse an instruction list
    // Uses recursion to descend until a constant is encountered, then constructs a list of
    // instructions that spell out each modification performed on the constant
    Tky::Value parseInstructionList(Ast::ExpressionPtr& e, InstructionList& list);

    // Helper function to handle content in the Ast::Statement node
    // Directs to the parseInstructionList function
    InstructionList preParseInstructionList(Ast::Statement& statement);

    std::unique_ptr<Tky::Function> parseFunction(const Ast::Function& function);

    Tky::Program parseProgram(Ast::Program& program);
}
#endif //DCC_TACKY_GENERATOR_H