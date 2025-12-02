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
     class Unop {
          const std::string& m_unop;
     public:
          Unop() = delete;
          Unop(const std::string& unop)
               : m_unop(unop)
          {}

          const std::string& unop() const { return m_unop; }
     };

     class Binop {
          const std::string& m_binop;
     public:
          Binop() = delete;
          Binop(const std::string& binop)
               : m_binop(binop)
          {}

          const std::string& binop() const { return m_binop; }
     };

     /////////////
     /// Value ///
     /////////////
     // Represents a variable value
     class VariableValue {
          const std::string m_variable;
     public:
          VariableValue() = delete;
          VariableValue(const std::string& variable)
               : m_variable(variable)
          {}

          const std::string& variable() const { return m_variable; }
     };

     // represents a const value
     class ConstantValue {
          const int m_constant;
     public:
          ConstantValue() = delete;
          ConstantValue(int constant)
               : m_constant(constant)
          {}

          const int constant() const { return m_constant; }
     };


     using Value =  std::variant<
                         VariableValue,
                         ConstantValue
                    >;

     ////////////////////
     /// Instructions ///
     ////////////////////
     // represents a unary operator and the value it operates on
     // Can form part of a chain of operators
     class UnaryInstruction {
          Unop m_unop;
          Value m_src;
          Value m_dst;
     public:
          UnaryInstruction() = delete;
          UnaryInstruction(Unop& unop, Value& src, Value& dst)
               : m_unop(unop)
               , m_src(src)
               , m_dst(dst)
          {}

          const Unop& unop() const { return m_unop; }
          const Value& src() const { return m_src; }
          const Value& dst() const { return m_dst; }
     };

     class BinaryInstruction {
          Binop m_binop;
          Value m_src1;
          Value m_src2;
          Value m_dst;
     public:
          BinaryInstruction() = delete;
          BinaryInstruction(Binop& binop, Value& src1, Value& src2, Value& dst)
               : m_binop(binop)
               , m_src1(src1)
               , m_src2(src2)
               , m_dst(dst)
          {}

          const Binop& binop() const { return m_binop; }
          const Value& src1() const { return m_src1; }
          const Value& src2() const { return m_src2; }
          const Value& dst() const { return m_dst; }
     };

     // Represents a return instruction
     // Contains only the final value to return
     class ReturnInstruction {
          Value m_value;
     public:
          ReturnInstruction() = delete;
          ReturnInstruction(Value& value)
               : m_value(value)
          {}

          const Value& value() const { return m_value; }
     };


     using Instruction = std::variant<
                              UnaryInstruction,
                              BinaryInstruction,
                              ReturnInstruction
                         >;

     ////////////////
     /// Function ///
     ////////////////
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
          const std::string& identifier() const { return m_identifier; }
          const std::vector<std::unique_ptr<Instruction>>& instructions() const { return m_instructions; }
     };

     ///////////////
     /// Program ///
     ///////////////
     // Root node of the tacky tree
     // Contains a function
     class Program {
          std::unique_ptr<Function> m_function;
     public:
          Program() = delete;
          explicit Program(std::unique_ptr<Function>&& function)
               : m_function(std::move(function))
          {}

          const Function& function() const { return *m_function; }


     };
}
#endif //DCC_TACKY_H