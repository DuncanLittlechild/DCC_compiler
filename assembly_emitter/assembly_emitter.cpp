//
// Created by duncan on 11/15/25.
//

#include <iostream>
#include <fstream>

#include "../helpers/overload.h"
#include "assembly_emitter.h"

namespace AssemblyEmitter {
    using FilePath = std::filesystem::path;
    using AAstInstructionList = std::vector<std::unique_ptr<AAst::Instruction>>;

    std::string getOperandString(AAst::Operand& op) {
        return std::visit(Ol::overloaded{
            [](AAst::ImmOperand& op) -> std::string {
                return "$" + std::to_string(op.value());
            },
            [](AAst::RegisterOperand& op) -> std::string {
                return "%" + AAst::registerStrings[op.reg()];
            },
            [](AAst::StackOperand& op) -> std::string {
                return std::to_string(op.value()) + "(%rbp)";
            },
            [](AAst::PseudoOperand& op) -> std::string {
                throw std::runtime_error("PseudoOperand detected at the emission phase");
                return "";
            }
        }, op);
    }

    // Prints each instruction
    void emitFromMovInstruction(AAst::MovInstruction& inst, std::ofstream& outputFile) {
        outputFile << "\tmovq\t" << getOperandString(inst.toMove()) << ", " << getOperandString(inst.destination()) << "\n";
    }

    void emitFromUnopInstruction(AAst::UnopInstruction& inst, std::ofstream& outputFile) {
        outputFile << "\t" << AAst::unopStrings[inst.unop()] << "\t" << getOperandString(inst.operand()) << "\n";
    }

    // Prints the instructions
    void emitFromInstructions(const AAstInstructionList& instructions, std::ofstream& outputFile) {
        // Interate over the list of instructions and emit the appropriate code.
        // If any fail, return false immediately
        for (const auto& inst : instructions) {
            std::visit(Ol::overloaded{
                [&outputFile](AAst::MovInstruction& inst) -> void {
                    emitFromMovInstruction(inst, outputFile);
                },
                [&outputFile](AAst::UnopInstruction& inst) -> void {
                    emitFromUnopInstruction(inst, outputFile);
                },
                [&outputFile](AAst::StackallocInstruction& inst) -> void {
                    // Increment the stack pointer by the final size of the stack
                    outputFile << "\tsubq\t" << "$" << inst.stackSize() << ", %rsp\n";
                },
                [&outputFile](AAst::RetInstruction& inst) -> void {
                    // Deconstruct the stack frame and return
                    // Move the contents of the base pointer into the stack pointer
                    outputFile << "\tmovq\t" << "%rbp, %rsp\n";
                    // removes the value the stack pointer is pointing at and places it into the base pointer, then increments the
                    // stack pointer by the size of that value
                    outputFile << "\tpopq\t" << "%rbp\n";
                    // returns
                    outputFile << "\tret\n";
                },
            }, *inst
            );
        }
    }

    // Prints the start and end of the function
    void emitFromFunction(const AAst::Function& function, std::ofstream& outputFile) {
        // Print the function identifier
        const std::string functionName {function.identifier()};
        outputFile << "\t.globl " << functionName << "\n";
        outputFile << functionName << ":\n";

        // Constructs the stack frame
        outputFile << "\tpushq" << "\t%rbp\n";
        outputFile << "\tmovq" <<  "\t%rsp, %rbp\n";

        // traverse the instruction list
        emitFromInstructions(function.instructions(), outputFile);
    }

    // Prints the start and end of the program to the assembly file
    void emitFromProgram(AAst::Program& program, std::ofstream& outputFile) {
        emitFromFunction(program.function(), outputFile);

        // line to ensure the stack is non-executable
        outputFile << ".section .note.GNU-stack,\"\",@progbits";
    }


    // Loops over an assembly AST and uses it to generate an executable file of assembly code
    bool emitAssembly(AAst::Program& program, FilePath& filepath) {
        // Create outputFile
        std::ofstream outputFile {filepath};
        if (!outputFile) {
            std::cout << "failed to create assembly file at " << filepath << "\n";
            return false;
        }

        // Entry point for tree traversal
        emitFromProgram(program, outputFile);
        return true;
    }
}