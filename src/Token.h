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
    times
};

struct Token{
    TokenType type;
    std::optional<std::string> value;
};
#endif //CARBON_TOKEN_H
