#include "symbols.h"

int scope;
Hashtable<Decl*> declared_types;
Hashtable<VarDecl*> declared_variables;
Hashtable<int> scoped_variables;
Hashtable<Type*> fn_types;