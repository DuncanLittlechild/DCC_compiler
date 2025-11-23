cmake_minimum_required(VERSION 3.28.3)
project(dcc)

set(CMAKE_CXX_STANDARD 23)

add_executable(dcc main.cpp
        lexer/lexer.cpp
        lexer/tokens.h
        lexer/lexer.h
        parser/ast.h
        parser/parser.cpp
        parser/parser.h
        assembly_generator/assembly_generator.cpp
        assembly_generator/assembly_ast.h
        assembly_generator/assembly_generator.h
        assembly_emitter/assembly_emitter.cpp
        assembly_emitter/assembly_emitter.h
        tacky/tacky.h
)
