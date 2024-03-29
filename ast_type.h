/* File: ast_type.h
 * ----------------
 * In our parse tree, Type nodes are used to represent and
 * store type information. The base Type class is used
 * for built-in types, the NamedType for classes and interfaces,
 * and the ArrayType for arrays of other types.  
 */
 
#ifndef _H_ast_type
#define _H_ast_type

#include "ast.h"
#include "list.h"
 #include <string.h>


class Type : public Node 
{
  protected:
    char *typeName;

  public :
    static Type *intType, *doubleType, *boolType, *voidType,
                *nullType, *stringType, *errorType;

    Type(yyltype loc) : Node(loc) {}
    Type(const char *str);
    
    const char *GetPrintNameForNode() { return "Type"; }
    void PrintChildren(int indentLevel);
    virtual void Check();
    virtual const char *getTypeName() const {return typeName;}
    virtual bool operator!=(const Type *rhs) const;
    virtual bool isDescendedFrom(const Type *other) const { return false; }
    virtual bool isBasicType() const {return true;}
    virtual const Decl *getVariable(const char *name) const;
};

class NamedType : public Type 
{
  protected:
    Identifier *id;
    
  public:
    NamedType(Identifier *i);
    
    const char *GetPrintNameForNode() { return "NamedType"; }
    void PrintChildren(int indentLevel);
    bool IsDeclared();
    const Identifier * GetId() { return id; }
    virtual void Check();
    const char *getTypeName() const {return id->GetName();}
    virtual bool isDescendedFrom(const Type *other) const;
    virtual bool isBasicType() const {return false;}
};

class ArrayType : public Type 
{
  protected:
    Type *elemType;

  public:
    ArrayType(yyltype loc, Type *elemType);
    
    const char *GetPrintNameForNode() { return "ArrayType"; }
    void PrintChildren(int indentLevel);
    virtual void Check();
    const char *getTypeName() const;

    Type *getBaseType() const;
};

 
#endif
