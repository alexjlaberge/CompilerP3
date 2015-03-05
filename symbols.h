#ifndef _H_SYMBOLS
#define _H_SYMBOLS

#include "hashtable.h"
#include "ast_decl.h"

extern Hashtable<Decl*> declared_types;
extern Hashtable<VarDecl*> declared_variables;

#endif /* _H_SYMBOLS */
