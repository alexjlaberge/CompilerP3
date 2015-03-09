/* File: ast_type.cc
 * -----------------
 * Implementation of type node classes.
 */
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include "hashtable.h"
#include "errors.h"
#include "symbols.h"
#include <cassert>
#include <iostream>
#include "ast_expr.h"

using namespace std;
 
/* Class constants
 * ---------------
 * These are public constants for the built-in base types (int, double, etc.)
 * They can be accessed with the syntax Type::intType. This allows you to
 * directly access them and share the built-in types where needed rather that
 * creates lots of copies.
 */

Type *Type::intType    = new Type("int");
Type *Type::doubleType = new Type("double");
Type *Type::voidType   = new Type("void");
Type *Type::boolType   = new Type("bool");
Type *Type::nullType   = new Type("null");
Type *Type::stringType = new Type("string");
Type *Type::errorType  = new Type("error"); 

Type::Type(const char *n) {
    Assert(n);
    typeName = strdup(n);
}

void Type::Check()
{
        if (typeName == nullptr)
        {
                return;
        }

        if (strcmp(typeName, "int") != 0 &&
                        strcmp(typeName, "double") != 0 &&
                        strcmp(typeName, "string") != 0 &&
                        strcmp(typeName, "bool") != 0 &&
                        strcmp(typeName, "error") != 0 &&
                        strcmp(typeName, "null") != 0 &&
                        strcmp(typeName, "void") != 0)
        {
                ReportError::Formatted(location,
                                "No declaration found for type '%s'",
                                typeName);
        }
}

void Type::PrintChildren(int indentLevel) {
    printf("%s", typeName);
}

NamedType::NamedType(Identifier *i) : Type(*i->GetLocation()) {
    Assert(i != NULL);
    (id=i)->SetParent(this);
} 

void NamedType::Check()
{
        const Decl *par = getVariable(id->GetName());
        if (par == nullptr)
        {
                if (dynamic_cast<VarDecl*>(parent) != nullptr ||
                                dynamic_cast<NewArrayExpr*>(parent) != nullptr)
                {
                        ReportError::Formatted(location,
                                        "No declaration found for type '%s'",
                                        id->GetName());
                        return;
                }
                ReportError::Formatted(location,
                                "No declaration found for class '%s'",
                                id->GetName());
                return;
        }

        if (dynamic_cast<const ClassDecl*>(par) == nullptr &&
                        dynamic_cast<const InterfaceDecl*>(par) == nullptr)
        {
                if (dynamic_cast<VarDecl*>(parent) != nullptr ||
                                dynamic_cast<ArrayType*>(parent) != nullptr)
                {
                        ReportError::Formatted(location,
                                        "No declaration found for type '%s'",
                                        id->GetName());
                        return;
                }
                ReportError::Formatted(location,
                                "No declaration found for class '%s'",
                                id->GetName());
                return;
        }
}

void NamedType::PrintChildren(int indentLevel) {
    id->Print(indentLevel+1);
}

ArrayType::ArrayType(yyltype loc, Type *et) : Type(loc) {
    Assert(et != NULL);
    (elemType=et)->SetParent(this);
}

void ArrayType::PrintChildren(int indentLevel) {
    elemType->Print(indentLevel+1);
}

void ArrayType::Check() {
        elemType->Check();
}

bool NamedType::IsDeclared() {
        return type_exists(id->GetName());
}

bool Type::operator!=(const Type *rhs) const
{
        return (strcmp(getTypeName(), rhs->getTypeName()) != 0);
}

const char *ArrayType::getTypeName() const
{
      string name = string(elemType->getTypeName());
      name += "[]";
      return strdup(name.c_str());
}

Type *ArrayType::getBaseType() const
{
        return elemType;
}

bool NamedType::isDescendedFrom(const Type *other) const
{
        const Decl *me = parent->getVariable(getTypeName());
        const Decl *par = parent->getVariable(other->getTypeName());

        if (me == nullptr || par == nullptr)
        {
                return false;
        }

        return me->descendedFrom(par->getName());
}

const Decl *Type::getVariable(const char *name) const
{
        return parent->getVariable(name);
}
