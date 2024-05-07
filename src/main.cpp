#include<iostream>
#include<fstream>
#include<string>
#include <sstream>
#include "Lexer.hpp"
#include "Parser.hpp"
#include "AssemblyGen.hpp"

int main(int argc, char* argv[]){
    if (argc !=2){
        std::cerr<<"Invalid usage, correct usage is..."<<std::endl;
        std::cerr<<"Carbon <input.cb>"<<std::endl;
    }

    std::string source;
    {
        std::stringstream source_stream;
        std::ifstream file;
        std::fstream input(argv[1],std::ios::in);
        source_stream << input.rdbuf();
        source = source_stream.str();
        file.close();
    }
    Lexer lexer(std::move(source));
    Parser parser(lexer.lex());
    std::optional<Node_prog> program = parser.Parse_prog();
    if(!program){
        std::cerr << "Invalid program" << std::endl;
    }
    AssemblyGen assemblyGen(program.value());
    {
        std::fstream file("out.asm", std::ios::out);
        file << assemblyGen.gen_prog();
    }
    system("nasm -felf64 out.asm");
    system("ld -o out out.o");
    return 0;
}