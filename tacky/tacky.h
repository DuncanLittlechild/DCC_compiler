//
// Created by duncan on 11/23/25.
//

#ifndef DCC_TACKY_H
#define DCC_TACKY_H

#include <algorithm>
#include <memory>
#include <vector>
#include <variant>

namespace Tky {

     // Represents the negate operator
     class NegateUnop {

     };

     // Represents bitwise not
     class ComplementUnop {

     };

     // Represents unary operators
     // Contains a variant that allows for polymorphism
     class Unop {

     };

     // Represents a variable value
     class VariableValue {

     };

     // represents a const value
     class ConstantValue {

     };


     using Value =  std::variant<
                         std::unique_ptr<VariableValue>,
                         std::unique_ptr<ConstantValue>
                    >;

     // represents a unary operator and the value it operates on
     // Can form part of a chain of operators
     class UnaryInstruction {
          std::unique_ptr<Unop> m_unop;
          std::unique_ptr<Value> m_src;
          std::unique_ptr<Value> m_dst;
     public:
          UnaryInstruction() = delete;
          UnaryInstruction(std::unique_ptr<Unop>&& unop, std::unique_ptr<Value>&& src, std::unique_ptr<Value>&& dst)
               : m_unop(std::move(unop))
               , m_src(std::move(src))
               , m_dst(std::move(dst))
          {}
     };

     // Represents a return instruction
     // Contains only the final value to return
     class ReturnInstruction {
          std::unique_ptr<Value> m_value;
     public:
          ReturnInstruction() = delete;
          ReturnInstruction(std::unique_ptr<Value>&& value)
               : m_value(std::move(value))
          {}
     };


     using Instruction = std::variant<
                              UnaryInstruction,
                              ReturnInstruction
                         >;
     // Root node of functions
     // Contains an identifier, and a list of instructions
     class Function {
          const std::string m_identifier;
          std::vector<std::unique_ptr<Instruction>> m_instructions;
     public:
          Function() = delete;
          Function(const std::string& identifier, std::vector<std::unique_ptr<Instruction>>&& instructions)
               : m_identifier(identifier)
               , m_instructions(std::move(instructions))
          {}
     };

     // Root node of the tacky tree
     // Contains a function
     class Program {
          const std::unique_ptr<Function> m_function;
     public:
          Program() = delete;
          Program(std::unique_ptr<Function>&& function)
               : m_function(std::move(function))
          {}
     };
}
#endif //DCC_TACKY_H