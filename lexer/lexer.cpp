// Generate a list of tokens from an inputted file

#include "tokens.h"
#include <vector>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <functional>

namespace Lexer {
    static const std::regex initialWhitespace("^\\s+");

    void shrinkLine(std::string& line, std::smatch& match) {
        line = line.substr(match.position(0) + match.length());
    }

    // Iterate over file token by token, generating a vector of tokens as it goes.
    std::vector<Token::Token> lexFile(const std::filesystem::path& inputFile) {
        // Create the vector to be outputted
        std::vector<Token::Token> tokens;
        tokens.reserve(100);

        // Open the inputted file
        std::ifstream file { inputFile };

        // Iterate over the file line by line
        std::string line {};
        int lineCounter {0};
        while (std::getline(file, line)) {
            ++lineCounter;
            // While input isn't empty
            while (!line.empty()) {
                std::smatch match;
                //  if input starts with whitespace, trim it
                if (std::regex_search(line, match, initialWhitespace)) {
                    shrinkLine(line, match);
                }
                // else find longest regex match at start of input
                else {
                    int index {-1};
                    bool firstMatch {true};
                    bool matched {false};

                    // Iterate over the list of patterns to find the longest match
                    for (int i {0}; i < std::ssize(Token::patterns); ++i) {
                        std::smatch tmp;
                        // If the search returns anything, make sure it's the longest result
                        if (std::regex_search(line, tmp, Token::patterns.data()[i].first)) {
                            // Note that this relies on keywords being lower down in patterns than identifiers
                            if (tmp.length(0)>= match.length(0) || firstMatch) {
                                match = tmp;
                                index = i;
                            }
                            firstMatch = false;
                            matched = true;
                        }
                    }
                    // If there are no matches, exit with an error message
                    if (!matched) {
                        std::cout << "Incorrect syntax in line " << lineCounter << "\n";
                        exit(1);
                    }

                    // If there are, generate a token to match it and then append it to tokens
                    tokens.emplace_back(Token::patterns.data()[index].second(match));

                    shrinkLine(line, match);
                }
                // Iterate over list of regexes to find the longest match
                // if no match, raise error
                // convert match to token
                // remove match from input
            }
        }
        return tokens;


    }
}