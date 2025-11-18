#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <filesystem>
#include <cstdio>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "assembly_generator/assembly_generator.h"
#include "assembly_emitter/assembly_emitter.h"

constexpr std::string_view g_stopAtLexStr{ "--lex"};
constexpr char g_stopAtLexCode {'l'};

constexpr std::string_view g_stopAtParseStr { "--parse"};
constexpr char g_stopAtParseCode {'p'};

constexpr std::string_view g_stopAtCodegenStr { "--codegen"};
constexpr char g_stopAtCodegenCode {'c'};

constexpr std::string_view g_stopAtEmissionStr {"-S"};
constexpr char g_stopAtEmissionCode {'e'};

using FilePath = std::filesystem::path;

void runPreprocessor(const FilePath& fileName, const FilePath& preprocessedFileName) {
    // Construct the string, then execute it as a command line prompt
    std::stringstream ss {};
    ss << "gcc -E -P "<< fileName <<" -o " << preprocessedFileName;
    std::string  preprocessCommand {ss.str()};
    int result {std::system(preprocessCommand.c_str())};
    // If the command line prompt could not be executed, error and exit
    if (result) {
        std::cout << "Error: gcc preprocess aborted with error code "<< result <<"\n";
        throw std::runtime_error("gcc preprocess aborted");
    }
}

int main(const int argc, char* argv[]) {
    // Process command line arguments
    // If too few arguments, exit with error code
    if (argc <= 1) {
        if (argv[0]) {
            std::cout << "Usage: " << argv[0] << " path/to/file.c --option";
        } else {
            std::cout<<"Usage: ./dcc path/to/file.c --option";
        }
        return 1;
    }

    // If too many arguments, exit with error code
    // Three arguments allows for options
    if (argc > 3) {
        std::cout<<"Too many arguments";
        return 1;
    }

    // set flags for the different stages of the compiler
    // n means no exitcode. 'l' is exit at lexer, 'p' is exit at parser, 'c' is exit at codegen, and 'e' is exit at
    // emission.
    char stopCode{'n'};

    // Ensure that optional arguments use valid syntax
    if (argc == 3) {
        // Parse argv[2] to ensure it is correct
        std::string option = argv[2];

        if (option == g_stopAtLexStr) {
            stopCode = g_stopAtLexCode;
        } else if (option == g_stopAtParseStr ) {
            stopCode = g_stopAtParseCode;
        } else if (option == g_stopAtCodegenStr) {
            stopCode = g_stopAtCodegenCode;
        } else if (option == g_stopAtEmissionStr) {
            stopCode = g_stopAtEmissionCode;
        } else {
            // If the option is not valid, exit with an error code
            std::cout <<"Error: unrecognised option. Valid options are: " << g_stopAtLexStr << ", " << g_stopAtParseStr
            << ", " << g_stopAtCodegenStr << ", " << g_stopAtEmissionStr << ". \n";
            return 1;
        }
    }

    //Check that the filename is a c file
    const FilePath fileName {argv[1]};
    if (fileName.extension().string() != ".c") {
        std::cout<<"File must be a .c file";
        return 1;
    }

    // Check that file exists at the chosen location
    if (!std::filesystem::exists(fileName)) {
        std::cout<<"File "<< fileName <<" could not be found\n";
        return 1;
    }


    // Run preprocessor
    // generate string for new file name, replacing .c with .i
    FilePath preprocessedFileName {fileName};
    preprocessedFileName.replace_extension(".i");

    // Construct the string, then execute it as a command line prompt
    try {
        runPreprocessor(fileName, preprocessedFileName);
    } catch (const std::runtime_error& preProcessorError){
        std::cout << "Preprocessor failed";
        return 1;
    }

    // Run compiler
    std::vector<Token::Token> tokens {Lexer::lexFile(preprocessedFileName)};

    // check stopCode
    if (stopCode == g_stopAtLexCode) {
        std::cout << "Stopped at lexer";
        return 0;
    }

    // Run parser
    Ast::Program abstractSyntaxTree;
    try {
        abstractSyntaxTree = Parser::parseProgram(tokens);
    } catch (const std::runtime_error& syntaxTreeError) {
        std::cout << syntaxTreeError.what();
        return 1;
    }

    if (stopCode == g_stopAtParseCode) {
        std::cout << "Stopped at parser";
        return 0;
    }

    // Convert C Ast to assembly Ast
    // TODO: add a type member to all base classes that can be used to determine what type to dynamic_cast to
    AAst::Program assemblyAbstractSyntaxTree{AssemblyGenerator::parseProgram(abstractSyntaxTree)};

    // For now, just use gcc
    // generate string for compiled filename


    FilePath compiledFileName {preprocessedFileName};
    compiledFileName.replace_extension(".s");

    // Generate Assembly
    try {
        AssemblyEmitter::emitAssembly(assemblyAbstractSyntaxTree, compiledFileName);
    } catch (std::runtime_error& syntaxError) {
        std::cout << syntaxError.what();
        return 1;
    }

    // delete preprocessed file
    int result = std::remove(preprocessedFileName.c_str());
    if (result) {
        std::cout << "Error: preprocessed file not deleted with error code "<< result <<"\n";
        return 1;
    }


    return 0;
}