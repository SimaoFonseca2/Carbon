#ifndef CARBON_ASSEMBLYGEN_HPP
#define CARBON_ASSEMBLYGEN_HPP

#include <sstream>
#include "Node.hpp"
#include <map>
#include <stack>

struct Var{
    size_t stack_location;
};

class AssemblyGen{
public:
    inline explicit AssemblyGen(Node_prog program);
    [[nodiscard]] std::string inline gen_prog();
    void inline gen_stmt(const Node_stmt& stmt);
    void inline gen_expr(const Node_expr& expr);
    void inline gen_term(const Node_term* term);
    void inline gen_scope(const Node_stmt_scope* scope);
    void inline gen_oper(const Node_OperationExpr* op_expr);
    std::string inline create_label();
    void inline push(const std::string& reg);
    void inline pop(const std::string& reg);
private:
    Node_prog m_program;
    std::stringstream m_output;
    size_t stack_size=0;
    std::vector<size_t> scope_sizes;
    bool isIdentStmt = false;
    std::string ident_stmt_value;
    std::string let_stmt_value;
    std::stack<std::map<std::string, Var>> scope_stack;
    int n_labels = 0;
    int loop_count = -1;
    std::map<std::string, std::string> global_stack_locations;
    std::map<std::string, int> var_index;
    int index = 0;
    std::stringstream m_data;
    std::map<std::string, int> for_loops;
    std::map<int, std::map<std::string, int>> loop_list;
    int numCounter = 0;
    bool forflag = false;
};

AssemblyGen::AssemblyGen(Node_prog program): m_program(std::move(program)){
    scope_stack.push({});
}

[[nodiscard]] std::string AssemblyGen::gen_prog(){
    m_output << "global _start\n";
    m_output << "section .text\n";
    m_output << "_start:\n";
    for(const Node_stmt& stmt : m_program.stmts){
        gen_stmt(stmt);
    }
    m_output << "    mov rax, 60\n"
                "    mov rdi, 0\n"
                "    syscall\n";
    m_output << "section .data\n";
    m_output << "   " << m_data.str() <<"\n";
    m_output << "section .bss\n";
    m_output << "   num_str resb 20\n";
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
            if(assemblyGen->scope_stack.top().contains(let_stmt->ident.value.value())){
                std::cerr << "Identifier already used: " << let_stmt->ident.value.value() << std::endl;
                exit(EXIT_FAILURE);
            }else{
                assemblyGen->global_stack_locations.insert_or_assign(let_stmt->ident.value.value(),"QWORD [rsp + " + std::to_string(assemblyGen->stack_size)+ "]");
                assemblyGen->var_index.insert({let_stmt->ident.value.value(),assemblyGen->index});
                assemblyGen->index++;
                assemblyGen->scope_stack.top().insert({let_stmt->ident.value.value(), Var{.stack_location=assemblyGen->stack_size}});
                assemblyGen->gen_expr(*let_stmt->expr);
            }
        }
        void operator()(const Node_stmt_ident* ident_stmt) const {
            if(assemblyGen->scope_stack.top().contains(ident_stmt->ident.value.value())){
                assemblyGen->isIdentStmt = true;
                assemblyGen->ident_stmt_value = ident_stmt->ident.value.value();
                assemblyGen->gen_expr(*ident_stmt->expr);
            }else{
                std::cerr << "Identifier doesn't exist: " << ident_stmt->ident.value.value() << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        void operator()(const Node_stmt_scope* scope_stmt) const {
            assemblyGen->gen_scope(scope_stmt);
        }
        void operator()(const Node_stmt_if* stmt_if) const {
            if(!stmt_if->ident.has_value()){
                assemblyGen->gen_expr(*stmt_if->expr);
                assemblyGen->pop("rax");
                std::string label = assemblyGen->create_label();
                assemblyGen->m_output << "      test rax, rax" << "\n";
                assemblyGen->m_output << "      jz " << label << "\n";
                assemblyGen->gen_scope(stmt_if->scope);
                assemblyGen->m_output<< label << ":\n";
            }else{
                assemblyGen->gen_expr(*stmt_if->expr);
                assemblyGen->pop("rax");
                std::string label = assemblyGen->create_label();
                auto cmpvalue = assemblyGen->global_stack_locations.find(*stmt_if->ident->value);
                if(cmpvalue == assemblyGen->global_stack_locations.end()){
                    std::cerr << "cmpvalue" << std::endl;
                    exit(EXIT_FAILURE);
                }
                auto cmpindex = assemblyGen->var_index.find(*stmt_if->ident->value);
                if(cmpvalue == assemblyGen->global_stack_locations.end()){
                    std::cerr << "cmpindex" << std::endl;
                    exit(EXIT_FAILURE);
                }
                auto test = *cmpvalue;
                std::string str = std::string("QWORD [rsp + ") + std::to_string((assemblyGen->stack_size-cmpindex->second-1)*8) + "]";
                cmpvalue->second = str;
                assemblyGen->m_output << "      cmp " << cmpvalue->second <<", "<< "rax"<< "\n";
                assemblyGen->m_output << "      jnz " << label << "\n";
                assemblyGen->gen_scope(stmt_if->scope);
                assemblyGen->m_output<< label << ":\n";
            }
        }
        void operator()(const Node_stmt_print* stmt_print) const {

            if(stmt_print->msg.type==TokenType::string){
                std::string msgLabel = "     msg" + std::to_string(assemblyGen->numCounter);
                assemblyGen->m_output << "    mov rax, 1\n"
                                         "    mov rdi, 1\n"
                                         "    mov rsi, "<<msgLabel<<"\n"
                                                                    "    mov rdx, "<<stmt_print->msg.value.value().size()+1<<"\n"
                                                                                                                           "    syscall\n";

                assemblyGen->m_data<<msgLabel<<" db '"<< stmt_print->msg.value.value() <<"'\n";
                assemblyGen->numCounter++;
            }
            if(stmt_print->msg.type==TokenType::ident){
                auto index = assemblyGen->var_index.find(*stmt_print->msg.value);
                auto pos = assemblyGen->global_stack_locations.find(*stmt_print->msg.value);
                auto loop_pos = assemblyGen->for_loops.find(*stmt_print->msg.value);
                std::string str = std::string("[rsp + ") + std::to_string((assemblyGen->stack_size-index->second-1)*8) + "]";
                pos->second = str;
                if(index == assemblyGen->var_index.end()){
                    std::cerr<<"Identifier: "<<*stmt_print->msg.value<<" does not exits"<<std::endl;
                    exit(EXIT_FAILURE);
                }
                 assemblyGen->m_output << " mov rax, " << pos->second << "\n"
                                        "   mov rdi, num_str + 20\n"
                                        "   mov rcx, 10\n"
                                        "convert_loop"<<assemblyGen->numCounter<<":\n"
                                        "   dec rdi\n"
                                        "   xor rdx, rdx\n"
                                        "   div rcx\n"
                                        "   add dl, '0'\n"
                                        "   mov [rdi], dl\n"
                                        "   test rax, rax\n"
                                        "   jnz convert_loop"<<assemblyGen->numCounter<<"\n"
                                        "   mov rsi, rdi\n"
                                        "   mov rdx, num_str + 20\n"
                                        "   sub rdx, rsi\n"
                                        "   mov rax, 1\n"
                                        "   mov rdi, 1\n"
                                        "   syscall\n";

                assemblyGen->numCounter++;
            }

        }
        void operator()(const Node_stmt_incr* stmt_incr) const {
            if(assemblyGen->scope_stack.top().contains(stmt_incr->ident.value.value())){
                auto index = assemblyGen->var_index.find(*stmt_incr->ident.value);
                auto pos = assemblyGen->global_stack_locations.find(*stmt_incr->ident.value);
                std::string str = std::string("QWORD [rsp + ") + std::to_string((assemblyGen->stack_size-index->second-1)*8) + "]";
                pos->second = str;
                assemblyGen->m_output << "    inc " << pos->second << "\n";
            }else{
                std::cerr << "Identifier doesn't exist: " << stmt_incr->ident.value.value() << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        void operator()(const Node_stmt_decr* stmt_decr) const {
            if(assemblyGen->scope_stack.top().contains(stmt_decr->ident.value.value())){
                auto index = assemblyGen->var_index.find(*stmt_decr->ident.value);
                auto pos = assemblyGen->global_stack_locations.find(*stmt_decr->ident.value);
                std::string str = std::string("QWORD [rsp + ") + std::to_string((assemblyGen->stack_size-index->second-1)*8) + "]";
                pos->second = str;
                assemblyGen->m_output << "    dec " << pos->second << "\n";
            }else{
                std::cerr << "Identifier doesn't exist: " << stmt_decr->ident.value.value() << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        void operator()(const Node_stmt_for* stmt_for) const {
                struct Visitor_stmt_for_first {
                    AssemblyGen* assemblyGen;
                    const Node_stmt_for* stmt_for;
                    void operator()(const Node_stmt_let* let_stmt) const {
                        if(assemblyGen->scope_stack.top().contains(let_stmt->ident.value.value())){
                            std::cerr << "Identifier already used: " << let_stmt->ident.value.value() << std::endl;
                            exit(EXIT_FAILURE);
                        }else{
                            if(let_stmt->ident.value != stmt_for->ident.value){
                                std::cerr<<"Expected '"<<let_stmt->ident.value.value()<<"' but got '"<<stmt_for->ident.value.value()<<"'"<<std::endl;
                                exit(EXIT_FAILURE);
                            }
                            assemblyGen->global_stack_locations.insert_or_assign(let_stmt->ident.value.value(),"QWORD [rsp + " + std::to_string(assemblyGen->stack_size)+ "]");
                            assemblyGen->var_index.insert({let_stmt->ident.value.value(),assemblyGen->index});
                            assemblyGen->index++;
                            assemblyGen->scope_stack.top().insert({let_stmt->ident.value.value(), Var{.stack_location=assemblyGen->stack_size}});
                            assemblyGen->gen_expr(*let_stmt->expr);
                        }
                    }
                    void operator()(const Node_stmt_ident* stmt_ident) const {
                        if(stmt_ident->ident.value != stmt_for->ident.value){
                            std::cerr<<"Expected '"<<stmt_ident->ident.value.value()<<"' but got '"<<stmt_for->ident.value.value()<<"'"<<std::endl;
                            exit(EXIT_FAILURE);
                        }
                        if(assemblyGen->scope_stack.top().contains(stmt_ident->ident.value.value())){
                            assemblyGen->isIdentStmt = true;
                            assemblyGen->ident_stmt_value = stmt_ident->ident.value.value();
                            assemblyGen->gen_expr(*stmt_ident->expr);
                        }else{
                            std::cerr << "Identifier doesn't exist: " << stmt_ident->ident.value.value() << std::endl;
                            exit(EXIT_FAILURE);
                        }
                    }
                };
                Visitor_stmt_for_first visitorStmtFor_first{.assemblyGen=assemblyGen,.stmt_for = stmt_for};
                std::visit(visitorStmtFor_first, stmt_for->first_expr_var);
                auto index = assemblyGen->var_index.find(*stmt_for->ident.value);
                if(index == assemblyGen->var_index.end()){
                    std::cerr<< "Identifier: "<<*stmt_for->ident.value<<" does not exist"<<std::endl;
                    exit(EXIT_FAILURE);
                }
                auto pos = assemblyGen->global_stack_locations.find(*stmt_for->ident.value);

                std::string str = std::string("QWORD [rsp + ") + std::to_string((assemblyGen->stack_size-index->second-1)*8) + "]";
                pos->second = str;
                assemblyGen->m_output << "    mov rcx, " << pos->second << "\n";
                assemblyGen->loop_count++;
                assemblyGen->for_loops.insert_or_assign(*stmt_for->ident.value, assemblyGen->loop_count);

                assemblyGen->m_output << "loop"<<assemblyGen->loop_count<<":"<<"\n";
                assemblyGen->push("rcx");
                int previous_loop_count = assemblyGen->loop_count;
                assemblyGen->gen_scope(stmt_for->scope);
                if(assemblyGen->loop_count!=previous_loop_count){
                    assemblyGen->forflag = true;
                }
                assemblyGen->pop("rcx");

                struct Visitor_stmt_for_third {
                    AssemblyGen* assemblyGen;
                    void operator()(const Node_stmt_incr* stmt_incr) const {
                        assemblyGen->m_output<<"inc rcx\n";
                    }
                    void operator()(const Node_stmt_decr* stmt_decr) const {
                        assemblyGen->m_output<<"dec rcx\n";
                    }
                };
                Visitor_stmt_for_third visitorStmtFor_third{.assemblyGen=assemblyGen};
                std::visit(visitorStmtFor_third, stmt_for->third_expr_var);
                int value = 0;
                if(assemblyGen->forflag){
                    value = 1;
                }
                assemblyGen->forflag =false;
                std::string str_loop = std::string("QWORD [rsp + ") + std::to_string((assemblyGen->stack_size-index->second+(assemblyGen->loop_count*value)-1)*8) + "]";
                assemblyGen->m_output<<"mov " << str_loop <<", "<< "rcx\n";
                assemblyGen->m_output<<"cmp rcx, " << stmt_for->op.value.value() << "\n";
                if(stmt_for->op.type == TokenType::smaller){
                    assemblyGen->m_output<<"jne "<<"loop"<<assemblyGen->loop_count<<"\n";
                }
        }
    };
    Visitor_stmt visitorStmt{.assemblyGen=this};
    std::visit(visitorStmt, stmt.stmt_var);
}



void inline AssemblyGen::gen_expr(const Node_expr& expr){
    struct Visitor_expr{
        AssemblyGen* assemblyGen;
        void operator()(const Node_term* node_term) const {
            assemblyGen->gen_term(node_term);
        }
        void operator()(const Node_OperationExpr* operation_expr) const {
            assemblyGen->gen_oper(operation_expr);
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

void inline AssemblyGen::gen_term(const Node_term *term) {
    struct Visitor_term{
        AssemblyGen* assemblyGen;
        void operator()(const Node_term_intlit* term_intlit) const{
            if(!assemblyGen->isIdentStmt){
                assemblyGen->m_output << "     mov rax, " << term_intlit->int_lit.value.value() << "\n";
                assemblyGen->push("rax");
            }else{
                assemblyGen->m_output << "      mov QWORD [rsp + " << (assemblyGen->stack_size - assemblyGen->scope_stack.top().at(assemblyGen->ident_stmt_value).stack_location-1) * 8 << "], "<< term_intlit->int_lit.value.value() << "\n";
                assemblyGen->global_stack_locations.insert_or_assign(term_intlit->int_lit.value.value(),"QWORD [rsp + " + std::to_string((assemblyGen->stack_size - assemblyGen->scope_stack.top().at(assemblyGen->ident_stmt_value).stack_location-1) * 8)+ "]");
                assemblyGen->isIdentStmt=false;
            }
        }
        void operator()(const Node_term_ident* term_ident) const{
            if(!assemblyGen->scope_stack.top().contains(term_ident->ident.value.value())){
                std::cerr << "Undeclared identifier: " << term_ident->ident.value.value();
                exit(EXIT_FAILURE);
            }
            if(assemblyGen->isIdentStmt && assemblyGen->scope_stack.top().contains(term_ident->ident.value.value())){
                assemblyGen->m_output << "      mov rax, QWORD [rsp + " << (assemblyGen->stack_size - assemblyGen->scope_stack.top().at(assemblyGen->ident_stmt_value).stack_location-1) * 8 << "], "<< "\n";
                assemblyGen->m_output << "      mov QWORD [rsp + " << (assemblyGen->stack_size - assemblyGen->scope_stack.top().at(assemblyGen->ident_stmt_value).stack_location-1) * 8 << "], rax"<< "\n";
                assemblyGen->isIdentStmt=false;
            }else if(assemblyGen->scope_stack.top().contains(term_ident->ident.value.value())){
                std::stringstream stack_off;
                if(!assemblyGen->ident_stmt_value.empty()){
                    assemblyGen->global_stack_locations.insert_or_assign(assemblyGen->ident_stmt_value,"QWORD [rsp + " + std::to_string((assemblyGen->stack_size - assemblyGen->scope_stack.top().at(assemblyGen->ident_stmt_value).stack_location-1) * 8)+ "]");
                }
                stack_off << "  QWORD [rsp + " << (assemblyGen->stack_size - assemblyGen->scope_stack.top().at(term_ident->ident.value.value()).stack_location-1) * 8 << "]\n";
                assemblyGen->push(stack_off.str());
            }
        }
        void operator()(const Node_term_paren* term_paren) const{
            assemblyGen->gen_expr(*term_paren->expr);
        }
    };
    Visitor_term visitorterm{.assemblyGen = this};
    std::visit(visitorterm, term->term_var);
}

void inline AssemblyGen::gen_oper(const Node_OperationExpr *op_expr) {
    struct Visitor_oper{
        AssemblyGen* assemblyGen;
        void operator()(const Node_OperationExprAdd* add) const {
            assemblyGen->gen_expr(*add->lhs);
            assemblyGen->gen_expr(*add->rhs);
            assemblyGen->pop("rax");
            assemblyGen->pop("rbx");
            assemblyGen->m_output << "      add rax, rbx\n";
            assemblyGen->push("rax");
        }
        void operator()(const Node_OperationExprSub* sub) const {
            assemblyGen->gen_expr(*sub->lhs);
            assemblyGen->gen_expr(*sub->rhs);
            assemblyGen->pop("rax");
            assemblyGen->pop("rbx");
            assemblyGen->m_output << "      sub rbx, rax\n";
            assemblyGen->push("rbx");
        }
        void operator()(const Node_OperationExprMulti* multi) const {
            assemblyGen->gen_expr(*multi->lhs);
            assemblyGen->gen_expr(*multi->rhs);
            assemblyGen->pop("rax");
            assemblyGen->pop("rbx");
            assemblyGen->m_output << "      mul rbx\n";
            assemblyGen->push("rax");
        }
        void operator()(const Node_OperationExprDiv* div) const {
            assemblyGen->gen_expr(*div->rhs);
            assemblyGen->gen_expr(*div->lhs);
            assemblyGen->pop("rax");
            assemblyGen->pop("rbx");
            assemblyGen->m_output << "      div rbx\n";
            assemblyGen->push("rax");
        }
    };
    Visitor_oper visitorOper{.assemblyGen = this};
    std::visit(visitorOper, op_expr->operation_var);
}

std::string inline AssemblyGen::create_label() {
    return "Label" + std::to_string(n_labels++);
}

void inline AssemblyGen::gen_scope(const Node_stmt_scope *scope) {
    auto previous_stack = scope_stack.top();
    scope_stack.push(previous_stack);
    for(const Node_stmt* stmt : scope->stmts){
        gen_stmt(*stmt);
    }
    size_t count = scope_stack.top().size() - previous_stack.size();
    m_output <<"       add rsp, " << count * 8 << "\n";
    stack_size -= count;
    scope_stack.pop();
}


#endif //CARBON_ASSEMBLYGEN_HPP
