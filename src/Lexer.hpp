
#ifndef CARBON_LEXER_HPP
#define CARBON_LEXER_HPP
#include <iostream>
#include "Token.h"
#include <vector>
class Lexer{
public:
    inline explicit Lexer(std::string source);
    inline std::vector<Token> lex();
    [[nodiscard]] inline std::optional<char> search() const;
    [[nodiscard]] inline std::optional<char> search(int offset) const;
    inline void step();
private:
    std::vector<Token> tokens;
    const std::string m_source;
    int m_index=0;

};

inline std::vector <Token> Lexer::lex() {
    std::string buffer;
    while(search().has_value()){
        if(std::isalpha(search().value())){
            buffer.push_back(search().value());
            step();
        }
        while(search().has_value() && std::isalpha(search().value())){
            buffer.push_back(search().value());
            step();
        }
        if(buffer=="return"){
            tokens.push_back({.type=TokenType::_return,.value=buffer});
            buffer.clear();
        }
        if(buffer=="let"){
            tokens.push_back({.type=TokenType::_let,.value=buffer});
            buffer.clear();
        }
        if(search().value()=='='){
            tokens.push_back({.type=TokenType::equals,.value=buffer});
            step();
        }
        if(isspace(search().value())){
            step();
        }
        if(isalpha(search().value())){
            while(std::isalpha(search().value())){
                buffer.push_back(search().value());
                step();
            }
            if(buffer=="return"){
                tokens.push_back({.type=TokenType::_return,.value=buffer});
                buffer.clear();
            }else if(buffer=="let"){
                tokens.push_back({.type=TokenType::_let,.value=buffer});
                buffer.clear();
            }else{
                tokens.push_back({.type=TokenType::ident,.value=buffer});
            }
            buffer.clear();
        }

        if(isdigit(search().value())){
            while(std::isdigit(search().value())){
                buffer.push_back(search().value());
                step();
            }
            tokens.push_back({.type=TokenType::int_lit,.value=buffer});
            buffer.clear();
        }
        if(search().value() == '+'){
            tokens.push_back({.type=TokenType::plus,.value=buffer});
            step();
        }else if(search().value() == '*'){
            tokens.push_back({.type=TokenType::times,.value=buffer});
            step();
        }
        if(search().value()==';'){
            tokens.push_back({.type=TokenType::semi,.value=buffer});
            step();
        }
    }

    return tokens;
}



inline Lexer::Lexer(std::string source):m_source(std::move(source)) {}

std::optional<char> Lexer::search() const {
    if(m_index+1 > m_source.length()){
        return {};
    }
    else{
        return m_source.at(m_index);
    }
}

void Lexer::step() {
    m_index++;
}

std::optional<char> Lexer::search(int offset = 1) const {
    if(m_index+offset > m_source.length()){
        return {};
    }
    else{
        return m_source.at(m_index);
    }
}

#endif //CARBON_LEXER_HPP
