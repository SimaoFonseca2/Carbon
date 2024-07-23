#ifndef CARBON_PARSER_HPP
#define CARBON_PARSER_HPP

#include <vector>
#include "Token.h"
#include "Allocator.hpp"
#include "Node.hpp"
#include <variant>


class Parser{
public:
    inline explicit Parser(const std::vector<Token>& tokens);
    [[nodiscard]] inline std::optional<Token> search(int offset) const;
    inline Token retrieve_token();
    std::optional<Node_expr*> Parse_expr(int min_prece);
    std::optional<Node_term*> Parse_term();
    std::optional<Node_stmt*> Parse_stmt();
    std::optional<Node_stmt_scope*> Parse_scope();
    std::optional<Node_prog> Parse_prog();
    static std::optional<int> find_prece(TokenType type);
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



std::optional<Node_expr*> Parser::Parse_expr(int min_prece = 0) {
    std::optional<Node_term*> term_lhs = Parse_term();
    if (!term_lhs.has_value()) {
        return {};
    }
    auto expr_lhs = m_allocator.alloc<Node_expr>();
    expr_lhs->expr_var = term_lhs.value();
    std::optional<int> prece;
    std::optional<Token> curr_token = search();

    while (true) {
        if (curr_token.has_value()) {
            prece = find_prece(curr_token.value().type);
            if (!prece.has_value() || prece < min_prece) {
                break;
            }
        } else {
            break;
        }

        Token op = retrieve_token();
        int next_prece = prece.value() + 1;
        auto expr_rhs = Parse_expr(next_prece);
        if (!expr_rhs.has_value()) {
            std::cerr << "Expression could not be parsed" << std::endl;
            exit(EXIT_FAILURE);
        }

        switch (op.type) {
            case TokenType::plus: {
                auto add = m_allocator.alloc<Node_OperationExprAdd>();
                add->lhs = expr_lhs;
                add->rhs = expr_rhs.value();

                auto op_expr = m_allocator.alloc<Node_OperationExpr>();
                op_expr->operation_var = add;

                auto new_expr_lhs = m_allocator.alloc<Node_expr>();
                new_expr_lhs->expr_var = op_expr;

                expr_lhs = new_expr_lhs;
                break;
            }
            case TokenType::times: {
                auto multi = m_allocator.alloc<Node_OperationExprMulti>();
                multi->lhs = expr_lhs;
                multi->rhs = expr_rhs.value();
                auto op_expr = m_allocator.alloc<Node_OperationExpr>();
                op_expr->operation_var = multi;
                auto new_expr_lhs = m_allocator.alloc<Node_expr>();
                new_expr_lhs->expr_var = op_expr;
                expr_lhs = new_expr_lhs;
                break;
            }
            case TokenType::slash: {
                auto div = m_allocator.alloc<Node_OperationExprDiv>();
                div->lhs = expr_lhs;
                div->rhs = expr_rhs.value();
                auto op_expr = m_allocator.alloc<Node_OperationExpr>();
                op_expr->operation_var = div;
                auto new_expr_lhs = m_allocator.alloc<Node_expr>();
                new_expr_lhs->expr_var = op_expr;
                expr_lhs = new_expr_lhs;
                break;
            }
            case TokenType::minus: {
                auto sub = m_allocator.alloc<Node_OperationExprSub>();
                sub->lhs = expr_lhs;
                sub->rhs = expr_rhs.value();
                auto op_expr = m_allocator.alloc<Node_OperationExpr>();
                op_expr->operation_var = sub;
                auto new_expr_lhs = m_allocator.alloc<Node_expr>();
                new_expr_lhs->expr_var = op_expr;
                expr_lhs = new_expr_lhs;
                break;
            }
            case TokenType::smaller: {

            }
            default:
                break;
        }
        curr_token = search();
    }
        return expr_lhs;
    }

std::optional<Node_stmt*> Parser::Parse_stmt() {
    while(search().has_value()){
        if(search().has_value() && search().value().type == TokenType::open_curly){
            if(std::optional<Node_stmt_scope*> scope = Parse_scope()){
                auto stmt = m_allocator.alloc<Node_stmt>();
                stmt->stmt_var = scope.value();
                return stmt;
            }
        }
        if(search().value().type == TokenType::_if){
            retrieve_token();
            auto stmt_if = m_allocator.alloc<Node_stmt_if>();
            if(search().has_value() && search().value().type == TokenType::open_paren){
                retrieve_token();
                if(search().has_value() && search().value().type == TokenType::ident && search(1).has_value() && search(1).value().type == TokenType::equals && search(2).has_value() &&
                   search(2).value().type==TokenType::equals && search(3).has_value() && search(3).value().type==TokenType::int_lit){
                    stmt_if->ident = retrieve_token();
                    retrieve_token();
                    retrieve_token();
                }
                if(std::optional<Node_expr*> expr = Parse_expr()){
                    stmt_if->expr = expr.value();
                }else{
                    std::cerr << "Failed to parse if statement expresion" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }else{
                std::cerr << "Missing '('" << std::endl;
                exit(EXIT_FAILURE);
            }
            if(search().has_value() && search().value().type == TokenType::close_paren){
                retrieve_token();
                if(search(1).has_value() && search(1).value().type == TokenType::close_curly){
                    std::cerr << "If scope cannot be empty" << std::endl;
                    exit(EXIT_FAILURE);
                }
                if(std::optional<Node_stmt_scope*> scope = Parse_scope()){
                    stmt_if->scope = scope.value();
                }else{
                    std::cerr << "Scope in if statement could not be parsed" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }else{
                std::cerr << "Missing ')'" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto stmt = m_allocator.alloc<Node_stmt>();
            stmt->stmt_var = stmt_if;
            return stmt;
        }
        if(search().value().type == TokenType::_for){
            auto stmt_for = m_allocator.alloc<Node_stmt_for>();
            retrieve_token();
            if(search().value().type == TokenType::open_paren){
                retrieve_token();
            }else{
                std::cerr << "Expected '('" << std::endl;
                exit(EXIT_FAILURE);
            }
            if(search().has_value() && search().value().type == TokenType::ident){
                auto stmt_ident = m_allocator.alloc<Node_stmt_ident>();
                stmt_ident->ident = retrieve_token();
                if(search().has_value() && search().value().type == TokenType::equals){
                    retrieve_token();
                }else{
                    std::cerr << "Expected '='" << std::endl;
                    exit(EXIT_FAILURE);
                }
                if (std::optional<Node_expr*> expr = Parse_expr()){
                    stmt_ident->expr = expr.value();
                }else{
                    std::cerr << "Invalid expression" << std::endl;
                    exit(EXIT_FAILURE);
                }
                if(search().has_value() && search().value().type == TokenType::semi){
                    retrieve_token();
                    stmt_for->first_expr_var = stmt_ident;
                }else{
                    std::cerr << "Expected ';'" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            if(search().has_value() && search().value().type == TokenType::_let && search(1).has_value() && search(1).value().type == TokenType::ident && search(2).has_value() &&
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
                stmt_for->first_expr_var = stmt_let;
            }
            if(search().has_value() && search().value().type == TokenType::ident && search(1).has_value() && search(2).has_value()){
                stmt_for->ident = retrieve_token();
                stmt_for->op.type = retrieve_token().type;
                stmt_for->op.value = retrieve_token().value;

                if(search().has_value() && search().value().type == TokenType::semi){
                    retrieve_token();
                }else{
                    std::cerr << "Expected ';'" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            if(search().has_value() && search().value().type == TokenType::ident && search(1).has_value() && search(1).value().type == TokenType::plus && search(2).has_value() && search(2).value().type == TokenType::plus){
                auto stmt_incr = m_allocator.alloc<Node_stmt_incr>();
                stmt_incr->ident = retrieve_token();
                retrieve_token();
                retrieve_token();
                stmt_for->third_expr_var = stmt_incr;
            }
            if(search().has_value() && search().value().type == TokenType::ident && search(1).has_value() && search(1).value().type == TokenType::minus && search(2).has_value() && search(2).value().type == TokenType::minus){
                auto stmt_decr = m_allocator.alloc<Node_stmt_decr>();
                stmt_decr->ident = retrieve_token();
                retrieve_token();
                retrieve_token();
                stmt_for->third_expr_var = stmt_decr;
            }
            if(search().has_value() && search().value().type == TokenType::close_paren){
                retrieve_token();
                if(search(1).has_value() && search(1).value().type == TokenType::close_curly){
                    std::cerr << "For scope cannot be empty" << std::endl;
                    exit(EXIT_FAILURE);
                }
                if(std::optional<Node_stmt_scope*> scope = Parse_scope()){
                    stmt_for->scope = scope.value();
                }else{
                    std::cerr << "Scope in if statement could not be parsed" << std::endl;
                    exit(EXIT_FAILURE);
                }
                auto node_stmt = m_allocator.alloc<Node_stmt>();
                node_stmt->stmt_var = stmt_for;
                return node_stmt;
            }else{
                std::cerr << "Expected ')'" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        if(search().value().type == TokenType::_print){
            auto node_stmt = m_allocator.alloc<Node_stmt>();
            retrieve_token();
            if(search().value().type == TokenType::open_paren){
                retrieve_token();
            }else{
                std::cerr << "Expected '('" << std::endl;
                exit(EXIT_FAILURE);
            }
            if(search().value().type == TokenType::quote){
                retrieve_token();
                auto node_stmt_print = m_allocator.alloc<Node_stmt_print>();
                node_stmt_print->msg.value = search().value().value;
                node_stmt_print->msg.type = search().value().type;
                node_stmt->stmt_var = node_stmt_print;
                retrieve_token();
                if(search().value().type == TokenType::quote){
                    retrieve_token();
                }else{
                    std::cerr << "Expected '\"'" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }else if(search().value().type == TokenType::ident){
                auto node_stmt_print = m_allocator.alloc<Node_stmt_print>();
                node_stmt_print->msg.value = search().value().value;
                node_stmt_print->msg.type = search().value().type;
                node_stmt->stmt_var = node_stmt_print;
                retrieve_token();
            }else{
                std::cerr << "Expected '\"'" << std::endl;
                exit(EXIT_FAILURE);
            }

            if(search().value().type == TokenType::close_paren){
                retrieve_token();
            }else{
                std::cerr << "Expected ')'" << std::endl;
                exit(EXIT_FAILURE);
            }
            if(search().value().type == TokenType::semi){
                retrieve_token();
            }else{
                std::cerr << "Expected ';'" << std::endl;
                exit(EXIT_FAILURE);
            }
            return node_stmt;
        }
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
        }else if(search().has_value() && search().value().type == TokenType::ident && search(1).has_value() && search(1).value().type == TokenType::plus && search(2).has_value() && search(2).value().type == TokenType::plus){
            auto stmt_incr = m_allocator.alloc<Node_stmt_incr>();
            stmt_incr->ident = retrieve_token();
            retrieve_token();
            retrieve_token();
            if(search().has_value() && search().value().type == TokenType::semi){
                retrieve_token();
                auto node_stmt = m_allocator.alloc<Node_stmt>();
                node_stmt->stmt_var = stmt_incr;
                return node_stmt;
            }else{
                std::cerr << "Expected ';'" << std::endl;
                exit(EXIT_FAILURE);
            }
        }else if(search().has_value() && search().value().type == TokenType::ident && search(1).has_value() && search(1).value().type == TokenType::minus && search(2).has_value() && search(2).value().type == TokenType::minus){
            auto stmt_decr = m_allocator.alloc<Node_stmt_decr>();
            stmt_decr->ident = retrieve_token();
            retrieve_token();
            retrieve_token();
            if(search().has_value() && search().value().type == TokenType::semi){
                retrieve_token();
                auto node_stmt = m_allocator.alloc<Node_stmt>();
                node_stmt->stmt_var = stmt_decr;
                return node_stmt;
            }else{
                std::cerr << "Expected ';'" << std::endl;
                exit(EXIT_FAILURE);
            }
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
    }else if(search().has_value() && search().value().type == TokenType::open_paren){
        retrieve_token();
        std::optional<Node_expr*> expr = Parse_expr();
        if(!expr.has_value()){
            std::cerr << "Expected an expression" << std::endl;
            exit(EXIT_FAILURE);
        }
        if(retrieve_token().type==TokenType::close_paren){
            auto term_paren = m_allocator.alloc<Node_term_paren>();
            term_paren->expr = expr.value();
            auto term = m_allocator.alloc<Node_term>();
            term->term_var=term_paren;
            return term;
        }else{
            std::cerr << "Expected ')'" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    else{
        return {};
    }
}

std::optional<int> Parser::find_prece(TokenType type) {
    switch(type){
        case TokenType::plus:
        case TokenType::minus:
            return 1;
        case TokenType::times:
        case TokenType::slash:
            return 2;
        default:
            return {};
    }
}

std::optional<Node_stmt_scope*> Parser::Parse_scope() {
    if(!search().has_value() && search()->type == TokenType::open_curly){
        return {};
    }
    retrieve_token();
    auto scope = m_allocator.alloc<Node_stmt_scope>();
    while(std::optional<Node_stmt*> stmt = Parse_stmt()){
        scope->stmts.push_back(stmt.value());
        if(search().has_value() && search()->type == TokenType::close_curly){
            retrieve_token();
            break;
        }
        if(!stmt.has_value()){
            std::cerr << "Invalid scope" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    return scope;
}


#endif //CARBON_PARSER_HPP
