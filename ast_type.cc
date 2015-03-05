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
        if(strcmp(typeName, "int") && strcmp(typeName, "double") && strcmp(typeName, "void") && strcmp(typeName, "bool") && strcmp(typeName, "null") && strcmp(typeName, "string") && strcmp(typeName, "error"))
        if (declared_types.Lookup(typeName) == nullptr)
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
        Type::Check();

        if (!IsDeclared())
        {
                ReportError::Formatted(location,
                                "No declaration found for type '%s'",
                                id->GetName());
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
}

bool NamedType::IsDeclared() {
        return (declared_types.Lookup(id->GetName()) != nullptr);
}
