
#ifndef CARBON_NODE_HPP
#define CARBON_NODE_HPP

#include <vector>
#include <optional>
#include <variant>
#include "Token.h"

struct Node_expr;

struct Node_OperationExprAdd {
    Node_expr* lhs;
    Node_expr* rhs;
};

struct Node_OperationExprMulti {
    Node_expr* lhs;
    Node_expr* rhs;
};

struct Node_OperationExprDiv {
    Node_expr* lhs;
    Node_expr* rhs;
};

struct Node_OperationExprSub {
    Node_expr* lhs;
    Node_expr* rhs;
};

struct Node_OperationExpr {
    std::variant<Node_OperationExprAdd*, Node_OperationExprMulti*, Node_OperationExprDiv*, Node_OperationExprSub*> operation_var;
};

struct Node_term_intlit{
    Token int_lit;
};

struct Node_term_ident{
    Token ident;
};

struct Node_term_paren {
    Node_expr* expr;
};

struct Node_term {
    std::variant<Node_term_intlit*, Node_term_ident*, Node_term_paren*> term_var;
};

struct Node_expr{
    std::variant<Node_term*,Node_OperationExpr*> expr_var;
};

struct Node_stmt_return{
    Node_expr* expr;
};

struct Node_stmt_let{
    Token ident;
    Node_expr* expr{};
};

struct Node_stmt_ident{
    Token ident;
    Node_expr* expr{};
};

struct Node_stmt_print{
    Token msg;
};
struct Node_stmt_incr{
    Token ident;
};

struct Node_stmt_decr{
    Token ident;
};

struct Node_stmt;

struct Node_stmt_scope{
    std::vector<Node_stmt*> stmts;
};

struct Node_stmt_for{
    std::variant<Node_stmt_ident*, Node_stmt_let*> first_expr_var;
    Token ident;
    Token op;
    std::variant<Node_stmt_incr*,Node_stmt_decr*> third_expr_var;
    Node_stmt_scope* scope{};
};

struct Node_stmt_if{
    std::optional<Token> ident;
    Node_expr* expr{};
    Node_stmt_scope* scope{};
};



struct Node_stmt{
    std::variant<Node_stmt_return*, Node_stmt_let*, Node_stmt_ident*, Node_stmt_scope*, Node_stmt_if*, Node_stmt_print*, Node_stmt_incr*, Node_stmt_decr*, Node_stmt_for*> stmt_var;
};

struct Node_prog{
    std::vector<Node_stmt> stmts;
};


#endif //CARBON_NODE_HPP
