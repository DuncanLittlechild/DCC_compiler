//
// Created by duncan on 11/15/25.
//

#ifndef DCC_ASSEMBLY_EMITTER_H
#define DCC_ASSEMBLY_EMITTER_H
#include <filesystem>

#include "../assembly_generator/assembly_ast.h"

namespace AssemblyEmitter {
  using FilePath = std::filesystem::path;
    using AAstInstructionList = std::vector<std::unique_ptr<AAst::Instruction>>;

    std::string getOperandString(AAst::Operand& op);

    // Prints each instruction
    void emitFromMovInstruction(AAst::MovInstruction& inst, std::ofstream& outputFile);

    void emitFromUnopInstruction(AAst::UnopInstruction& inst, std::ofstream& outputFile);

    // Prints the instructions
    void emitFromInstructions(const AAstInstructionList& instructions, std::ofstream& outputFile);

    // Prints the start and end of the function
    void emitFromFunction(const AAst::Function& function, std::ofstream& outputFile);

    // Prints the start and end of the program to the assembly file
    void emitFromProgram(AAst::Program& program, std::ofstream& outputFile);

    // Loops over an assembly AST and uses it to generate an executable file of assembly code
    bool emitAssembly(AAst::Program& program, FilePath& filepath);
}
#endif //DCC_ASSEMBLY_EMITTER_H