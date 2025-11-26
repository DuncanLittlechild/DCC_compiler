//
// Created by duncan on 11/24/25.
//
#include "tacky_generator.h"
#include "../assembly_generator/assembly_ast.h"
#include "../lexer/tokens.h"

namespace TkyGen {
    using InstructionList = std::vector<std::unique_ptr<Tky::Instruction>>;

    std::string createTempName() {
        static int counter {0};
        return "tmp" + std::to_string(++counter);
    }

    Tky::Unop parseUnop(Ast::UnaryOperator& unop) {
        return Tky::Unop {unop.unop()};
    }

    Tky::ConstantValue parseConstantValue(Ast::IntConstant& constant) {
        return Tky::ConstantValue {constant.value()};
    }

    Tky::ReturnInstruction parseReturnInstruction(Tky::Value& value ) {
        Tky::Value returnValue {value};
        return Tky::ReturnInstruction{returnValue};
    }

    // Recursively parse an instruction list
    // Uses recursion to descend until a constant is encountered, then constructs a list of
    // instructions that spell out each modification performed on the constant
    Tky::Value parseInstructionList(Ast::Expression& e, InstructionList& list) {
        return std::visit([&](auto&& arg)-> Tky::Value {
            using T = std::decay_t<decltype(arg)>;
            // If constant expression, create the constant
            if constexpr (std::is_same_v<T, Ast::ConstantExpression>) {
                return parseConstantValue(arg.constant());
            } else if constexpr (std::is_same_v<T, Ast::UnopExpression>) {
                Tky::Value src {parseInstructionList(e, list)};
                Tky::Value dst {createTempName()};
                Tky::Unop unop {parseUnop(std::get<Ast::UnopExpression>(e).unop())};
                Tky::UnaryInstruction tmp {unop, src, dst};
                list.push_back(std::make_unique<Tky::Instruction>(tmp));
                return dst;
            }
        }, e);
    }

    // Helper function to handle content in the Ast::Statement node
    // Directs to the parseInstructionList function
    InstructionList preParseInstructionList(Ast::Statement& statement) {
        Ast::NodeType type {std::visit(Ast::GetStatementType{}, statement)};
        InstructionList instructions;
        if (type == Ast::KeywordStatementT) {
            const std::string& keyword {std::get<Ast::KeywordStatement>(statement).keyword()};
            if (keyword == Token::returnString) {
                Ast::Expression& expression {std::get<Ast::KeywordStatement>(statement).expression()};
                Tky::Value returnVal = parseInstructionList(expression, instructions);
                instructions.push_back(std::make_unique<Tky::Instruction>(parseReturnInstruction(returnVal)));
            }
        }
        return instructions;
    }

    std::unique_ptr<Tky::Function> parseFunction(const Ast::Function& function) {
        const std::string& identifier {function.identifier().name()};
        std::vector<std::unique_ptr<Tky::Instruction>> instructions {preParseInstructionList(function.statement())};
    }

    Tky::Program parseProgram(Ast::Program& program) {
        Tky::Program tmp {parseFunction(program.function())};
        return tmp;
    }
}