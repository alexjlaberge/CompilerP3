#ifndef _H_SYMBOLS
#define _H_SYMBOLS

#include "hashtable.h"
#include "ast_decl.h"

extern Hashtable<ClassDecl*> declared_classes;
extern Hashtable<FnDecl*> declared_functions;
extern Hashtable <InterfaceDecl*> declared_interfaces;

bool type_exists(const char *name);

bool add_type(const char *name, ClassDecl *decl);

#endif /* _H_SYMBOLS */
