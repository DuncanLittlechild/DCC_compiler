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
                // Work out if the string matches a register
                auto regStrPtr {std::find(AAst::registerStrings.begin(),
                                                  AAst::registerStrings.end(), ops.variable())};
                if (regStrPtr != AAst::registerStrings.end()) {
                    auto index {regStrPtr - AAst::registerStrings.begin()};
                    return AAst::RegisterOperand{AAst::registers[index]};
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

        throw std::runtime_error("Invalid unop in generateUnop: " + unopString);
    }

    AAst::Binop generateBinop(const Tky::Binop& binop) {
        using namespace Token;
        const std::string& binopString {binop.binop()};
        if (binopString == addString) {
            return AAst::AddBinop;
        }
        else if (binopString == negateString) {
            return AAst::SubBinop;
        }
        else if (binopString == multiplyString) {
            return AAst::MultiplyBinop;
        }

        throw std::runtime_error("Invalid binop in generateBinop: " + binopString);
    }

    // Create unique pointer to a Retinstruction
    std::unique_ptr<AAst::Instruction> generateRetInstruction() {
        AAst::RetInstruction rInst{};
        return std::make_unique<AAst::Instruction>(rInst);
    }

    std::unique_ptr<AAst::Instruction> generateCdqInstruction() {
        AAst::CdqInstruction cInst{};
        return std::make_unique<AAst::Instruction>(cInst);
    }

    std::unique_ptr<AAst::Instruction> generateIdivInstruction(const Tky::Value& value) {
        AAst::IdivInstruction idInst {generateOperand(value)};
        return std::make_unique<AAst::Instruction>(idInst);
    }

    std::unique_ptr<AAst::Instruction> generateBinopInstruction(const Tky::Binop& binop, const Tky::Value& left, const Tky::Value& right) {
        AAst::Binop binaryOperator {generateBinop(binop)};
        AAst::Operand leftOperand {generateOperand(left)};
        AAst::Operand rightOperand {generateOperand(right)};

        AAst::BinopInstruction binopInstruction {binaryOperator, leftOperand, rightOperand};
        return std::make_unique<AAst::Instruction>(binopInstruction);
    }

    std::unique_ptr<AAst::Instruction> generateMovInstruction(const Tky::Value& src, const Tky::Value& dst) {
        // Construct the unique pointers to the Operands
        AAst::Operand toMove {generateOperand(src)};
        AAst::Operand destination {generateOperand(dst)};

        // Construct the MovInstruction
        AAst::MovInstruction movInst {toMove, destination};

        // Make the MovInstruction a unique pointer and return it
        return std::make_unique<AAst::Instruction>(std::move(movInst));
    }

    std::unique_ptr<AAst::Instruction> generateUnopInstruction(const Tky::Unop& unop, const Tky::Value& dst) {
        // Construct the unique pointer to the unary operator and the target register
        AAst::Unop unaryOperator {generateUnop(unop)};
        AAst::Operand destination {generateOperand(dst)};

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
        AAstInstructionList finalInstructions;

        // Get the instruction type, and branch to the relevant function
        for (auto& instruction : instructionList) {
            std::visit(Ol::overloaded{
                [&finalInstructions](Tky::UnaryInstruction& inst) {
                    finalInstructions.push_back(generateMovInstruction(inst.src(), inst.dst()));
                    finalInstructions.push_back(generateUnopInstruction(inst.unop(), inst.dst()));
                },
                [&finalInstructions](Tky::BinaryInstruction& inst) {
                    // Work out if it's divide/ modulo or if its add/subtract/multiply
                    using namespace Token;

                    const std::string& binop {inst.binop().binop()};
                    Tky::VariableValue registerEax {AAst::registerStrings[AAst::AX]};

                    // If the binary operator needs to use the idiv command
                    if (binop == divideString || binop == moduloString) {
                        // Move the dividend into EAX
                        finalInstructions.push_back(generateMovInstruction(inst.src1(), registerEax));
                        // Sign extend the dividend
                        finalInstructions.push_back(generateCdqInstruction());
                        finalInstructions.push_back(generateIdivInstruction(inst.src2()));

                        if (binop == divideString) {
                            finalInstructions.push_back(generateMovInstruction(inst.src2(), registerEax));
                        }
                        else {
                            Tky::VariableValue registerEdx {AAst::registerStrings[AAst::DX]};
                            finalInstructions.push_back(generateMovInstruction(inst.src2(), registerEdx));
                        }
                    }
                    else {
                        finalInstructions.push_back(generateMovInstruction(inst.src1(), inst.dst()));
                        finalInstructions.push_back(generateBinopInstruction(inst.binop(), inst.src1(), inst.dst()));
                    }
                },
                [&finalInstructions](Tky::ReturnInstruction& inst) {
                    Tky::VariableValue registerDst {AAst::registerStrings[AAst::AX]};
                    finalInstructions.push_back(generateMovInstruction(inst.value(), registerDst));
                    finalInstructions.push_back(generateRetInstruction());
                }
            }, *instruction);
        }
        return finalInstructions;
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

    // Takes an instruction that has an operand as one of it's members, and member pointers to getter and setter for
    // that operand.
    template<typename Ti>
    void replacePseudoOperand(Ti& inst, AAst::Operand& (Ti::*getter)(), void (Ti::*setter)(AAst::Operand),
                               PrToOffsetMap& prToStackOffset) {
        AAst::Operand& op {(inst.*getter)()};
        if (isPseudoOperand(op)) {
            // Determine if the pseudoOperand has been recorded in the map
            AAst::PseudoOperand& pseudoOp {std::get<AAst::PseudoOperand>(op)};
            auto pseudoAddressPos {prToStackOffset.find(pseudoOp.pseudoAddress())};

            int stackOffsetValue;
            // If it has, get the value from the map
            if (pseudoAddressPos != prToStackOffset.end()) {
                stackOffsetValue = pseudoAddressPos->second;
            }
            // If it has not, update the latest stackoffset and create a new entry in the map
            else {
                int stackOffset {getStackOffset(-4)};
                prToStackOffset[pseudoOp.pseudoAddress()] = stackOffset;
                stackOffsetValue = stackOffset;
            }

            AAst::StackOperand stackOffsetOp {stackOffsetValue};
            (inst.*setter)(stackOffsetOp);
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
                    using AAst::MovInstruction;

                    auto toMoveG {&MovInstruction::toMove};
                    auto toMoveS {&MovInstruction::setToMove};
                    replacePseudoOperand(inst, toMoveG, toMoveS, prToStackOffset);

                    auto destinationG {&MovInstruction::destination};
                    auto destinationS {&MovInstruction::setDestination};
                    replacePseudoOperand(inst, destinationG, destinationS, prToStackOffset);
                },
                [&prToStackOffset](AAst::UnopInstruction& inst) -> void {
                    using AAst::UnopInstruction;

                    auto operandG {&UnopInstruction::operand};
                    auto operandS {&UnopInstruction::setOperand};
                    replacePseudoOperand(inst, operandG, operandS, prToStackOffset);
                },
                [&prToStackOffset](AAst::BinopInstruction& inst) -> void {
                    using AAst::BinopInstruction;

                    auto leftG {&BinopInstruction::left};
                    auto leftS {&BinopInstruction::setLeft};
                    replacePseudoOperand(inst, leftG, leftS, prToStackOffset);

                    auto rightG {&BinopInstruction::right};
                    auto rightS {&BinopInstruction::setRight};
                    replacePseudoOperand(inst, rightG, rightS, prToStackOffset);
                },
                [&prToStackOffset](AAst::IdivInstruction& inst) -> void {
                    using AAst::IdivInstruction;

                    auto operandG {&IdivInstruction::operand};
                    auto operandS {&IdivInstruction::setOperand};
                    replacePseudoOperand(inst, operandG, operandS, prToStackOffset);
                },
                [](AAst::CdqInstruction& inst) -> void {
                    // CdqInstructions do not contain pseudoregisters
                },
                [](AAst::RetInstruction& inst) -> void {
                    // RetInstructions do not contain pseudoregisters
                },
                [](AAst::StackallocInstruction& inst) -> void {
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
