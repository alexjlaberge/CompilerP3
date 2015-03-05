/* File: main.cc
 * -------------
 * This file defines the main() routine for the program and not much else.
 * You should not need to modify this file.
 */
 
#include <string.h>
#include <stdio.h>
#include "utility.h"
#include "errors.h"
#include "parser.h"
#include "symbols.h"


/* Function: main()
 * ----------------
 * Entry point to the entire program.  We parse the command line and turn
 * on any debugging flags requested by the user when invoking the program.
 * InitScanner() is used to set up the scanner.
 * InitParser() is used to set up the parser. The call to yyparse() will
 * attempt to parse a complete program from the input. 
 */
int main(int argc, char *argv[])
{
        declared_types.Enter("int", (Decl *) Type::intType);
        declared_types.Enter("double", (Decl *) Type::doubleType);
        declared_types.Enter("void", (Decl *) Type::voidType);
        declared_types.Enter("bool", (Decl *) Type::boolType);
        declared_types.Enter("null", (Decl *) Type::nullType);
        declared_types.Enter("string", (Decl *) Type::stringType);
        declared_types.Enter("error", (Decl *) Type::errorType);

        ParseCommandLine(argc, argv);

        InitScanner();
        InitParser();
        yyparse();
        return (ReportError::NumErrors() == 0? 0 : -1);
}

