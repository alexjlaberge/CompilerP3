/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "symbols.h"

Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this);
    if(parent != nullptr)
    {
        level = parent->getLevel();
    }
    else
    {
        level = 0; 
    }
}


VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
    //std::cout << n->GetName() << " LEVEL " << level << std::endl;
}
  
void VarDecl::PrintChildren(int indentLevel) { 
   type->Print(indentLevel+1);
   id->Print(indentLevel+1);
}

ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType*> *imp, List<Decl*> *m) : Decl(n) {
    // extends can be NULL, impl & mem may be empty lists but cannot be NULL
    Assert(n != NULL && imp != NULL && m != NULL);     
    extends = ex;
    if (extends) extends->SetParent(this);
    (implements=imp)->SetParentAll(this);
    (members=m)->SetParentAll(this);
}

void ClassDecl::PrintChildren(int indentLevel) {
    id->Print(indentLevel+1);
    if (extends) extends->Print(indentLevel+1, "(extends) ");
    implements->PrintAll(indentLevel+1, "(implements) ");
    members->PrintAll(indentLevel+1);
}


InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n) {
    Assert(n != NULL && m != NULL);
    (members=m)->SetParentAll(this);
}

void InterfaceDecl::PrintChildren(int indentLevel) {
    id->Print(indentLevel+1);
    members->PrintAll(indentLevel+1);
}
	
FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
    body->setLevel(level);
}

void FnDecl::PrintChildren(int indentLevel) {
    returnType->Print(indentLevel+1, "(return type) ");
    id->Print(indentLevel+1);
    formals->PrintAll(indentLevel+1, "(formals) ");
    if (body) body->Print(indentLevel+1, "(body) ");
}

void FnDecl::Check() {
        

        int i = 0;

        while (i < formals->NumElements())
        {
                formals->Nth(i)->setLevel(level);
                formals->Nth(i)->Check();
                i++;
        }
        body->setLevel(level);
        body->Check();
        /* TODO returnType->Check() */
}

void InterfaceDecl::Check() {
        Decl::Check();

        int i = 0;

        while (i < members->NumElements())
        {
                members->Nth(i)->Check();
                i++;
        }
}

void Decl::Check() {
        /* TODO Put decl id in symbol table */
        id->Check();
}

void VarDecl::Check() {
        Decl::Check();
        type->Check();
        //scoped_variables.emplace((char*)id->GetName(), getLevel());
        //std::cout << "Entering " << id->GetName() << " at " << getLevel() << std::endl; 
        //std::cout << id->GetName() << " " << getLevel() << std::endl;
}

void ClassDecl::Check() {
        Decl::Check();

        for(int i = 0; i < members->NumElements(); i++)
        {
            members->Nth(i)->setLevel(level+1);
        }

        int i = 0;

        if (extends != nullptr)
        {
                extends->Check();
        }

        while (i < members->NumElements())
        {
                members->Nth(i)->Check();
                i++;
        }

        i = 0;
        while (i < implements->NumElements())
        {
                implements->Nth(i)->Check();
                i++;
        }
}

const Decl * ClassDecl::getVariable(const char *name) const
{
        for (int i = 0; i < members->NumElements(); i++)
        {
                if (strcmp(members->Nth(i)->getName(), name) == 0)
                {
                        return members->Nth(i);
                }
        }

        return parent->getVariable(name);
}

const Decl * FnDecl::getVariable(const char *name) const
{
        for (int i = 0; i < formals->NumElements(); i++)
        {
                if (strcmp(formals->Nth(i)->getName(), name) == 0)
                {
                        return formals->Nth(i);
                }
        }

        return parent->getVariable(name);
}

const Decl * VarDecl::getVariable(const char *name) const
{
        return parent->getVariable(name);
}
