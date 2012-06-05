/*

Copyright 2012 William Hart. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY William Hart ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL William Hart OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdio.h>
#include "combinator.h"

ast_t * eval(ast_t * a)
{
   if (a == NULL)
      exception("missing operand\n");
  
   switch (a->typ)
   {
   case T_INT:
      break;
   case T_ADD:
      eval(a->child);
      eval(a->child->next);
      a->l = a->child->l + a->child->next->l;
      break;
   case T_SUB:
      eval(a->child);
      eval(a->child->next);
      a->l = a->child->l - a->child->next->l;
      break;
   case T_MUL:
      eval(a->child);
      eval(a->child->next);
      a->l = a->child->l * a->child->next->l;
      break;
   case T_DIV:
      eval(a->child);
      eval(a->child->next);
      a->l = a->child->l / a->child->next->l;
      break;
   default:
      exception("Unknown tag in AST node\n");
   }

   return a;
}

int main(void)
{
   ast_t * a;
   
   input_t in = { NULL, 0, 0, 0 };
   
   Combinator(Expr);
   Combinator(Term);
   Combinator(Factor);
   Combinator(Stmt);

   char * msg1 = "Error: \")\" expected!\n";

   Factor = Or(
              Integer(), 
              And(
                 And(
                    Match(T_NULL, "("), 
                    Expr
                 ), 
                 Expect(T_NULL, ")", msg1)
              )
            );

   Term   = Lloop(
                 Factor, 
                 Or(
                   Match(T_MUL, "*"), 
                   Match(T_DIV, "/")
                 )
            );

   Expr   = Lloop(
                 Term, 
                 Or(
                   Match(T_ADD, "+"), 
                   Match(T_SUB, "-")
                 )
            );
   
   Stmt   = And(
               Expr,
               Match(T_NULL, ";")
            );

   printf("Welcome to Calc v1.0\n\n");
   printf("> ");

   while (a = parse(&in, Stmt)) 
   {
      printf("%ld\n", eval(a)->l);
      printf("\n> ");
   }

   return 0;
}

