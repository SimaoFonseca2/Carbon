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
    bigger,
    smaller,
    _print,
    quote,
    string,
    _for,
    bigger_n_equal,
    smaller_n_equal
};

struct Token{
    TokenType type;
    std::optional<std::string> value;
};
#endif //CARBON_TOKEN_H
