#ifndef CARBON_ASSEMBLYGEN_HPP
#define CARBON_ASSEMBLYGEN_HPP

#include <sstream>
#include "Parser.hpp"
#include <map>
struct Var{
    size_t stack_location;
};

class AssemblyGen{
public:
    inline explicit AssemblyGen(Node_prog program);
    [[nodiscard]] std::string inline gen_prog();
    void inline gen_stmt(const Node_stmt& stmt);
    void inline gen_expr(const Node_expr& expr);
    void inline push(const std::string& reg);
    void inline pop(const std::string& reg);
private:
    Node_prog m_program;
    std::stringstream m_output;
    size_t stack_size=0;
    std::map<std::string, Var> variables{};
    bool isIdentStmt;
    std::string ident_stmt_value;
};

AssemblyGen::AssemblyGen(Node_prog program): m_program(std::move(program)){}

[[nodiscard]] std::string AssemblyGen::gen_prog(){

    m_output << "global _start\n_start:\n";
    for(const Node_stmt& stmt : m_program.stmts){
        gen_stmt(stmt);
    }
    m_output << "     mov rax, 60\n";
    m_output << "     mov rdi, 0\n";
    m_output << "     syscall";
    return m_output.str();
}

void inline AssemblyGen::gen_stmt(const Node_stmt& stmt) {
    struct Visitor_stmt{
        AssemblyGen *assemblyGen;
        void operator()(const Node_stmt_return* return_stmt) const {
            assemblyGen->gen_expr(*return_stmt->expr);
            assemblyGen->m_output << "     mov rax, 60\n";
            assemblyGen->pop("rdi");
            assemblyGen->m_output << "     syscall\n";
        }
        void operator()(const Node_stmt_let* let_stmt) const {
            if(assemblyGen->variables.contains(let_stmt->ident.value.value())){
                std::cerr << "Identifier already used: " << let_stmt->ident.value.value() << std::endl;
                exit(EXIT_FAILURE);
            }else{
                assemblyGen->variables.insert({let_stmt->ident.value.value(), Var{.stack_location=assemblyGen->stack_size}});
                assemblyGen->gen_expr(*let_stmt->expr);
            }
        }
        void operator()(const Node_stmt_ident* ident_stmt) const {
            if(assemblyGen->variables.contains(ident_stmt->ident.value.value())){
                assemblyGen->isIdentStmt = true;
                assemblyGen->ident_stmt_value = ident_stmt->ident.value.value();
                assemblyGen->gen_expr(*ident_stmt->expr);
            }else{
                std::cerr << "Identifier doesn't exist: " << ident_stmt->ident.value.value() << std::endl;
            }
        }
        void operator()(const Node_OperationExpr* operation_expr) const {
            //todo
        }
    };
    Visitor_stmt visitorStmt{.assemblyGen=this};
    std::visit(visitorStmt, stmt.stmt_var);
}

void inline AssemblyGen::gen_expr(const Node_expr& expr){
    struct Visitor_expr{
        AssemblyGen* assemblyGen;
        void operator()(const Node_expr_intlit* expr_intlit) const{
            if(!assemblyGen->isIdentStmt){
                assemblyGen->m_output << "     mov rax, " << expr_intlit->int_lit.value.value() << "\n";
                assemblyGen->push("rax");
            }else{
                assemblyGen->m_output << "      mov QWORD [rsp + " << (assemblyGen->stack_size - assemblyGen->variables.at(assemblyGen->ident_stmt_value).stack_location-1) * 8 << "], "<< expr_intlit->int_lit.value.value() << "\n";
                assemblyGen->isIdentStmt=false;
            }
        }
        void operator()(const Node_expr_ident* expr_ident) const{
            if(assemblyGen->variables.find(expr_ident->ident.value.value())==assemblyGen->variables.end()){
                std::cerr << "Undeclared identifier: " << expr_ident->ident.value.value();
                exit(EXIT_FAILURE);
            }
            if(assemblyGen->isIdentStmt && assemblyGen->variables.find(expr_ident->ident.value.value())!=assemblyGen->variables.end()){
                auto y = expr_ident->ident.value.value();
                assemblyGen->m_output << "      mov rax, QWORD [rsp + " << (assemblyGen->stack_size - assemblyGen->variables.at(expr_ident->ident.value.value()).stack_location-1) * 8 << "], "<< "\n";
                assemblyGen->m_output << "      mov QWORD [rsp + " << (assemblyGen->stack_size - assemblyGen->variables.at(assemblyGen->ident_stmt_value).stack_location-1) * 8 << "], rax"<< "\n"; //this is a place holder
                assemblyGen->isIdentStmt=false;
            }else if(assemblyGen->variables.contains(expr_ident->ident.value.value())){
                std::stringstream stack_off;
                stack_off << "      QWORD [rsp + " << (assemblyGen->stack_size - assemblyGen->variables.at(expr_ident->ident.value.value()).stack_location-1) * 8 << "]\n";
                assemblyGen->push(stack_off.str());
            }
        }
        void operator()(const Node_OperationExpr* operation_expr) const {
            //todo
        }
    };
    Visitor_expr visitorExpr{.assemblyGen = this};
    std::visit(visitorExpr, expr.expr_var);
}

void inline AssemblyGen::push(const std::string &reg) {
    m_output << "       push " << reg << "\n";
    stack_size++;
}

void inline AssemblyGen::pop(const std::string &reg) {
    m_output << "       pop " << reg << "\n";
    stack_size--;
}



#endif //CARBON_ASSEMBLYGEN_HPP
