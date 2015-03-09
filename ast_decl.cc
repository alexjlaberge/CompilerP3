/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "symbols.h"
#include "errors.h"
#include <iostream>

using namespace std;

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
    declared_interfaces.Enter(n->GetName(), this);
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

        if (body == nullptr)
        {
                if (dynamic_cast<InterfaceDecl*>(parent) == nullptr)
                {
                        /* there's an error here */
                        assert(0);
                }
        }
        else
        {
                body->Check();
        }

        returnType->Check();
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
                if (parent->getVariable(extends->getTypeName()) == nullptr)
                {
                        ReportError::Formatted(extends->GetLocation(),
                                        "No declaration found for class '%s'",
                                        extends->getTypeName());
                }
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

                const InterfaceDecl *iface =
                        dynamic_cast<const InterfaceDecl*>(
                                        parent->getVariable(implements->Nth(i)->getTypeName())
                                        );

                if (iface == nullptr)
                {
                        ReportError::Formatted(implements->Nth(i)->GetLocation(),
                                        "No declaration found for interface '%s'",
                                        implements->Nth(i)->getTypeName());
                        return;
                }

                for (int j = 0; j < iface->numMembers(); j++)
                {
                        FnDecl *myFn = nullptr;
                        const FnDecl *ifaceFn = dynamic_cast<const FnDecl*>(
                                        iface->getMember(j));

                        for (int k = 0; k < members->NumElements(); k++)
                        {
                                if (strcmp(members->Nth(k)->getName(),
                                                        ifaceFn->getName()) == 0)
                                {
                                        myFn = dynamic_cast<FnDecl*>(
                                                        members->Nth(k));
                                        assert(myFn);
                                        break;
                                }
                        }

                        if (myFn == nullptr)
                        {
                                ReportError::Formatted(implements->Nth(i)->GetLocation(),
                                                "Class '%s' does not implement entire interface '%s'",
                                                id->GetName(),
                                                iface->getName());
                                continue;
                        }
                }

                for (int j = 0; j < iface->numMembers(); j++)
                {
                        FnDecl *myFn = nullptr;
                        const FnDecl *ifaceFn = dynamic_cast<const FnDecl*>(
                                        iface->getMember(j));
                        if (ifaceFn == nullptr)
                        {
                                assert(0);
                        }

                        for (int k = 0; k < members->NumElements(); k++)
                        {
                                if (strcmp(members->Nth(k)->getName(),
                                                        ifaceFn->getName()) == 0)
                                {
                                        myFn = dynamic_cast<FnDecl*>(
                                                        members->Nth(k));
                                        assert(myFn);
                                        break;
                                }
                        }

                        if (myFn != nullptr && !myFn->signatureEqual(ifaceFn))
                        {
                                ReportError::Formatted(myFn->GetLocation(),
                                                "Method '%s' must match inherited type signature",
                                                myFn->getName());
                        }
                }

                i++;
        }
}

bool FnDecl::signatureEqual(const FnDecl *other) const
{
        if (returnType->operator!=(other->returnType))
        {
                return false;
        }

        if (formals->NumElements() != other->formals->NumElements())
        {
                return false;
        }

        for (int i = 0; i < formals->NumElements(); i++)
        {
                if (formals->Nth(i)->getType()->operator!=(
                                        other->formals->Nth(i)->getType()))
                {
                        return false;
                }
        }

        return true;
}

const Decl *InterfaceDecl::getMember(int i) const
{
        return members->Nth(i);
}

int InterfaceDecl::numMembers() const
{
        return members->NumElements();
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

        if (extends != nullptr)
        {
                const ClassDecl *super = dynamic_cast<const ClassDecl*>(
                                parent->getVariable(extends->getTypeName()));
                if (super != nullptr)
                {
                        return super->getVariable(name);
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

const ClassDecl *ClassDecl::getThis() const
{
        return this;
}

const ClassDecl *FnDecl::getThis() const
{
        return parent->getThis();
}

const Type *FnDecl::formalType(int i) const
{
        if (formals != nullptr)
        {
                return formals->Nth(i)->getType();
        }

        return nullptr;
}

int FnDecl::NumFormals() const
{
        if (formals != nullptr)
        {
                return formals->NumElements();
        }

        return 0;
}

const Decl *InterfaceDecl::getVariable(const char *name) const
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

bool Decl::descendedFrom(const char *name) const
{
        return false;
}

bool ClassDecl::descendedFrom(const char *name) const
{
        if (extends != nullptr)
        {
                if (strcmp(extends->getTypeName(), name) == 0)
                {
                        return true;
                }

                if (parent->getVariable(extends->getTypeName())->descendedFrom(name))
                {
                        return true;
                }
        }

        for (int i = 0; i < implements->NumElements(); i++)
        {
                if (strcmp(implements->Nth(i)->getTypeName(), name) == 0)
                {
                        return true;
                }
        }

        return false;
}
