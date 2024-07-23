#include<gtest/gtest.h>
#include "../src/Parser.hpp"
class TestParser : public ::testing::Test {};

TEST_F(TestParser,ParseStmtLet){
    std::vector<Token> tokens = {
            {TokenType::_let, "let"},
            {TokenType::ident, "x"},
            {TokenType::equals, "="},
            {TokenType::int_lit, "3"},
            {TokenType::semi, ";"}
    };
    Parser parser(tokens);
    auto stmt = parser.Parse_stmt();
    assert(stmt.has_value());
    auto stmt_let = std::get<Node_stmt_let*>(stmt.value()->stmt_var);
    assert(stmt_let->ident.value == "x");
    auto expr_term = std::get<Node_term*>(stmt_let->expr->expr_var);
    auto expr_term_int = std::get<Node_term_intlit*>(expr_term->term_var);
    EXPECT_TRUE(expr_term_int->int_lit.value == "3");
}

TEST_F(TestParser,ParseStmtEmptyIf){
    std::vector<Token> tokens = {
            {TokenType::_if, "if"},
            {TokenType::open_paren, "("},
            {TokenType::ident, "x"},
            {TokenType::equals, "="},
            {TokenType::equals, "="},
            {TokenType::int_lit, "1"},
            {TokenType::close_paren, ")"},
            {TokenType::open_curly, "{"},
            {TokenType::close_curly, "}"}
    };
    Parser parser(tokens);
    int status, exit_status;
    pid_t pid = fork();
    if (pid == 0) {
        parser.Parse_stmt();
        _exit(EXIT_SUCCESS);
    } else {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            exit_status = WEXITSTATUS(status);
        } else {
            exit_status = -1;
        }
    }
    EXPECT_EQ(exit_status, WEXITSTATUS(status));
}

TEST_F(TestParser,ParseStmtIf){
    std::vector<Token> tokens = {
            {TokenType::_if, "if"},
            {TokenType::open_paren, "("},
            {TokenType::ident, "x"},
            {TokenType::equals, "="},
            {TokenType::equals, "="},
            {TokenType::int_lit, "1"},
            {TokenType::close_paren, ")"},
            {TokenType::open_curly, "{"},
            {TokenType::_return, "return"},
            {TokenType::int_lit, "3"},
            {TokenType::semi, ";"},
            {TokenType::close_curly, "}"}
    };
    Parser parser(tokens);
    auto stmt = parser.Parse_stmt();
    EXPECT_TRUE(stmt.has_value());
    auto stmt_if = std::get<Node_stmt_if*>(stmt.value()->stmt_var);
    EXPECT_EQ(stmt_if->ident->value, "x");
    auto stmt_if_term = std::get<Node_term*>(stmt_if->expr->expr_var);
    auto stmt_if_term_int = std::get<Node_term_intlit*>(stmt_if_term->term_var);
    EXPECT_EQ(stmt_if_term_int->int_lit.value, "1");
    auto stmt_if_scope = std::get<Node_stmt_return*>(stmt_if->scope->stmts.front()->stmt_var);
    auto stmt_if_scope_term = std::get<Node_term*>(stmt_if_scope->expr->expr_var);
    auto stmt_if_scope_term_int = std::get<Node_term_intlit*>(stmt_if_scope_term->term_var);
    EXPECT_EQ(stmt_if_scope_term_int->int_lit.value, "3");
}


TEST_F(TestParser,ParseEmptyStmtFor){
    std::vector<Token> tokens = {
            {TokenType::_for, "for"},
            {TokenType::open_paren, "("},
            {TokenType::ident, "x"},
            {TokenType::equals, "="},
            {TokenType::int_lit, "0"},
            {TokenType::semi, ";"},
            {TokenType::ident, "x"},
            {TokenType::smaller, "<"},
            {TokenType::int_lit, "2"},
            {TokenType::semi, ";"},
            {TokenType::ident, "x"},
            {TokenType::plus, "+"},
            {TokenType::plus, "+"},
            {TokenType::close_paren, ")"},
            {TokenType::open_curly, "{"},
            {TokenType::close_curly, "}"}
    };
    Parser parser(tokens);
    int status, exit_status;
    pid_t pid = fork();
    if (pid == 0) {
        parser.Parse_stmt();
        _exit(EXIT_SUCCESS);
    } else {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
           exit_status = WEXITSTATUS(status);
        } else {
            exit_status = -1;
        }
    }
    EXPECT_EQ(exit_status, WEXITSTATUS(status));
}


TEST_F(TestParser, ParseStmtReturn) {
    std::vector<Token> tokens = {
            {TokenType::_return, "return"},
            {TokenType::int_lit, "3"},
            {TokenType::semi, ";"}
    };
    Parser parser(tokens);
    auto stmt = parser.Parse_stmt();
    EXPECT_TRUE(stmt.has_value());
    auto stmt_return = std::get<Node_stmt_return*>(stmt.value()->stmt_var);
    auto return_stmt_term = std::get<Node_term*>(stmt_return->expr->expr_var);
    auto return_stmt_int = std::get<Node_term_intlit*>(return_stmt_term->term_var);
    EXPECT_TRUE(return_stmt_int->int_lit.value == "3");
}

TEST_F(TestParser, ParseTermInt) {
    std::vector<Token> tokens = { {TokenType::int_lit, "5"} };
    Parser parser(tokens);
    auto term = parser.Parse_term();
    EXPECT_TRUE(term.has_value());
    auto term_intlit = std::get<Node_term_intlit*>(term.value()->term_var);
    EXPECT_TRUE(term_intlit->int_lit.value == "5");
}

TEST_F(TestParser, ParseTermIdent) {
    std::vector<Token> tokens = { {TokenType::ident, "x"} };
    Parser parser(tokens);
    auto term = parser.Parse_term();
    EXPECT_TRUE(term.has_value());
    auto term_intlit = std::get<Node_term_ident*>(term.value()->term_var);
    EXPECT_TRUE(term_intlit->ident.value == "x");
}

TEST_F(TestParser, ParseExprAdd) {
    std::vector<Token> tokens = {
            {TokenType::int_lit, "5"},
            {TokenType::plus, "+"},
            {TokenType::int_lit, "3"}
    };
    Parser parser(tokens);
    auto expr = parser.Parse_expr();
    EXPECT_TRUE(expr.has_value());
    auto expr_op = std::get<Node_OperationExpr*>(expr.value()->expr_var);
    auto add_expr = std::get<Node_OperationExprAdd*>(expr_op->operation_var);
    auto lhs_term = std::get<Node_term*>(add_expr->lhs->expr_var);
    auto rhs_term = std::get<Node_term*>(add_expr->rhs->expr_var);
    auto lhs = std::get<Node_term_intlit*>(lhs_term->term_var);
    auto rhs = std::get<Node_term_intlit*>(rhs_term->term_var);
    EXPECT_TRUE(lhs->int_lit.value == "5");
    EXPECT_TRUE(rhs->int_lit.value == "3");
}

TEST_F(TestParser, ParseExprAddOperation_n_Term) {
    std::vector<Token> tokens = {
            {TokenType::int_lit, "5"},
            {TokenType::plus, "+"},
            {TokenType::int_lit, "3"},
            {TokenType::plus, "+"},
            {TokenType::int_lit, "5"},
    };
    Parser parser(tokens);
    auto expr = parser.Parse_expr();
    EXPECT_TRUE(expr.has_value());
    auto expr_op = std::get<Node_OperationExpr*>(expr.value()->expr_var);
    auto add_expr = std::get<Node_OperationExprAdd*>(expr_op->operation_var);
    auto lhs_op_expr = std::get<Node_OperationExpr*>(add_expr->lhs->expr_var);
    auto lhs_add_expr = std::get<Node_OperationExprAdd*>(lhs_op_expr->operation_var);
    auto lhs_expr_lhs = std::get<Node_term*>(lhs_add_expr->lhs->expr_var);
    auto lhs_expr_rhs = std::get<Node_term*>(lhs_add_expr->rhs->expr_var);
    auto lhs_expr_lhs_expr = std::get<Node_term_intlit*>(lhs_expr_lhs->term_var);
    auto lhs_expr_rhs_expr = std::get<Node_term_intlit*>(lhs_expr_rhs->term_var);
    auto rhs_expr = std::get<Node_term*>(add_expr->rhs->expr_var);
    auto rhs_expr_term = std::get<Node_term_intlit*>(rhs_expr->term_var);
    EXPECT_TRUE(lhs_expr_lhs_expr->int_lit.value == "5");
    EXPECT_TRUE(lhs_expr_rhs_expr->int_lit.value == "3");
    EXPECT_TRUE(rhs_expr_term->int_lit.value == "5");
}

TEST_F(TestParser, ParseExprAddOperation_n_Term_bigger) {
    std::vector<Token> tokens = {
            {TokenType::int_lit, "52"},
            {TokenType::plus, "+"},
            {TokenType::int_lit, "33"},
            {TokenType::plus, "+"},
            {TokenType::int_lit, "54"},
    };
    Parser parser(tokens);
    auto expr = parser.Parse_expr();
    EXPECT_TRUE(expr.has_value());
    auto expr_op = std::get<Node_OperationExpr*>(expr.value()->expr_var);
    auto add_expr = std::get<Node_OperationExprAdd*>(expr_op->operation_var);
    auto lhs_op_expr = std::get<Node_OperationExpr*>(add_expr->lhs->expr_var);
    auto lhs_add_expr = std::get<Node_OperationExprAdd*>(lhs_op_expr->operation_var);
    auto lhs_expr_lhs = std::get<Node_term*>(lhs_add_expr->lhs->expr_var);
    auto lhs_expr_rhs = std::get<Node_term*>(lhs_add_expr->rhs->expr_var);
    auto lhs_expr_lhs_expr = std::get<Node_term_intlit*>(lhs_expr_lhs->term_var);
    auto lhs_expr_rhs_expr = std::get<Node_term_intlit*>(lhs_expr_rhs->term_var);
    auto rhs_expr = std::get<Node_term*>(add_expr->rhs->expr_var);
    auto rhs_expr_term = std::get<Node_term_intlit*>(rhs_expr->term_var);
    EXPECT_TRUE(lhs_expr_lhs_expr->int_lit.value == "52");
    EXPECT_TRUE(lhs_expr_rhs_expr->int_lit.value == "33");
    EXPECT_TRUE(rhs_expr_term->int_lit.value == "54");
}

TEST_F(TestParser,ParseStmtFor){
    std::vector<Token> tokens = {
            {TokenType::_for, "for"},
            {TokenType::open_paren, "("},
            {TokenType::ident, "x"},
            {TokenType::equals, "="},
            {TokenType::int_lit, "0"},
            {TokenType::semi, ";"},
            {TokenType::ident, "x"},
            {TokenType::smaller, "<"},
            {TokenType::int_lit, "2"},
            {TokenType::semi, ";"},
            {TokenType::ident, "x"},
            {TokenType::plus, "+"},
            {TokenType::plus, "+"},
            {TokenType::close_paren, ")"},
            {TokenType::open_curly, "{"},
            {TokenType::_let, "let"},
            {TokenType::ident, "x"},
            {TokenType::equals, "="},
            {TokenType::int_lit, "3"},
            {TokenType::semi, ";"},
            {TokenType::close_curly, "}"}
    };
    Parser parser(tokens);
    auto stmt = parser.Parse_stmt();
    EXPECT_TRUE(stmt.has_value());
    auto stmt_for = std::get<Node_stmt_for*>(stmt.value()->stmt_var);
    auto stmt_for_first = std::get<Node_stmt_ident*>(stmt_for->first_expr_var);
    EXPECT_EQ(stmt_for_first->ident.value, "x");
    EXPECT_TRUE(stmt_for_first->expr);
    EXPECT_EQ(stmt_for->op.type,TokenType::smaller);
    auto stmt_for_scope_stmt = std::get<Node_stmt_let*>(stmt_for->scope->stmts.front()->stmt_var);
    EXPECT_TRUE(stmt_for_scope_stmt);
    auto stmt_for_incr = std::get<Node_stmt_incr*>(stmt_for->third_expr_var);
    EXPECT_EQ(stmt_for_incr->ident.value, "x");
}
