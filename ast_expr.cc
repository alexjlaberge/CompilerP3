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

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
    type = new Type("int");
}
void IntConstant::PrintChildren(int indentLevel) { 
    printf("%d", value);
}

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
    type = new Type("double");
}
void DoubleConstant::PrintChildren(int indentLevel) { 
    printf("%g", value);
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
    type = new Type("bool");
}
void BoolConstant::PrintChildren(int indentLevel) { 
    printf("%s", value ? "true" : "false");
}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc) {
    Assert(val != NULL);
    value = strdup(val);
    type = new Type("string");
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
        /* TODO Check that both operands and operator are compatible */
        printf("COMP");
		string leftType;
		string rightType;
		string opType;
        if (left != nullptr)
        {
                left->Check();
        }
        op->Check();
        right->Check();
        //if(left->getType() != nullptr && right->getType() != nullptr)
        //printf("%s, %s\n", left->getType()->getTypeName(), right->getType()->getTypeName());
}

void ArithmeticExpr::Check() {
    left->Check();
    op->Check();
    right->Check();

    assert(left->getType());
    assert(right->getType());

    if (left->getType() == Type::errorType ||
                    right->getType() == Type::errorType)
    {
            type = Type::errorType;
            return;
    }

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

Type* ArithmeticExpr::getType() {
	return type;
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

Type* RelationalExpr::getType() {
	return type;
}

void LogicalExpr::Check() {
        left->Check();
        op->Check();
        right->Check();

        assert(left->getType());
        assert(right->getType());

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

Type* LogicalExpr::getType() {
	return type;
}

void EqualityExpr::Check() {
	
}

Type* EqualityExpr::getType() {
	return type;
}

Type* AssignExpr::getType() {
	return type;
}

void FieldAccess::Check() {
        //printf("%s \n", field->GetName());
        //printf("%s", type->getTypeName());

        if (base != nullptr)
        {
                /* This is the case where it's classname.methodname */
                base->Check();
        }
        else
        {
                /* this is the case where it's varname op */
                VarDecl* d = declared_variables.Lookup(field->GetName());

                field->Check();

                if(d == nullptr)
                {
                        ReportError::Formatted(location,
                                        "No declaration found for variable '%s'",
                                        field->GetName());
                        return;
                }
                type = d->getType();
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
        }

        field->Check();

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
        }
        elemType->Check();
        type = elemType;
}

void NewExpr::Check() {
        if (!cType->IsDeclared())
        {
                ReportError::Formatted(cType->GetLocation(),
                                "No declaration found for class '%s'",
                                cType->GetId()->GetName());
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
}

void ReadIntegerExpr::Check() {
}

void ReadLineExpr::Check() {
}

void EmptyExpr::Check() {
}

void LValue::Check() {
}

void AssignExpr::Check() {
        left->Check();
        op->Check();
        right->Check();

        if (left->getType() == Type::errorType ||
                        right->getType() == Type::errorType)
        {
                return;
        }

        assert(left->getType());
        assert(right->getType());

        if(right->getType()->operator!=(left->getType()))
        {
                ReportError::Formatted(op->GetLocation(),
                                "Incompatible operands: %s = %s",
                                left->getType()->getTypeName(),
                                right->getType()->getTypeName());
        }
}
