#include "symbols.h"
#include <string>

using std::string;

Hashtable<ClassDecl*> declared_classes;
Hashtable<FnDecl*> declared_functions;
Hashtable <InterfaceDecl*> declared_interfaces;


bool type_exists(const char *name)
{
        string actual;

        if (strstr(name, "[]") != nullptr)
        {
                for (int i = 0; name[i] != '['; i++)
                {
                        actual += name[i];
                }
        }
        else
        {
                actual = string(name);
        }

        if (declared_classes.Lookup(name) != nullptr)
        {
                return true;
        }

        if (strcmp(actual.c_str(), "int") == 0 ||
                        strcmp(actual.c_str(), "double") == 0 ||
                        strcmp(actual.c_str(), "void") == 0 ||
                        strcmp(actual.c_str(), "bool") == 0 ||
                        strcmp(actual.c_str(), "null") == 0 ||
                        strcmp(actual.c_str(), "string") == 0 ||
                        strcmp(actual.c_str(), "error") == 0)
        {
                return true;
        }

        return false;
}

bool add_type(const char *name, ClassDecl *decl)
{
        if (declared_classes.Lookup(name) != nullptr)
        {
                return false;
        }

        declared_classes.Enter(name, decl);
        return true;
}
