#ifndef _H_SYMBOLS
#define _H_SYMBOLS

#include "hashtable.h"
#include "ast_decl.h"

extern int scope;
extern Hashtable<Decl*> declared_types;
extern Hashtable<VarDecl*> declared_variables;
extern Hashtable<int> scoped_variables;
extern Hashtable<Type*> fn_types;

#endif /* _H_SYMBOLS */
