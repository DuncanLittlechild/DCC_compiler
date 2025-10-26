#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <filesystem>
#include <cstdio>

constexpr std::string_view g_stopAtLexStr{ "--lex"};
constexpr char g_stopAtLexCode {'l'};

constexpr std::string_view g_stopAtParseStr { "--parse"};
constexpr char g_stopAtParseCode {'p'};

constexpr std::string_view g_stopAtCodegenStr { "--codegen"};
constexpr char g_stopAtCodegenCode {'c'};

constexpr std::string_view g_stopAtEmissionStr {"-S"};
constexpr char g_stopAtEmissionCode {'e'};



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
    std::filesystem::path fileName {argv[1]};
    if (fileName.extension().string() != ".c") {
        std::cout<<"File must be a .c file";
        return 1;
    }

    // Check that file exists at the chosen location
    std::filesystem::path file {argv[1]};
    if (!std::filesystem::exists(file)) {
        std::cout<<"File "<<file<<" could not be found\n";
        return 1;
    }

    // Run preprocessor
    // generate string for new file name, replacing .c with .i
    std::filesystem::path preprocessedFileName {fileName};
    preprocessedFileName.replace_extension(".i");

    // Construct the string, then execute it as a command line prompt
    std::stringstream ss {};
    ss << "gcc -E -P "<< fileName <<" -o " << preprocessedFileName;
    std::string  preprocessCommand {ss.str()};
    int result {std::system(preprocessCommand.c_str())};

    // If the command line prompt could not be executed, error and exit
    if (result) {
        std::cout << "Error: gcc preprocess aborted with error code "<< result <<"\n";
        return 1;
    }

    // Run compiler
    // For now, just use gcc
    // generate string for compiled filename
    std::filesystem::path compiledFileName {preprocessedFileName};
    compiledFileName.replace_extension(".s");

    //Reset stringstream buffer
    ss.str("");
    ss << "gcc -S "<<preprocessedFileName<< " -o "<< compiledFileName;
    std::string compileCommand {ss.str()};

    result = std::system(compileCommand.c_str());
    if (result) {
        std::cout << "Error: gcc compile aborted with error code "<< result <<"\n";
        return 1;
    }

    // Run lexer
    
    // Run parser
    // Generate Assembly
    // delete preprocessed file
    result = std::remove(preprocessedFileName.c_str());
    if (result) {
        std::cout << "Error: preprocessed file not deleted with error code "<< result <<"\n";
        return 1;
    }

    // Run assembler and linker
    std::filesystem::path outputFileName {compiledFileName};
    outputFileName.replace_extension();

    ss.str("");
    ss << "gcc "<<compiledFileName<<" -o "<< outputFileName;
    std::string outputCommand {ss.str()};

    result = std::system(outputCommand.c_str());
    if (result) {
        std::cout << "Error: gcc assembly and linking aborted with error code" << result << "\n";
        return 1;
    }

    //delete compiledFile
    result = std::remove( compiledFileName.c_str());
    if (result) {
        std::cout << "Error: compiled file not deleted with error code "<< result <<"\n";
        return 1;
    }


    return 0;
}