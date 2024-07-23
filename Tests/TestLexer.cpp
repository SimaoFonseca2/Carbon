
#include <gtest/gtest.h>
#include "../src/Token.h"
#include "../src/Lexer.hpp"

std::string tokens_to_string(const std::vector<Token>& tokens) {
    std::string result;
    for (const auto& token : tokens) {
        result += "{" + std::to_string(static_cast<int>(token.type)) + ", " + token.value.value() + "} ";
    }
    return result;
}

TEST(LexerTest, EmptyInput) {
    Lexer lexer("");
    auto tokens = lexer.lex();
    EXPECT_TRUE(tokens.empty());
}

TEST(LexerTest, SimpleIdentifiers) {
    Lexer lexer("print let return for if");
    auto tokens = lexer.lex();
    std::vector<Token> expected = {
            {TokenType::_print, "print"},
            {TokenType::_let, "let"},
            {TokenType::_return, "return"},
            {TokenType::_for, "for"},
            {TokenType::_if, "if"}
    };
    EXPECT_EQ(tokens_to_string(tokens), tokens_to_string(expected));
}

TEST(LexerTest, SimpleNumbers) {
    Lexer lexer("123 456");
    auto tokens = lexer.lex();
    std::vector<Token> expected = {
            {TokenType::int_lit, "123"},
            {TokenType::int_lit, "456"}
    };
    EXPECT_EQ(tokens_to_string(tokens), tokens_to_string(expected));
}

TEST(LexerTest, SimpleOperators) {
    Lexer lexer("+-*/");
    auto tokens = lexer.lex();
    std::vector<Token> expected = {
            {TokenType::plus, "+"},
            {TokenType::minus, "-"},
            {TokenType::times, "*"},
            {TokenType::slash, "/"}
    };
    EXPECT_EQ(tokens_to_string(tokens), tokens_to_string(expected));
}

TEST(LexerTest, ParenthesesAndBraces) {
    Lexer lexer("(){}");
    auto tokens = lexer.lex();
    std::vector<Token> expected = {
            {TokenType::open_paren, "("},
            {TokenType::close_paren, ")"},
            {TokenType::open_curly, "{"},
            {TokenType::close_curly, "}"}
    };
    EXPECT_EQ(tokens_to_string(tokens), tokens_to_string(expected));
}

TEST(LexerTest, SimpleString) {
    Lexer lexer("\"hello\"");
    auto tokens = lexer.lex();
    std::vector<Token> expected = {
            {TokenType::quote, ""},
            {TokenType::string, "hello"},
            {TokenType::quote, ""}
    };
    EXPECT_EQ(tokens_to_string(tokens), tokens_to_string(expected));
}

TEST(LexerTest, RelationalOperators) {
    Lexer lexer("> >= < <=");
    auto tokens = lexer.lex();
    std::vector<Token> expected = {
            {TokenType::bigger, ">"},
            {TokenType::bigger_n_equal, ">="},
            {TokenType::smaller, "<"},
            {TokenType::smaller_n_equal, "<="}
    };
    EXPECT_EQ(tokens_to_string(tokens), tokens_to_string(expected));
}

TEST(LexerTest, MixedInput) {
    Lexer lexer("print 123 + let");
    auto tokens = lexer.lex();
    std::vector<Token> expected = {
            {TokenType::_print, "print"},
            {TokenType::int_lit, "123"},
            {TokenType::plus, "+"},
            {TokenType::_let, "let"}
    };
    EXPECT_EQ(tokens_to_string(tokens), tokens_to_string(expected));
}


