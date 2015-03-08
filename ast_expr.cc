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

#define compound_expr_return_if_errors() \
    if (left->getType() == Type::errorType || \
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
    (elemType=et)->SetParent(this);
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

void BreakStmt::Check() {
//Check that it is within the scope of a while or for loop
    Node* p = parent;
    while(p != nullptr)
    {    if(p->isBreakable())
            return;
         else
            p = p->GetParent();
    }
    ReportError::Formatted(location, "break is only allowed inside a loop");
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
        left->setLevel(level);
        right->setLevel(level);
        if (left != nullptr)
        {
                left->Check();

                type_assert(left->getType());
        }

        op->Check();

        right->Check();
        type_assert(right->getType());
}

void ArithmeticExpr::Check() {
        CompoundExpr::Check();
        compound_expr_return_if_errors();
    if (left->getType()->operator!=(right->getType()))
    {
        ReportError::Formatted(op->GetLocation(), "Incompatible operands: %s %s %s", left->getType()->getTypeName(), op->getOp(), right->getType()->getTypeName());
        type = Type::errorType;
    }
    else
    {
        type = left->getType();
    }

}

void RelationalExpr::Check() {
        left->Check();
        op->Check();
        right->Check();

        assert(left->getType());
        assert(right->getType());

        if (left->getType()->operator!=(right->getType()))
        {
                ReportError::Formatted(location,
                                "Cannot compare %s with %s",
                                left->getType()->getTypeName(),
                                right->getType()->getTypeName());
                type = Type::errorType;
        }
        else if(left->getType()->operator!=(Type::intType) &&
                        left->getType()->operator!=(Type::doubleType))
        {
                ReportError::Formatted(location,
                                "Must compare numbers");
                type = Type::errorType;
        }
        else
        {
                type = Type::boolType;
        }
}

void LogicalExpr::Check() {
        CompoundExpr::Check();

        compound_expr_return_if_errors();

        if (left->getType()->operator!=(right->getType()) ||
                        left->getType()->operator!=(Type::boolType))
        {
                ReportError::Formatted(location,
                                "Result of logical expression must be a bool");
                type = Type::errorType;
        }
        else
        {
                type = Type::boolType;
        }
}

void EqualityExpr::Check() {
        CompoundExpr::Check();

        compound_expr_return_if_errors();

        if (left->getType() != Type::intType &&
                        left->getType() != Type::doubleType)
        {
                ReportError::Formatted(left->GetLocation(),
                                "Operand must be numerical");
                type = Type::errorType;
                return;
        }

        if (right->getType() != Type::intType &&
                        right->getType() != Type::doubleType)
        {
                ReportError::Formatted(right->GetLocation(),
                                "Operand must be numerical");
                type = Type::errorType;
                return;
        }

        if (left->getType()->operator!=(right->getType()))
        {
                ReportError::Formatted(location,
                                "Operands %s and %s are not same type",
                                left->getType()->getTypeName(),
                                right->getType()->getTypeName());
                type = Type::errorType;
                return;
        }

        type = Type::boolType;
}

void FieldAccess::Check() {

        //std::cout << "Found " << field->GetName() << " at " << scoped_variables.Lookup(field->GetName()) << std::endl;
        //std::cout << "Awe Yee " << field->GetName() << std::endl<<std::flush;
        if (base != nullptr)
        {
                base->Check();
                field->Check();

                const Decl *cls = parent->getVariable(base->getType()->getTypeName());
                const Decl *var = cls->getVariable(field->GetName());
                std::cout << base->getType()->getTypeName() << std::endl;
                if (var == nullptr)
                {
                        ReportError::Formatted(location,
                                        "Class %s does not have variable %s",
                                        base->getType()->getTypeName(),
                                        field->GetName());
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
                const Decl *var = getVariable(field->GetName());

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

void ArrayAccess::Check() {
        base->Check();
        subscript->Check();
        if (subscript->getType()->operator!=(Type::intType))
        {
                /* TODO ERROR */
        }
}

void Operator::Check() {
        /* char tokenString[4] */
        /* TODO Check tokenString is actually an operator ? */
}

void Call::Check() {
        int i = 0;

        if (base != nullptr)
        {
                base->Check();

                if (base->getType() == Type::errorType)
                {
                        type = Type::errorType;
                        return;
                }

                if (base->getType() == nullptr)
                {
                        type = Type::errorType;
                        return;
                }

                if (parent->getVariable(base->getType()->getTypeName()) == nullptr)
                {
                        type = Type::errorType;
                        return;
                }

                field->Check();

                const Decl *cls = parent->getVariable(base->getType()->getTypeName());
                if (cls == nullptr)
                {
                        ReportError::Formatted(base->GetLocation(),
                                        "No such class");
                }

                const Decl *fn = cls->getVariable(field->GetName());
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
                const Decl *fn = parent->getVariable(field->GetName());
                if (fn == nullptr)
                {
                        ReportError::Formatted(field->GetLocation(),
                                        "No declaration found for function '%s'",
                                        field->GetName());
                        type = Type::errorType;
                }
                else
                {
                        type = fn->getType();
                }
        }

        while (i < actuals->NumElements())
        {
                actuals->Nth(i)->Check();
                i++;
        }
}

void NewArrayExpr::Check() {
        if(size->getType()->operator!=(Type::intType))
        {
            ReportError::Formatted(location,
                                        "Size for NewArray must be an integer");
            type = Type::errorType;
        }
        elemType->Check();
        type = elemType;
}

void NewExpr::Check() {
        const Decl *cls = parent->getVariable(cType->getTypeName());

        if (cls == nullptr)
        {
                ReportError::Formatted(cType->GetLocation(),
                                "No declaration found for class '%s'",
                                cType->GetId()->GetName());
                type = Type::errorType;
        }
        else
        {
                type = cType;
        }
}

void PostfixExpr::Check() {
        /* TODO Ensure that the op can be applied to the lvalue */
        lvalue->Check();
        op->Check();
}

void NullConstant::Check() {
}

void This::Check() {
        if (parent->getThis() == nullptr)
        {
                ReportError::Formatted(location,
                                "'this' used outside of class function");
                type = Type::errorType;
        }
        else
        {
                type = parent->getThis()->getType();
        }
}

void ReadIntegerExpr::Check() {
        type = Type::intType;
}

void ReadLineExpr::Check() {
        type = Type::stringType;
}

void EmptyExpr::Check() {
        type = Type::nullType;
}

void LValue::Check() {
}

void AssignExpr::Check() {

        left->setLevel(level);
        right->setLevel(level);
        CompoundExpr::Check();

        compound_expr_return_if_errors();

        if(right->getType()->operator!=(left->getType()))
        {
                ReportError::Formatted(op->GetLocation(),
                                "Incompatible operands: %s = %s",
                                left->getType()->getTypeName(),
                                right->getType()->getTypeName());
        }
}

const Decl *CompoundExpr::getVariable(const char *name) const
{
        return parent->getVariable(name);
}

const Decl *FieldAccess::getVariable(const char *name) const
{
        if (base != nullptr)
        {
                return base->getVariable(name);
        }

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
