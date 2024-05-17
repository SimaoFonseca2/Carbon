#ifndef CARBON_PARSER_HPP
#define CARBON_PARSER_HPP

#include <vector>
#include "Token.h"
#include "Allocator.hpp"
#include <variant>
struct Node_expr;

struct Node_OperationExprAdd {
    Node_expr* lhs;
    Node_expr* rhs;
};

struct Node_OperationExprMulti {
    Node_expr* lhs;
    Node_expr* rhs;
};

struct Node_OperationExpr {
    std::variant<Node_OperationExprAdd*, Node_OperationExprMulti*> operation_var;
};

struct Node_term_intlit{
    Token int_lit;
};

struct Node_term_ident{
    Token ident;
};

struct Node_term {
    std::variant<Node_term_intlit*, Node_term_ident*> term_var;
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

struct Node_stmt{
    std::variant<Node_stmt_return*, Node_stmt_let*, Node_stmt_ident*> stmt_var;
};

struct Node_prog{
    std::vector<Node_stmt> stmts;
};




class Parser{
public:
    inline explicit Parser(const std::vector<Token>& tokens);
    [[nodiscard]] inline std::optional<Token> search(int offset) const;
    inline Token retrieve_token();
    std::optional<Node_expr*> Parse_expr();
    std::optional<Node_OperationExpr*> Parse_operation_expr();
    std::optional<Node_term*> Parse_term();
    std::optional<Node_stmt*> Parse_stmt();
    std::optional<Node_prog> Parse_prog();
private:
    std::vector<Token> m_tokens;
    int m_index=0;
    Allocator m_allocator;
};



Parser::Parser(const std::vector<Token>& tokens):m_tokens(tokens), m_allocator(1024*1024*5) {}

std::optional<Token> Parser::search(int offset = 0) const {
    if(m_index+offset >= m_tokens.size()){
        return {};
    }
    else{
        return m_tokens.at(m_index+offset);
    }
}

Token Parser::retrieve_token() {
    return m_tokens.at(m_index++);
}


std::optional<Node_expr*> Parser::Parse_expr() {
        if(auto term = Parse_term()){
            if(search().has_value() && search().value().type==TokenType::plus){
                auto op_expr = m_allocator.alloc<Node_OperationExpr>();
                    auto op_expr_add = m_allocator.alloc<Node_OperationExprAdd>();
                    auto lhs_expr = m_allocator.alloc<Node_expr>();
                    lhs_expr->expr_var=term.value();
                    op_expr_add->lhs = lhs_expr;
                    retrieve_token();
                    if(auto rhs = Parse_expr()){
                        op_expr_add->rhs = rhs.value();
                        op_expr->operation_var = op_expr_add;
                        auto rhs_expr = m_allocator.alloc<Node_expr>();
                        rhs_expr->expr_var=op_expr;
                        return rhs_expr;
                    }
            }else if(search().has_value() && search().value().type==TokenType::times){
                auto op_expr = m_allocator.alloc<Node_OperationExpr>();
                auto op_expr_multi = m_allocator.alloc<Node_OperationExprMulti>();
                auto lhs_expr = m_allocator.alloc<Node_expr>();
                lhs_expr->expr_var=term.value();
                op_expr_multi->lhs = lhs_expr;
                retrieve_token();
                if(auto rhs = Parse_expr()){
                    op_expr_multi->rhs = rhs.value();
                    op_expr->operation_var = op_expr_multi;
                    auto rhs_expr = m_allocator.alloc<Node_expr>();
                    rhs_expr->expr_var=op_expr;
                    return rhs_expr;
                }
            }else{
                auto expr = m_allocator.alloc<Node_expr>();
                expr->expr_var = term.value();
                return expr;
            }
        }
    }

std::optional<Node_stmt*> Parser::Parse_stmt() {
    while(search().has_value()){
        if(search().value().type == TokenType::_return){
            retrieve_token();
            auto node_stmt_return = m_allocator.alloc<Node_stmt_return>();
            if(std::optional<Node_expr*> expr = Parse_expr()){
                node_stmt_return->expr = expr.value();
            }else{
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if(search().has_value() && search().value().type == TokenType::semi){
                retrieve_token();
            }else{
                std::cerr << "Semi colon expected" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto node_stmt = m_allocator.alloc<Node_stmt>();
            node_stmt->stmt_var=node_stmt_return;
            return node_stmt;
        }else if(search().has_value() && search().value().type == TokenType::_let && search(1).has_value() && search(1).value().type == TokenType::ident && search(2).has_value() &&
                search(2).value().type==TokenType::equals && search(3).has_value()){
            retrieve_token();
            auto stmt_let = m_allocator.alloc<Node_stmt_let>();
            stmt_let->ident=retrieve_token();
            retrieve_token();
            if (std::optional<Node_expr*> expr = Parse_expr()){
                stmt_let->expr = expr.value();
            }else{
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if(search().has_value() && search().value().type == TokenType::semi){
                retrieve_token();
            }else{
                std::cerr << "Expected ';'" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto node_stmt = m_allocator.alloc<Node_stmt>();
            node_stmt->stmt_var=stmt_let;
            return node_stmt;
        }else if(search().has_value() && search().value().type == TokenType::ident && search(1).has_value() && search(1).value().type == TokenType::equals &&
                search(2).has_value() && search(2).value().type == TokenType::int_lit || search(2).value().type == TokenType::ident && search(3).has_value()){
            auto stmt_ident = m_allocator.alloc<Node_stmt_ident>();
            stmt_ident->ident = retrieve_token();
            retrieve_token();
            if (std::optional<Node_expr*> expr = Parse_expr()){
                stmt_ident->expr = expr.value();
            }else{
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            retrieve_token();
            auto node_stmt = m_allocator.alloc<Node_stmt>();
            node_stmt->stmt_var = stmt_ident;
            return node_stmt;
        }
    }

}

std::optional<Node_prog> Parser::Parse_prog() {
    Node_prog prog;
    while(search().has_value()){
        if(std::optional<Node_stmt*> stmt = Parse_stmt()){
            prog.stmts.push_back(*stmt.value());
        }else{
          std::cerr << "Invalid statement" << std::endl;
          exit(EXIT_FAILURE);
        }
    }
    return prog;
}

std::optional<Node_OperationExpr*> Parser::Parse_operation_expr() {
    if (auto lhs = Parse_expr()){
        auto op_expr = m_allocator.alloc<Node_OperationExpr>();
        if (search().has_value() && search().value().type == TokenType::plus){
            auto op_expr_add = m_allocator.alloc<Node_OperationExprAdd>();
            op_expr_add->lhs = lhs.value();
            retrieve_token();
            if(auto rhs = Parse_expr()){
                op_expr_add->rhs = rhs.value();
                op_expr->operation_var = op_expr_add;
                retrieve_token();
                return op_expr;
            }else{
                std::cerr << "Expected expression" << std::endl;
            }
        }
    }else{
        return {};
    }
}

std::optional<Node_term*> Parser::Parse_term() {
    if(search().has_value() && search().value().type == TokenType::int_lit){
        auto term_intlit = m_allocator.alloc<Node_term_intlit>();
        term_intlit->int_lit=retrieve_token();
        auto term = m_allocator.alloc<Node_term>();
        term->term_var=term_intlit;
        return term;
    }else if(search().has_value() && search().value().type == TokenType::ident){
        auto node_term_ident = m_allocator.alloc<Node_term_ident>();
        node_term_ident->ident=retrieve_token();
        auto node_term = m_allocator.alloc<Node_term>();
        node_term->term_var=node_term_ident;
        return node_term;
    }
    else{
        return {};
    }
}


#endif //CARBON_PARSER_HPP
