//
// Created by duncan on 11/14/25.
//

#ifndef DCC_ASSEMBLY_GENERATOR_H
#define DCC_ASSEMBLY_GENERATOR_H
#include <unordered_map>

#include "assembly_ast.h"
#include "../tacky/tacky.h"

namespace AAstGen {
    ////////////////////////////////
    /// Initial Assembly Ast Gen ///
    ////////////////////////////////
    /// Functions intended to generate an initial AAst that can be further optimised.

    std::unique_ptr<AAst::ImmOperand> generateImmOperand(const int value);

    // works out the operand to return, then creates and returns it
    AAst::Operand generateOperand(const Tky::Value& value);

    AAst::Unop generateUnop(const Tky::Unop& unop);

    // Create unique pointer to a Retinstruction
    std::unique_ptr<AAst::Instruction> generateRetInstruction();

    std::unique_ptr<AAst::Instruction> generateMovInstruction(const Tky::Value& src, const Tky::Value& dst);

    std::unique_ptr<AAst::Instruction> generateUnopInstruction(const Tky::Unop& unop, const Tky::Value& dst);

    using TkyInstructionList = std::vector<std::unique_ptr<Tky::Instruction>>;
    using AAstInstructionList = std::vector<std::unique_ptr<AAst::Instruction>>;

    // Helper to construct the instruction list one at a timeAst::Statement& statement
    // Also checks the type of the statement, as single statements can produce multiple instructions
    std::vector<std::unique_ptr<AAst::Instruction>> generateInstructionList(const TkyInstructionList& instructionList);

    // Parses functions by looking at the identifier and the accompanying list of instructions
    std::unique_ptr<AAst::Function> generateFunction(const Tky::Function& function);

    AAst::Program generateProgram(Tky::Program& program);

    ///////////////////////////////
    /// Replace Pseudoregisters ///
    ///////////////////////////////
    /// Second compiler pass to replace all pseudoregister nodes with stack nodes
    /// An unordered_map is used to track what pseudoregister values map to stack values

    using PrToOffsetMap = std::unordered_map<std::string_view, int>;

    // Gets the latest stack offset
    // if no argument is given, defaults to just returning the current value of the offset
    int getStackOffset(int offset = 1);

    bool isPseudoOperand(AAst::Operand& operand);

    // Check if the pseudoaddress is already mapped to a stack offset
    // If it is, return that offset, otherwise create a new key/value pair
    int getStackOffset(AAst::PseudoOperand& pseudoOp, PrToOffsetMap& prToStackOffset);

    void replacePseudoOperandInUnop(AAst::UnopInstruction& inst,
                                    PrToOffsetMap& prToStackOffset);

    // Swap pseudooperands for stackoperands in a MovInstruction
    void replacePseudoOperandsInMov(AAst::MovInstruction& inst,
                                    PrToOffsetMap& prToStackOffset);

    void findAndReplacePseudoOperands(AAst::Program& program);

    //////////////////////////////////////
    /// Add stack size and rewrite Mov ///
    //////////////////////////////////////
    /// Add instructions to set the stack size and rewrite Mov instrutcions
    /// Mov instructions cannot have a src and dst as stack offsets, so intermediate steps must be added with registers

    bool needsRegisterStep(AAst::Instruction& inst);

    void getStackSizeAndAddMovRegisters(AAst::Program& program);
}
#endif //DCC_ASSEMBLY_GENERATOR_H