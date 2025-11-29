//
// Created by dunca on 02/11/2025.
//
#include <unordered_map>

#include "assembly_generator.h"
#include "../lexer/tokens.h"
#include "../tacky/tacky.h"
#include "../helpers/overload.h"


namespace AAstGen {
    ////////////////////////////////
    /// Initial Assembly Ast Gen ///
    ////////////////////////////////
    /// Functions intended to generate an initial AAst that can be further optimised.

    std::unique_ptr<AAst::ImmOperand> generateImmOperand(const int value) {
        return std::make_unique<AAst::ImmOperand>(value);
    }

    // works out the operand to return, then creates and returns it
    AAst::Operand generateOperand(const Tky::Value& value) {
        auto determineValueType = [](const auto& ops) -> AAst::Operand {
            using T = std::decay_t<decltype(ops)>;
            if constexpr (std::is_same_v<T, Tky::ConstantValue>) {
                return AAst::ImmOperand{ops.constant()};
            }
            else if constexpr (std::is_same_v<T, Tky::VariableValue>) {
                if (ops.variable() == AAst::registerStrings[AAst::AX]) {
                    return AAst::RegisterOperand{AAst::AX};
                } else {
                    return AAst::PseudoOperand{ops.variable()};
                }
            }
        };

        return std::visit(determineValueType, value);
    }

    AAst::Unop generateUnop(const Tky::Unop& unop) {
        // Go through the possible Unary operator strings and return the right object
        const std::string& unopString {unop.unop()};
        if (unopString == Token::bitwisenotString) {
            return AAst::NotUnop;
        }
        else if (unopString == Token::negateString) {
            return AAst::NegUnop;
        }
    }

    // Create unique pointer to a Retinstruction
    std::unique_ptr<AAst::Instruction> generateRetInstruction() {
        AAst::RetInstruction rInst{};
        return std::make_unique<AAst::Instruction>(rInst);
    }

    std::unique_ptr<AAst::Instruction> generateMovInstruction(const Tky::Value& src, const Tky::Value& dst) {
        // Construct the unique pointers to the Operands
        auto toMove {generateOperand(src)};
        auto destination {generateOperand(dst)};

        // Construct the MovInstruction
        AAst::MovInstruction movInst {toMove, destination};

        // Make the MovInstruction a unique pointer and return it
        return std::make_unique<AAst::Instruction>(std::move(movInst));
    }

    std::unique_ptr<AAst::Instruction> generateUnopInstruction(const Tky::Unop& unop, const Tky::Value& dst) {
        // Construct the unique pointer to the unary operator and the target register
        auto unaryOperator {generateUnop(unop)};
        auto destination {generateOperand(dst)};

        // Construct the UnopInstruction
        AAst::UnopInstruction unopInst {unaryOperator, destination};

        // Make the UnoppInstruction a unique pointer and return it
        return std::make_unique<AAst::Instruction>(std::move(unopInst));
    }

    using TkyInstructionList = std::vector<std::unique_ptr<Tky::Instruction>>;
    using AAstInstructionList = std::vector<std::unique_ptr<AAst::Instruction>>;

    // Helper to construct the instruction list one at a timeAst::Statement& statement
    // Also checks the type of the statement, as single statements can produce multiple instructions
    std::vector<std::unique_ptr<AAst::Instruction>> generateInstructionList(const TkyInstructionList& instructionList) {
        AAstInstructionList instructions;
        // Lambda to detect the instruction type so it can be correctly parsed
        auto parseInstruction = [&instructions](auto& inst) -> void {
            using T = std::decay_t<decltype(inst)>;
            // Unary instruction path. Create the mov and unop constructs
            if constexpr (std::is_same_v<T, Tky::UnaryInstruction>) {
                instructions.push_back(generateMovInstruction(inst.src(), inst.dst()));
                instructions.push_back(generateUnopInstruction(inst.unop(), inst.dst()));
            }
            // Return Instruction path
            else if constexpr (std::is_same_v<T, Tky::ReturnInstruction>) {
                Tky::VariableValue registerDst {AAst::registerStrings[AAst::AX]};
                instructions.push_back(generateMovInstruction(inst.value(), registerDst));
                instructions.push_back(generateRetInstruction());
            }
        };
        // Get the instruction type, and branch to the relevant function
        for (auto& instruction : instructionList) {
            std::visit(parseInstruction, *instruction);
        }
        return instructions;
    }

    // Parses functions by looking at the identifier and the accompanying list of instructions
    std::unique_ptr<AAst::Function> generateFunction(const Tky::Function& function) {
        const std::string& identifier {function.identifier()};

        // Construct the list of instructions from the contained statement
        AAstInstructionList instructionList {generateInstructionList(function.instructions())};
        return std::make_unique<AAst::Function>(identifier, std::move(instructionList));
    }

    AAst::Program generateProgram(Tky::Program& program) {
        AAst::Program tmp {generateFunction(program.function())};
        return tmp;
    }

    ///////////////////////////////
    /// Replace Pseudoregisters ///
    ///////////////////////////////
    /// Second compiler pass to replace all pseudoregister nodes with stack nodes
    /// An unordered_map is used to track what pseudoregister values map to stack values

    using PrToOffsetMap = std::unordered_map<std::string_view, int>;

    // Gets the latest stack offset
    // if no argument is given, defaults to just returning the current value of the offset
    int getStackOffset(int offset) {
        static int stackOffset {0};
        if (offset >= 0) {
            return stackOffset;
        }
        else {
            stackOffset += offset;
            return stackOffset;
        }
    }

    bool isPseudoOperand(AAst::Operand& operand) {
        return std::holds_alternative<AAst::PseudoOperand>(operand);
    }

    // Check if the pseudoaddress is already mapped to a stack offset
    // If it is, return that offset, otherwise create a new key/value pair
    int getStackOffset(AAst::PseudoOperand& pseudoOp, PrToOffsetMap& prToStackOffset) {
        auto pseudoAddressPos {prToStackOffset.find(pseudoOp.pseudoAddress())};
        if (pseudoAddressPos != prToStackOffset.end()) {
            return pseudoAddressPos->second;
        }
        else {
            int stackOffset {getStackOffset(-4)};
            prToStackOffset[pseudoOp.pseudoAddress()] = stackOffset;
            return stackOffset;
        }
    }


    void replacePseudoOperandInUnop(AAst::UnopInstruction& inst,
                                    PrToOffsetMap& prToStackOffset) {
        if (isPseudoOperand(inst.operand())) {
            AAst::StackOperand stackOffset {getStackOffset(std::get<AAst::PseudoOperand>(inst.operand()),
                                            prToStackOffset)};
            inst.setOperand(stackOffset);
        }
    }

    // Swap pseudooperands for stackoperands in a MovInstruction
    void replacePseudoOperandsInMov(AAst::MovInstruction& inst,
                                    PrToOffsetMap& prToStackOffset) {
        if (isPseudoOperand(inst.toMove())) {
            AAst::StackOperand stackOffset {getStackOffset(std::get<AAst::PseudoOperand>(inst.toMove()),
                                            prToStackOffset)};
            inst.setToMove(stackOffset);
        }
        if (isPseudoOperand(inst.destination())) {
            AAst::StackOperand stackOffset {getStackOffset(std::get<AAst::PseudoOperand>(inst.destination()),
                                            prToStackOffset)};
            inst.setDestination(stackOffset);
        }
    }

    void findAndReplacePseudoOperands(AAst::Program& program) {
        PrToOffsetMap prToStackOffset;
        AAstInstructionList& mainInstructionList{program.function().instructions()};
        for (auto& instruction : mainInstructionList) {
            // Check if the instruction type can contain a pseudooperand
            // If it can, send it to the relevant subfunction
            std::visit(Ol::overloaded{
                [&prToStackOffset](AAst::MovInstruction& inst) -> void {
                    replacePseudoOperandsInMov(inst, prToStackOffset);
                },
                [&prToStackOffset](AAst::UnopInstruction& inst) -> void {
                    replacePseudoOperandInUnop(inst, prToStackOffset);
                },
                [&prToStackOffset](AAst::RetInstruction& inst) -> void {
                    // RetInstructions do not contain pseudoregisters
                },
                [&prToStackOffset](AAst::StackallocInstruction& inst) -> void {
                    // StackallocInstructions do not contain pseudoregisters
                }}, *instruction
            );
        }
    }

    //////////////////////////////////////
    /// Add stack size and rewrite Mov ///
    //////////////////////////////////////
    /// Add instructions to set the stack size and rewrite Mov instrutcions
    /// Mov instructions cannot have a src and dst as stack offsets, so intermediate steps must be added with registers

    bool needsRegisterStep(AAst::Instruction& inst) {
        return std::holds_alternative<AAst::MovInstruction>(inst)
                && std::holds_alternative<AAst::StackOperand>(std::get<AAst::MovInstruction>(inst).toMove())
                && std::holds_alternative<AAst::StackOperand>(std::get<AAst::MovInstruction>(inst).destination());
    }

    void getStackSizeAndAddMovRegisters(AAst::Program& program) {
        // Iterate over the instructions to find out how many new mov instructions need to be added
        // Counter starts at 2 because of stackallocinstruction and the final mov instruction before ret
        int newIndicesCounter {2};
        AAstInstructionList& currentInstructions{program.function().instructions()};
        for (auto& inst : currentInstructions) {
            if (needsRegisterStep(*inst)) {
                ++newIndicesCounter;
            }
        }

        // Create a new vector pre-sized to match the number of added instructions
        AAstInstructionList finalInstructions;
        finalInstructions.reserve(newIndicesCounter + std::ssize(currentInstructions));

        // Get the final stackoffset, create a StackAlloc instruction and place it at the start of the instructions
        AAst::StackallocInstruction finalOffset {getStackOffset()};
        finalInstructions.push_back(std::make_unique<AAst::Instruction>(finalOffset));

        // counter to keep track of the last offset
        int lastOffset{0};

        for (auto& inst : currentInstructions) {
           if (needsRegisterStep(*inst)) {
               // All modifications are done on the instruction inst points to
               AAst::MovInstruction& movInst1 {std::get<AAst::MovInstruction>(*inst)};
               AAst::Operand dst {movInst1.destination()};
               AAst::RegisterOperand reg {AAst::R10};
               movInst1.setDestination(reg);

               // Create new MovInstruction
               AAst::MovInstruction movInst2 {reg, dst};

               // Move inst to the new vector
               finalInstructions.push_back(std::move(inst));
               finalInstructions.push_back(std::make_unique<AAst::Instruction>(movInst2));
           } else {
               finalInstructions.push_back(std::move(inst));
           }
        }

        program.function().setInstructions(std::move(finalInstructions));
    }
}
