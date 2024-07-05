#ifndef CARBON_TOKEN_H
#define CARBON_TOKEN_H

#include <optional>
#include <string>

enum class TokenType{
    _return,
    int_lit,
    _let,
    ident,
    equals,
    semi,
    plus,
    times,
    slash,
    minus,
    close_paren,
    open_paren,
    open_curly,
    close_curly,
    _if,
    bigger,//not implemented yet
    smaller,//not implemented yet
    _print,
    quote,
    string
};

struct Token{
    TokenType type;
    std::optional<std::string> value;
};
#endif //CARBON_TOKEN_H
