//
// Created by duncan on 11/15/25.
//

#include <iostream>
#include <fstream>
#include <sstream>

#include "assembly_emitter.h"

namespace AssemblyEmitter {
    using FilePath = std::filesystem::path;
    using InstructionList = std::vector<std::unique_ptr<AAst::Instruction>>;

    // Prints each instruction
    void emitFromMovInstruction(AAst::MovInstruction& instruction, std::ofstream& outputFile) {
        outputFile << "movl " << instruction.toMove().opString() << ", "
            << instruction.destination().opString();
    }

    // Prints the instructions
    void emitFromInstructions(const InstructionList& instructions, std::ofstream& outputFile) {
        // Interate over the list of instructions and emit the appropriate code.
        // If any fail, return false immediately
        for (const auto& instruction : instructions) {
            outputFile << "\t";

            switch (instruction->type()) {
                case AAst::MovInstructionT:
                    emitFromMovInstruction(dynamic_cast<AAst::MovInstruction&>(*instruction), outputFile);
                    break;

                case AAst::RetInstructionT:
                    outputFile << "ret";
                    break;

                default:
                    std::stringstream errorMessage {};
                    errorMessage << "Instruction Type: " << AAst::nodeTypeStrings[instruction->type()] << " is not supported";
                    throw std::runtime_error(errorMessage.str());
            }

            outputFile << "\n";
        }
    }

    // Prints the start and end of the function
    void emitFromFunction(const AAst::Function& function, std::ofstream& outputFile) {
        // Print the function identifier
        const std::string functionName {function.identifier().identifier()};
        outputFile << "\t.globl " << functionName << "\n";
        outputFile << functionName << ":\n";

        // traverse the instruction list
        emitFromInstructions(function.instructions(), outputFile);
    }

    // Prints the start and end of the program to the assembly file
    void emitFromProgram(AAst::Program& program, std::ofstream& outputFile) {
        emitFromFunction(program.function(), outputFile);
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