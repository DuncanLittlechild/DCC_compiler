//
// Created by dunca on 02/11/2025.
//

#include "assembly_generator.h"
#include "../lexer/tokens.h"

namespace AssemblyGenerator {

    std::unique_ptr<AAst::RegisterOperand> parseRegisterOperand(const Ast::Constant& constant) {
        return std::make_unique<AAst::RegisterOperand>("%%EAX");
    }

    std::unique_ptr<AAst::ImmOperand> parseImmOperand(const int value) {
        return std::make_unique<AAst::ImmOperand>(value);
    }

    // works out the operand to return, then creates and returns it
    std::unique_ptr<AAst::Operand> parseOperand(const Ast::Constant& constant) {
        return parseImmOperand(dynamic_cast<const Ast::IntConstant&>(constant).value());
    }

    std::unique_ptr<AAst::RetInstruction> parseRetInstruction() {
        return std::make_unique<AAst::RetInstruction>();
    }

    // Works out what is being moved, then where it needs to be moved to
    // Constructs a pointer using those two values
    std::unique_ptr<AAst::MovInstruction> parseMovInstruction(const Ast::Constant& constant) {
        auto toMove {parseOperand(constant)};
        auto destination {parseRegisterOperand(constant)};
        return std::make_unique<AAst::MovInstruction>(std::move(toMove), std::move(destination));
    }

    // Helper to construct the instruction list one at a time
    // Also checks the type of the statement, as single statements can produce multiple instructions
    std::vector<std::unique_ptr<AAst::Instruction>> parseInstructionList(Ast::Statement& statement) {
        std::vector<std::unique_ptr<AAst::Instruction>> instructions;
        // If the type of the current statement is return, return a mov and ret instruction
        // TODO: add a check, as I don't think there should be anything left after a return
        if (statement.type() == Token::returnString) {
            instructions.push_back(parseMovInstruction(dynamic_cast<Ast::KeywordStatement&>(statement).constant()));
            instructions.push_back(parseRetInstruction());
        }
        return instructions;
    }

    // Parses identifiers and returns a copy of the string
    std::unique_ptr<AAst::Identifier> parseIdentifier(const Ast::Identifier& identifier) {
        auto id {identifier.name()};
        return std::make_unique<AAst::Identifier>(std::move(id));
    }

    // Parses functions by looking at the identifier and the accompanying list of instructions
    std::unique_ptr<AAst::Function> parseFunction(const Ast::Function& function) {
        auto identifier {parseIdentifier(function.identifier())};
        auto instructions {parseInstructionList(function.statement())};
        return std::make_unique<AAst::Function>(std::move(identifier), std::move(instructions));
    }

    AAst::Program parseProgram(Ast::Program& program) {
        AAst::Program tmp {parseFunction(program.function())};
        return tmp;
    }
}
