
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

inline std::vector<Token> Lexer::lex() {
    std::string buffer;
    while (search().has_value()) {
        if (std::isalpha(search().value())) {
            buffer.push_back(search().value());
            step();
            while (search().has_value() && std::isalnum(search().value())) {
                buffer.push_back(search().value());
                step();
            }
            if (buffer == "print") {
                tokens.push_back({TokenType::_print, buffer});
            }
            else if (buffer == "return") {
                tokens.push_back({TokenType::_return, buffer});
            } else if (buffer == "let") {
                tokens.push_back({TokenType::_let, buffer});
            }else if(buffer == "for"){
                tokens.push_back({TokenType::_for, buffer});
            }else if(buffer == "if"){
                tokens.push_back({TokenType::_if, buffer});
            }else {
                tokens.push_back({TokenType::ident, buffer});
            }
            buffer.clear();
        } else if (isdigit(search().value())) {
            while (search().has_value() && std::isdigit(search().value())) {
                buffer.push_back(search().value());
                step();
            }
            tokens.push_back({TokenType::int_lit, buffer});
            buffer.clear();
        } else {
            switch (search().value()) {
                case '+':
                    tokens.push_back({TokenType::plus, "+"});
                    break;
                case '-':
                    tokens.push_back({TokenType::minus, "-"});
                    break;
                case '*':
                    tokens.push_back({TokenType::times, "*"});
                    break;
                case '/':
                    tokens.push_back({TokenType::slash, "/"});
                    break;
                case '(':
                    tokens.push_back({TokenType::open_paren, "("});
                    break;
                case ')':
                    tokens.push_back({TokenType::close_paren, ")"});
                    break;
                case '=':
                    tokens.push_back({TokenType::equals, "="});
                    break;
                case ';':
                    tokens.push_back({TokenType::semi, ";"});
                    break;
                case '{':
                    tokens.push_back({TokenType::open_curly, "{"});
                    break;
                case '}':
                    tokens.push_back({TokenType::close_curly, "}"});
                    break;
                case '>':
                    if(search(1).has_value() && search(1).value() == '='){
                        tokens.push_back({TokenType::bigger_n_equal, ">="});
                        step();
                        break;
                    }else{
                        tokens.push_back({TokenType::bigger, ">"});
                        break;
                    }
                case '<':
                    if(search(1).has_value() && search(1).value() == '='){
                        tokens.push_back({TokenType::smaller_n_equal, "<="});
                        step();
                        break;
                    }else{
                        tokens.push_back({TokenType::smaller, "<"});
                        break;
                    }
                case '"':
                    tokens.push_back({TokenType::quote, ""});
                    step();
                    buffer.clear();
                    while (search().has_value() && search().value() != '"') {
                        buffer.push_back(search().value());
                        step();
                    }
                    tokens.push_back({TokenType::string, buffer});
                    buffer.clear();
                    step();
                    if(buffer.find('\"')){
                        tokens.push_back({TokenType::quote, buffer});
                    }
                    m_index--;
                    break;
                default:
                    break;
            }
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
        return m_source.at(m_index+offset);
    }
}

#endif //CARBON_LEXER_HPP
