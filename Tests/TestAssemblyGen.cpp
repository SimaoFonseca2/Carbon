#include <gtest/gtest.h>
#include "../src/Node.hpp"
#include "../src/AssemblyGen.hpp"

class AssemblyGenTest : public ::testing::Test {
protected:
    static std::string generateAssembly(const Node_prog& program) {
        AssemblyGen assemblyGen(program);
        return assemblyGen.gen_prog();
    }
};

TEST_F(AssemblyGenTest, GenerateReturnStatement) {
    Node_expr expr;
    Node_term_intlit term_intlit{.int_lit = Token{TokenType::int_lit, "42"}};
    Node_term term;
    term.term_var = &term_intlit;
    expr.expr_var = &term;
    Node_stmt_return stmt_return{.expr = &expr};
    Node_stmt stmt;
    stmt.stmt_var = &stmt_return;
    Node_prog program;
    program.stmts.push_back(stmt);
    std::string expected_asm = "global _start\nsection .text\n_start:\n     mov rax, 42\n       push rax\n     mov rax, 60\n       pop rdi\n     syscall\n    mov rax, 60\n    mov rdi, 0\n    syscall\nsection .data\n   \nsection .bss\n   num_str resb 20\n";

    EXPECT_EQ(generateAssembly(program), expected_asm);
}

TEST_F(AssemblyGenTest, GenerateLetStatement) {
    Node_expr expr;
    Node_term_intlit term_intlit{.int_lit = Token{TokenType::int_lit, "10"}};
    Node_term term;
    term.term_var = &term_intlit;
    expr.expr_var = &term;
    Node_stmt_let stmt_let{.ident = Token{TokenType::ident, "x"}, .expr = &expr};
    Node_stmt stmt;
    stmt.stmt_var = &stmt_let;
    Node_prog program;
    program.stmts.push_back(stmt);

    std::string expected_asm = "global _start\nsection .text\n_start:\n     mov rax, 10\n       push rax\n    mov rax, 60\n    mov rdi, 0\n    syscall\nsection .data\n   \nsection .bss\n   num_str resb 20\n";

    EXPECT_EQ(generateAssembly(program), expected_asm);
}

TEST_F(AssemblyGenTest, GeneratePrintStatement) {
    Node_stmt_print stmt_print{.msg = Token{TokenType::string, "Hello, World!"}};
    Node_stmt stmt;
    stmt.stmt_var = &stmt_print;
    Node_prog program;
    program.stmts.push_back(stmt);

    std::string expected_asm = "global _start\nsection .text\n_start:\n    mov rax, 1\n    mov rdi, 1\n    mov rsi,      msg0\n    mov rdx, 14\n    syscall\n    mov rax, 60\n    mov rdi, 0\n    syscall\nsection .data\n        msg0 db 'Hello, World!'\n\nsection .bss\n   num_str resb 20\n";

    EXPECT_EQ(generateAssembly(program), expected_asm);
}

TEST_F(AssemblyGenTest, GenerateForStatement) {
    //Declaring the index
    Node_term_intlit node_term_intlit;
    node_term_intlit.int_lit = Token{.type = TokenType::int_lit, .value = "2"};
    Node_term node_term;
    node_term.term_var = &node_term_intlit;
    Node_expr expr;
    expr.expr_var = &node_term;
    Node_stmt_let stmt_let;
    stmt_let.ident = Token{.type = TokenType::ident, .value = "i"};
    stmt_let.expr = &expr;
    //Declaring the scope
    Node_stmt_scope scope;
    Node_term_intlit node_term_intlit_s;
    node_term_intlit_s.int_lit = Token{.type = TokenType::int_lit, .value = "3"};
    Node_term node_term_s;
    node_term_s.term_var = &node_term_intlit_s;
    Node_expr expr_s;
    expr_s.expr_var = &node_term_s;
    Node_stmt_let stmt_let_s;
    stmt_let_s.ident = Token{.type = TokenType::ident, .value = "x"};
    stmt_let_s.expr = &expr_s;
    Node_stmt stmt_s;
    stmt_s.stmt_var = &stmt_let_s;
    scope.stmts.push_back(&stmt_s);
    //Declaring increment statement
    Node_stmt_incr stmt_incr;
    stmt_incr.ident = Token{.type = TokenType::ident,.value = "i"};
    //Declaring for statement
    Node_stmt_for stmtFor{.first_expr_var = &stmt_let,.ident = Token{.type = TokenType::ident,.value = "i"},.op = Token{.type = TokenType::smaller, .value = "<"},.third_expr_var = &stmt_incr,.scope = &scope};
    Node_stmt stmt;
    stmt.stmt_var = &stmtFor;
    Node_prog program;
    program.stmts.push_back(stmt);

    std::string expected_asm = "global _start\nsection .text\n_start:\n     mov rax, 2\n       push rax\n    mov rcx, QWORD [rsp + 0]\nloop0:\n       push rcx\n     mov rax, 3\n       push rax\n       add rsp, 8\n       pop rcx\ninc rcx\nmov QWORD [rsp + 0], rcx\ncmp rcx, <\njne loop0\n    mov rax, 60\n    mov rdi, 0\n    syscall\nsection .data\n   \nsection .bss\n   num_str resb 20\n";

    EXPECT_EQ(generateAssembly(program), expected_asm);
}


TEST_F(AssemblyGenTest, GenerateIfStatement) {
    Node_term_intlit node_term_intlit_let;
    node_term_intlit_let.int_lit = Token{.type = TokenType::int_lit, .value = "2"};
    Node_term node_term_let;
    node_term_let.term_var = &node_term_intlit_let;
    Node_expr expr_let;
    expr_let.expr_var = &node_term_let;
    Node_stmt_let stmt_let_ini;
    stmt_let_ini.ident = Token{.type = TokenType::ident, .value = "y"};
    stmt_let_ini.expr = &expr_let;
    Node_stmt stmt_first_let;
    stmt_first_let.stmt_var = &stmt_let_ini;
    //Declare scope
    Node_stmt_scope scope;
    Node_term_intlit node_term_intlit_s;
    node_term_intlit_s.int_lit = Token{.type = TokenType::int_lit, .value = "1"};
    Node_term node_term_s;
    node_term_s.term_var = &node_term_intlit_s;
    Node_expr expr_s;
    expr_s.expr_var = &node_term_s;
    Node_stmt_let stmt_let_s;
    stmt_let_s.ident = Token{.type = TokenType::ident, .value = "x"};
    stmt_let_s.expr = &expr_s;
    Node_stmt stmt_s;
    stmt_s.stmt_var = &stmt_let_s;
    scope.stmts.push_back(&stmt_s);
    //Declare expr
    Node_term_intlit node_term_intlit;
    node_term_intlit.int_lit = Token{.type = TokenType::int_lit, .value = "3"};
    Node_term node_term;
    node_term.term_var = &node_term_intlit;
    Node_expr expr;
    expr.expr_var = &node_term;
    //Declare If statement
    Node_stmt_if stmtIf{.ident = Token{.type = TokenType::ident,.value = "y"},.expr = &expr,.scope = &scope};
    Node_stmt stmt;
    stmt.stmt_var = &stmtIf;
    Node_prog program;
    program.stmts.push_back(stmt_first_let);
    program.stmts.push_back(stmt);

    std::string expected_asm = "global _start\nsection .text\n_start:\n     mov rax, 2\n       push rax\n     mov rax, 3\n       push rax\n       pop rax\n      cmp QWORD [rsp + 0], rax\n      jnz Label0\n     mov rax, 1\n       push rax\n       add rsp, 8\nLabel0:\n    mov rax, 60\n    mov rdi, 0\n    syscall\nsection .data\n   \nsection .bss\n   num_str resb 20\n";

    EXPECT_EQ(generateAssembly(program), expected_asm);
}

