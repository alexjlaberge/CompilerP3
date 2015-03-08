/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_decl.h"
#include "ast_expr.h"
#include "ast_stmt.h"
#include "ast_type.h"
#include "errors.h"
#include <iostream>
#include <string.h>

using std::cout;
using std::endl;

Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    printf("\n");
}

void Program::Check() {
        for(int i = 0; i < decls->NumElements(); i++)
        {
            decls->Nth(i)->setLevel(1);
        }

        int i = 0;
        while (i < decls->NumElements())
        {
                decls->Nth(i)->Check();
                i++;
        }
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s):Stmt() {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
    
}

void StmtBlock::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    stmts->PrintAll(indentLevel+1);
}

void StmtBlock::Check()
{
        for(int i = 0; i < decls->NumElements(); i++)
        {
            decls->Nth(i)->setLevel(level+1);
        }
        for(int i = 0; i < stmts->NumElements(); i++)
        {
            stmts->Nth(i)->setLevel(level+1);
        }

        int i = 0;

        while (i < decls->NumElements())
        {
                decls->Nth(i)->Check();
                i++;
        }
        i = 0;
        while (i < stmts->NumElements())
        {
                stmts->Nth(i)->Check();
                i++;
        }
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init=i)->SetParent(this);
    (step=s)->SetParent(this);
}

void ForStmt::PrintChildren(int indentLevel) {
    std::cout << isBreakable() << std::endl;
    init->Print(indentLevel+1, "(init) ");
    test->Print(indentLevel+1, "(test) ");
    step->Print(indentLevel+1, "(step) ");
    body->Print(indentLevel+1, "(body) ");
}

void WhileStmt::PrintChildren(int indentLevel) {
    std::cout << isBreakable() << std::endl;
    test->Print(indentLevel+1, "(test) ");
    body->Print(indentLevel+1, "(body) ");
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

void IfStmt::PrintChildren(int indentLevel) {
    test->Print(indentLevel+1, "(test) ");
    body->Print(indentLevel+1, "(then) ");
    if (elseBody) elseBody->Print(indentLevel+1, "(else) ");
}


ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    Assert(e != NULL);
    (expr=e)->SetParent(this);
}

void ReturnStmt::PrintChildren(int indentLevel) {
    expr->Print(indentLevel+1);
}
  
PrintStmt::PrintStmt(List<Expr*> *a) {    
    Assert(a != NULL);
    (args=a)->SetParentAll(this);
}

void PrintStmt::PrintChildren(int indentLevel) {
    args->PrintAll(indentLevel+1, "(args) ");
}

void PrintStmt::Check() {
        int i = 0;

        while (i < args->NumElements())
        {
                args->Nth(i)->Check();

                const Type *t = args->Nth(i)->getType();
                assert(t);
                if (t != Type::intType && t != Type::boolType &&
                                t != Type::stringType && t != Type::errorType)
                {
                        ReportError::Formatted(args->Nth(i)->GetLocation(),
                                        "Incompatible argument %d: %s given, int/bool/string expected",
                                        i + 1,
                                        t->getTypeName());
                }
                i++;
        }
}

void BreakStmt::Check() { //DONE
    Node* p = parent;
    while(p != nullptr)
    {    if(p->isBreakable())
            return;
         else
            p = p->GetParent();
    }
    ReportError::Formatted(location, "break is only allowed inside a loop");
}

Case::Case(IntConstant *v, List<Stmt*> *s) {
    Assert(s != NULL);
    value = v;
    if (value) value->SetParent(this);
    (stmts=s)->SetParentAll(this);
}

void Case::PrintChildren(int indentLevel) {
    if (value) value->Print(indentLevel+1);
    stmts->PrintAll(indentLevel+1);
}

SwitchStmt::SwitchStmt(Expr *e, List<Case*> *c) {
    Assert(e != NULL && c != NULL);
    (expr=e)->SetParent(this);
    (cases=c)->SetParentAll(this);
}
      
void SwitchStmt::PrintChildren(int indentLevel) {
    expr->Print(indentLevel+1);
    cases->PrintAll(indentLevel+1);
}

void SwitchStmt::Check() {
    int i = 0;

    expr->Check(); //Evaluate to int

    while (i < cases->NumElements())
    {
            cases->Nth(i)->Check();
            i++;
    }
}

void ConditionalStmt::Check() {
        test->Check();
        if(test->getType() != Type::boolType)
        {
                ReportError::Formatted(location,
                                "Test expression must have boolean type");
        }
        body->Check();
}

void ForStmt::Check() {
        init->Check(); 
        test->Check();
        if(test->getType() != Type::boolType)
        {
                ReportError::Formatted(test->GetLocation(),
                                "Test expression must have boolean type");
        }
        step->Check();
        body->Check();
}

void IfStmt::Check() {
        test->Check();
        if(test->getType() != Type::errorType &&
                        test->getType() != Type::boolType)
        {
                ReportError::Formatted(test->GetLocation(),
                                "Test expression must have boolean type");
        }

        body->Check();

        if (elseBody != nullptr)
        {
                elseBody->Check();
        }
}

void ReturnStmt::Check() {
        expr->setLevel(level);
        expr->Check();
        Type* t = expr->getType();
        
        Node *par = this;
        FnDecl *fn = nullptr;
        while (true)
        {
                par = par->GetParent();
                fn = dynamic_cast<FnDecl*>(par);
                if (fn != nullptr)
                {
                        break;
                }
        }

        if (fn->getType()->operator!=(t))
        {
                ReportError::Formatted(expr->GetLocation(),
                                "Incompatible return: %s given, %s expected",
                                t->getTypeName(),
                                fn->getType()->getTypeName());
        }
}

void Case::Check() {
        int i = 0;

        while (i < stmts->NumElements())
        {
                stmts->Nth(i)->Check();
                i++;
        }
}

const Decl *StmtBlock::getVariable(const char *name) const
{
        for (int i = 0; i < decls->NumElements(); i++)
        {
                if (strcmp(decls->Nth(i)->getName(), name) == 0)
                {
                        return decls->Nth(i);
                }
        }

        return parent->getVariable(name);
}

const Decl *Program::getVariable(const char *name) const
{
        for (int i = 0; i < decls->NumElements(); i++)
        {
                if (strcmp(decls->Nth(i)->getName(), name) == 0)
                {
                        return decls->Nth(i);
                }
        }

        return nullptr;
}

const Decl *Stmt::getVariable(const char *name) const
{
        return parent->getVariable(name);
}

const ClassDecl *Stmt::getThis() const
{
        return parent->getThis();
}
