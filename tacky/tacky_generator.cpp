//
// Created by duncan on 11/24/25.
//
#include "tacky_generator.h"
#include "../assembly_generator/assembly_ast.h"
#include "../helpers/overload.h"
#include "../lexer/tokens.h"

// Generates a three address code Ast from a C Ast
namespace TkyGen {
    using InstructionList = std::vector<std::unique_ptr<Tky::Instruction>>;

    std::string createTempName() {
        static int counter {0};
        return "tmp." + std::to_string(++counter);
    }

    Tky::Binop parseBinop(Ast::BinaryOperator& binop) {
        return Tky::Binop {binop.binop()};
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
    Tky::Value parseInstructionList(Ast::ExpressionPtr& e, InstructionList& list) {
        return std::visit(Ol::overloaded{
            [&list](std::unique_ptr<Ast::ConstantExpression>& exp) -> Tky::Value {
                return parseConstantValue(exp->constant());
            },
            [&list](std::unique_ptr<Ast::UnopExpression>& exp) ->Tky::Value {
                Tky::Unop unop {parseUnop(exp->unop())};
                Tky::Value src {parseInstructionList(exp->expression(), list)};
                Tky::Value dst {createTempName()};
                Tky::UnaryInstruction tmp {unop, src, dst};
                list.emplace_back(std::make_unique<Tky::Instruction>(tmp));
                return dst;
            },
            [&list](std::unique_ptr<Ast::BinopExpression>& exp) -> Tky::Value {
                Tky::Binop binop {parseBinop(exp->binop())};
                Tky::Value src1 {parseInstructionList(exp->leftExpression(), list)};
                Tky::Value src2 {parseInstructionList(exp->rightExpression(), list)};
                Tky::Value dst {createTempName()};
                Tky::BinaryInstruction tmp {binop, src1, src2, dst};
                list.emplace_back(std::make_unique<Tky::Instruction>(tmp));
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
                Ast::ExpressionPtr& expression {std::get<Ast::KeywordStatement>(statement).expression()};
                Tky::Value returnVal = parseInstructionList(expression, instructions);
                instructions.push_back(std::make_unique<Tky::Instruction>(parseReturnInstruction(returnVal)));
            }
        }
        return instructions;
    }

    std::unique_ptr<Tky::Function> parseFunction(const Ast::Function& function) {
        const std::string& identifier {function.identifier().name()};
        std::vector<std::unique_ptr<Tky::Instruction>> instructions {preParseInstructionList(function.statement())};
        return std::make_unique<Tky::Function>(identifier, std::move(instructions));
    }

    Tky::Program parseProgram(Ast::Program& program) {
        Tky::Program tmp {parseFunction(program.function())};
        return tmp;
    }
}