/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include "errors.h"
#include "symbols.h"
#include <cassert>
#include <iostream>

using namespace std;

#define compound_expr_return_if_errors() \
        assert(right->getType()); \
        if (left != nullptr) assert(left->getType()); \
        if ((left != nullptr && left->getType() == Type::errorType) || \
                        right->getType() == Type::errorType) \
        { \
                type = Type::errorType; \
                return; \
        }

#define type_assert(expr) \
        if (!expr) { \
                ReportError::Formatted(location, "Null type"); \
                assert(expr); \
        }

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
    type = Type::intType;
}
void IntConstant::PrintChildren(int indentLevel) { 
    printf("%d", value);
}

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
    type = Type::doubleType;
}
void DoubleConstant::PrintChildren(int indentLevel) { 
    printf("%g", value);
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
    type = Type::boolType;
}
void BoolConstant::PrintChildren(int indentLevel) { 
    printf("%s", value ? "true" : "false");
}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc) {
    Assert(val != NULL);
    value = strdup(val);
    type = Type::stringType;
}
void StringConstant::PrintChildren(int indentLevel) { 
    printf("%s",value);
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}

void Operator::PrintChildren(int indentLevel) {
    printf("%s",tokenString);
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}

void CompoundExpr::PrintChildren(int indentLevel) {
   if (left) left->Print(indentLevel+1);
   op->Print(indentLevel+1);
   right->Print(indentLevel+1);
}
   
  
ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

void ArrayAccess::PrintChildren(int indentLevel) {
    base->Print(indentLevel+1);
    subscript->Print(indentLevel+1, "(subscript) ");
  }
     
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}


  void FieldAccess::PrintChildren(int indentLevel) {
    if (base) base->Print(indentLevel+1);
    field->Print(indentLevel+1);
  }

Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}

 void Call::PrintChildren(int indentLevel) {
    if (base) base->Print(indentLevel+1);
    field->Print(indentLevel+1);
    actuals->PrintAll(indentLevel+1, "(actuals) ");
  }
 

NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc) { 
  Assert(c != NULL);
  (cType=c)->SetParent(this);
}

void NewExpr::PrintChildren(int indentLevel) {	
    cType->Print(indentLevel+1);
}

NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc) {
    Assert(sz != NULL && et != NULL);
    (size=sz)->SetParent(this); 
    elemType = new ArrayType(loc, et);
    elemType->SetParent(this);
    type = elemType;
}

void NewArrayExpr::PrintChildren(int indentLevel) {
    size->Print(indentLevel+1);
    elemType->Print(indentLevel+1);
}

PostfixExpr::PostfixExpr(LValue *lv, Operator *o) : Expr(Join(lv->GetLocation(), o->GetLocation())) {
    Assert(lv != NULL && o != NULL);
    (lvalue=lv)->SetParent(this);
    (op=o)->SetParent(this);
}

void PostfixExpr::PrintChildren(int indentLevel) {
    lvalue->Print(indentLevel+1);
    op->Print(indentLevel+1);
}

void DoubleConstant::Check() {

}

void IntConstant::Check() {

}

void BoolConstant::Check() {

}

void StringConstant::Check() {

}

void CompoundExpr::Check() {
        if (left != nullptr)
        {
                left->Check();

                type_assert(left->getType());
        }

        op->Check();

        right->Check();
}

Type *ArithmeticExpr::getType() {
        if (type != nullptr)
        {
                return type;
        }

        if (left != nullptr && left->getType()->operator!=(right->getType()))
        {
                type = Type::errorType;
        }
        else
        {
                type = right->getType();
        }

        return type;
}

void ArithmeticExpr::Check() {
        if (left != nullptr)
        {
                left->Check();

                if (left->getType()->operator!=(right->getType()) ||
                                (left->getType() != Type::intType &&
                                 left->getType() != Type::doubleType))
                {
                        if (left->getType() != Type::errorType &&
                                        right->getType() != Type::errorType)
                        {
                                ReportError::Formatted(op->GetLocation(),
                                                "Incompatible operands: %s %s %s",
                                                left->getType()->getTypeName(),
                                                op->getOp(),
                                                right->getType()->getTypeName());
                        }
                        if (left->getType()->operator!=(right->getType()) &&
                                                right->getType() == Type::stringType)
                        {
                                ReportError::Formatted(op->GetLocation(),
                                                "Incompatible operands: %s %s %s",
                                                left->getType()->getTypeName(),
                                                op->getOp(),
                                                right->getType()->getTypeName());
                        }
                        type = Type::errorType;
                }
        }

        right->Check();
        if (type != Type::errorType && right->getType() != Type::intType &&
                        right->getType() != Type::doubleType)
        {
                ReportError::Formatted(right->GetLocation(),
                                "%s where int/double expected",
                                right->getType()->getTypeName());
                type = Type::errorType;
        }
}

void RelationalExpr::Check() {
        left->Check();
        op->Check();

        assert(left->getType());
        assert(right->getType());

        if (left->getType()->operator!=(right->getType()) ||
                        (left->getType() != Type::intType &&
                         left->getType() != Type::doubleType))
        {
                if (right->getType() != Type::errorType)
                {
                        ReportError::Formatted(op->GetLocation(),
                                        "Incompatible operands: %s %s %s",
                                        left->getType()->getTypeName(),
                                        op->getOp(),
                                        right->getType()->getTypeName());
                }
        }

        right->Check();
}

void LogicalExpr::Check() {
        //CompoundExpr::Check();
        //compound_expr_return_if_errors();

        if (left == nullptr)
        {
                /* unary not */
                if (strcmp(op->getOp(), "!") != 0)
                {
                        ReportError::Formatted(location,
                                        "Weird unary error");
                        type = Type::errorType;
                        return;
                }

                if (right->getType() != Type::boolType)
                {
                        ReportError::Formatted(op->GetLocation(),
                                        "Incompatible operand: ! %s",
                                        right->getType()->getTypeName());
                        type = Type::boolType;
                }
                else
                {
                        right->Check();
                }
        }
        else
        {
                left->Check();
                if (left->getType()->operator!=(right->getType()))
                {
                        ReportError::Formatted(op->GetLocation(),
                                        "Incompatible operands: %s %s %s",
                                        left->getType()->getTypeName(),
                                        op->getOp(),
                                        right->getType()->getTypeName());
                }
                else if (right->getType() != Type::boolType)
                {
                        ReportError::Formatted(op->GetLocation(),
                                        "Incompatible operands: %s %s %s",
                                        left->getType()->getTypeName(),
                                        op->getOp(),
                                        right->getType()->getTypeName());
                        //type = Type::errorType;
                        return;
                }
                right->Check();
        }
}

void EqualityExpr::Check() {
        CompoundExpr::Check();

        compound_expr_return_if_errors();

        if (left->getType()->operator!=(right->getType()))
        {
                if (left->getType()->isBasicType() || right->getType() != Type::nullType)
                {
                        ReportError::Formatted(op->GetLocation(),
                                        "Incompatible operands: %s %s %s",
                                        left->getType()->getTypeName(),
                                        op->getOp(),
                                        right->getType()->getTypeName());
                        //type = Type::errorType;
                        return;
                }
        }

        type = Type::boolType;
}

Type *FieldAccess::getType() {
        if (type != nullptr)
        {
                return type;
        }

        if (base != nullptr)
        {
                const Decl *cls = parent->getVariable(base->getType()->getTypeName());
                const Decl *var = nullptr;

                if (cls != nullptr)
                {
                        var = cls->getVariable(field->GetName());
                }
                else
                {
                        var = getVariable(field->GetName());
                }

                if (var == nullptr)
                {
                        type = Type::errorType;
                }
                else if(getThis() == nullptr &&
                                dynamic_cast<const VarDecl*>(var) != nullptr)
                {
                        type = Type::errorType;
                }
                else
                {
                        type = var->getType();
                }
        }
        else
        {
                const VarDecl *var = dynamic_cast<const VarDecl*>(getVariable(field->GetName()));

                if(var == nullptr)
                {
                        type = Type::errorType;
                }
                else
                {
                        type = var->getType();
                }
        }

        assert(type);
        return type;
}

void FieldAccess::Check() {
        if (base != nullptr)
        {
                /* this is the classname.functionname variant */
                base->Check();
                field->Check();

                const Decl *cls = parent->getVariable(base->getType()->getTypeName());
                const Decl *var = nullptr;

                if (cls != nullptr)
                {
                        var = cls->getVariable(field->GetName());
                }
                else
                {
                        var = getVariable(field->GetName());
                }

                if (var == nullptr && base->getType() != Type::errorType)
                {
                        ReportError::Formatted(field->GetLocation(),
                                        "%s has no such field '%s'",
                                        base->getType()->getTypeName(),
                                        field->GetName());
                        type = Type::errorType;
                }
                else if(getThis() == nullptr &&
                                dynamic_cast<const VarDecl*>(var) != nullptr)
                {
                        ReportError::Formatted(field->GetLocation(),
                                        "%s field '%s' only accessible within class scope",
                                        cls->getName(),
                                        var->getName());
                        type = Type::errorType;
                }
                else
                {
                        type = var->getType();
                }
        }
        else
        {
                /* this is the case where it's varname op */
                const VarDecl *var = dynamic_cast<const VarDecl*>(getVariable(field->GetName()));

                field->Check();

                if(var == nullptr)
                {
                        ReportError::Formatted(location,
                                        "No declaration found for variable '%s'",
                                        field->GetName());
                        type = Type::errorType;
                }
                else
                {
                        type = var->getType();
                }
        }
}

Type *ArrayAccess::getType() {
        if (type != nullptr)
        {
                return type;
        }

        const ArrayType *t = dynamic_cast<const ArrayType*>(base->getType());
        if (t == nullptr)
        {
                type = Type::errorType;
        }
        else
        {
                type = t->getBaseType();
        }

        return type;
}

void ArrayAccess::Check() {
        subscript->Check();

        if (subscript->getType() != Type::intType)
        {
                ReportError::Formatted(subscript->GetLocation(),
                                "Array subscript must be an integer",
                                subscript->getType()->getTypeName());
        }

        const ArrayType *t = dynamic_cast<const ArrayType*>(base->getType());
        if (t == nullptr)
        {
                if (dynamic_cast<ArrayAccess*>(base) != nullptr &&
                                base->getType() != Type::errorType)
                {
                        ReportError::Formatted(base->GetLocation(),
                                        "[] can only be applied to arrays");
                        type = Type::errorType;
                }
                else if (dynamic_cast<ArrayAccess*>(base) == nullptr)
                {
                        ReportError::Formatted(base->GetLocation(),
                                        "[] can only be applied to arrays");
                        type = Type::errorType;
                }
        }
        else
        {
                type = t->getBaseType();
        }

        base->Check();
}

void Operator::Check() {
        /* char tokenString[4] */
        /* TODO Check tokenString is actually an operator ? */
}

Type *Call::getType() {
        if (type != nullptr)
        {
                return type;
        }

        const FnDecl *fn = nullptr;

        if (base != nullptr)
        {
                const Decl *cls = parent->getVariable(base->getType()->getTypeName());
                if (cls == nullptr)
                {
                        type = Type::errorType;
                }
                else
                {
                        fn = dynamic_cast<const FnDecl*>(cls->getVariable(field->GetName()));
                        if (fn == nullptr)
                        {
                                type = Type::errorType;
                        }
                        else
                        {
                                type = fn->getType();
                        }
                }
        }
        else
        {
                fn = dynamic_cast<const FnDecl*>(parent->getVariable(field->GetName()));
                if (fn == nullptr)
                {
                        type = Type::errorType;
                }
                else
                {
                        type = fn->getType();
                }
        }

        if (fn == nullptr)
        {
                type = Type::errorType;
        }

        return type;
}

void Call::Check() {
        int i = 0;
        const FnDecl *fn = nullptr;

        if (base != nullptr)
        {
                base->Check();

                if (base->getType() == nullptr ||
                                base->getType() == Type::errorType)
                {
                        type = Type::errorType;
                        return;
                }

                const Decl *cls = parent->getVariable(base->getType()->getTypeName());
                if (cls == nullptr)
                {
                        const Type *t = base->getType();
                        assert(t);
                        if (t == Type::intType ||
                                        t == Type::doubleType ||
                                        t == Type::boolType ||
                                        t == Type::stringType)
                        {
                                ReportError::Formatted(field->GetLocation(),
                                                "%s has no such field '%s'",
                                                t->getTypeName(),
                                                field->GetName());
                        }
                        else if (dynamic_cast<const ArrayType*>(t) != nullptr &&
                                        strcmp(field->GetName(), "length") != 0)
                        {
                                ReportError::Formatted(field->GetLocation(),
                                                "%s has no such field '%s'",
                                                t->getTypeName(),
                                                field->GetName());
                        }
                        type = Type::errorType;
                        return;
                }

                field->Check();

                fn = dynamic_cast<const FnDecl*>(cls->getVariable(field->GetName()));
                if (fn == nullptr)
                {
                        ReportError::Formatted(field->GetLocation(),
                                        "%s has no such field '%s'",
                                        base->getType()->getTypeName(),
                                        field->GetName());
                        type = Type::errorType;
                }
                else
                {
                        type = fn->getType();
                }
        }
        else
        {
                fn = dynamic_cast<const FnDecl*>(parent->getVariable(field->GetName()));
                if (fn == nullptr)
                {
                        ReportError::Formatted(field->GetLocation(),
                                        "No declaration found for function '%s'",
                                        field->GetName());
                        type = Type::errorType;
                        return;
                }
                else
                {
                        type = fn->getType();
                }
        }

        for (i = 0; i < actuals->NumElements(); i++)
        {
                actuals->Nth(i)->Check();
        }

        if (fn == nullptr)
        {
                type = Type::errorType;
                return;
        }

        if (actuals->NumElements() != fn->NumFormals())
        {
                ReportError::Formatted(field->GetLocation(),
                                "Function '%s' expects %d arguments but %d given",
                                field->GetName(),
                                fn->NumFormals(),
                                actuals->NumElements());
                type = Type::errorType;
        }

        for (i = 0; i < actuals->NumElements() && i < fn->NumFormals(); i++)
        {
                const Type *actualType = actuals->Nth(i)->getType();
                if (actualType == Type::errorType)
                {
                        continue;
                }

                if(!fn->formalType(i)->isBasicType() && actualType == Type::nullType)
                {
                        continue;
                }

                if (actualType->operator!=(fn->formalType(i)))
                {
                        ReportError::Formatted(actuals->Nth(i)->GetLocation(),
                                        "Incompatible argument %d: %s given, %s expected",
                                        i + 1,
                                        actuals->Nth(i)->getType()->getTypeName(),
                                        fn->formalType(i)->getTypeName());
                        type = Type::errorType;
                }
        }
}

void NewArrayExpr::Check() {
        size->Check();

        if(size->getType()->operator!=(Type::intType))
        {
            ReportError::Formatted(size->GetLocation(),
                                        "Size for NewArray must be an integer");
            type = Type::errorType;
        }

        elemType->Check();
        const ArrayType *t = dynamic_cast<const ArrayType*>(elemType);
        assert(t);
        const Type *bt = t->getBaseType();

        type = elemType;
}

void NewExpr::Check() {
        const ClassDecl *cls = dynamic_cast<const ClassDecl*>(
                        parent->getVariable(cType->getTypeName()));
        if (cls == nullptr)
        {
                ReportError::Formatted(cType->GetLocation(),
                                "No declaration found for class '%s'",
                                cType->getTypeName());
                type = Type::errorType;
        }
        else
        {
                cType->Check();
                type = cType;
        }
}

void PostfixExpr::Check() {
        /* TODO Ensure that the op can be applied to the lvalue */
        lvalue->Check();
        op->Check();
}

void NullConstant::Check() {
    type = Type::nullType;
}

Type *This::getType()
{
        if (type != nullptr)
        {
                return type;
        }

        if (parent->getThis() == nullptr)
        {
                type = Type::errorType;
        }
        else
        {
                type = parent->getThis()->getType();
        }

        return type;
}

void This::Check() {
        if (getType() == Type::errorType)
        {
                ReportError::Formatted(location,
                                "'this' is only valid within class scope");
        }
}

void ReadIntegerExpr::Check() {
        type = Type::intType;
}

void ReadLineExpr::Check() {
        type = Type::stringType;
}

void EmptyExpr::Check() {
        type = nullptr;
}

Type *NewExpr::getType() {
        if (type != nullptr)
        {
                return type;
        }

        const ClassDecl *cls = dynamic_cast<const ClassDecl*>(
                        parent->getVariable(cType->getTypeName()));
        if (cls == nullptr)
        {
                type = Type::errorType;
        }
        else
        {
                type = cType;
        }

        return type;
}

void AssignExpr::Check() {
        left->Check();

        if(right->getType()->operator!=(left->getType()) &&
                        !right->getType()->isDescendedFrom(left->getType()))
        {
                if(right->getType() == Type::nullType)
                {
                    if(left->getType()->isBasicType())
                    {
                            ReportError::Formatted(op->GetLocation(),
                                "Incompatible operands: %s = %s",
                                left->getType()->getTypeName(),
                                right->getType()->getTypeName());
                    }
                }
                else if (right->getType() != Type::errorType &&
                                left->getType() != Type::errorType)
                {
                    ReportError::Formatted(op->GetLocation(),
                                "Incompatible operands: %s = %s",
                                left->getType()->getTypeName(),
                                right->getType()->getTypeName());
                }
        }

        right->Check();
}

const Decl *CompoundExpr::getVariable(const char *name) const
{
        return parent->getVariable(name);
}

const Decl *FieldAccess::getVariable(const char *name) const
{
        return parent->getVariable(name);
}

const Decl *Call::getVariable(const char *name) const
{
        return parent->getVariable(name);
}

const ClassDecl *Expr::getThis() const
{
        return parent->getThis();
}
