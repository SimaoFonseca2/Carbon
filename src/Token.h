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
    semi
};

struct Token{
    TokenType type;
    std::optional<std::string> value;
};
#endif //CARBON_TOKEN_H
