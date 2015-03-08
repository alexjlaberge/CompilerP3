#include "symbols.h"

Hashtable<Decl*> declared_types;
Hashtable<VarDecl*> declared_variables;
std::multimap<int, List<VarDecl*> > scoped_variables;