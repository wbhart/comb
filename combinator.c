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
#include <stdlib.h>
#include <string.h>
#include "combinator.h"

void exception(char * err)
{
   fprintf(stderr, err);
   abort();
}

ast_t * new_ast(tag_t typ, ast_t * child, ast_t * next)
{
   ast_t * ast = malloc(sizeof(ast_t));
   
   ast->typ = typ;
   ast->child = child;
   ast->next = next;
   
   return ast;
}

char read1(input_t * in)
{
   if (in->start < in->length)
      return in->input[in->start++];

   if (in->alloc == in->length)
   {
      in->input = realloc(in->input, in->alloc + 50);
      in->alloc += 50;
   }

   in->start++;
   return in->input[in->length++] = getchar();
}

void skip_whitespace(input_t * in)
{
   char c;

   while ((c = read1(in)) == ' ' || c == '\n' || c == '\t') ;

   in->start--;
}

ast_t * match(input_t * in, tag_t tag, char * str)
{
   int start = in->start;
   int i = 0, len = strlen(str);
   
   skip_whitespace(in);
   
   while (i < len && str[i] == read1(in)) i++;
   
   if (i != len)
   {
      in->start = start;
      return NULL;
   }

   return new_ast(tag, NULL, NULL);
}

ast_t * expect(input_t * in, tag_t tag, char * str, char * err)
{
   ast_t * ast;
   
   if (!(ast = match(in, tag, str)))
      exception(err);

   return ast;
}

ast_t * parse(input_t * in, comb_t * c)
{
   switch (c->type)
   {
   case C_COMB:
      return c->clos->comb(in, c->clos->cl1, c->clos->cl2);
   case C_MATCH:
      return match(in, c->tag, c->str);
   case C_EXPECT:
      return expect(in, c->tag, c->s2.str, c->s2.err);
   case C_LIT:
      return c->lit(in);
   case C_FORWARD:
      return parse(in, *c->forward);
   default:
      printf("tag = %d\n", c->type);
      exception("Unknown tag in parse()\n");
   }
}

ast_t * comb_and(input_t * in, comb_t * c1, comb_t * c2)
{
   ast_t * a1, * a2;
   int start = in->start;

   if ((a1 = parse(in, c1)) && (a2 = parse(in, c2)))
   {
      if (c1->tag != T_NULL)
      {
         if (c2->tag != T_NULL)
         {
            a1->next = a2;
            return a1;
         } else
            return a1;
      }
      if (c2->tag != T_NULL)
         return a2;
   }

   in->start = start;
   return NULL;
}

ast_t * comb_or(input_t * in, comb_t * c1, comb_t * c2)
{
   ast_t * a;
   int start = in->start;

   if (a = parse(in, c1))
      return a;
   
   in->start = start;

   if (a = parse(in, c2))
      return a;

   in->start = start;
   return NULL;
}

ast_t * comb_laloop(input_t * in, comb_t * c1, comb_t * c2)
{
   ast_t * a, * b, * t, *op;
   int start = in->start;

   if (a = parse(in, c1))
   {
      start = in->start;
      while (op = parse(in, c2))
      {
         b = parse(in, c1);
         a->next = b;
         op->child = a;
         a = op;
         start = in->start;
      }
      
      in->start = start;
      return a;
   }
   
   in->start = start;
   return NULL;
}

comb_t * Match(tag_t tag, char * str)
{
   comb_t * c = malloc(sizeof(comb_t));
   
   c->type = C_MATCH;
   c->tag = tag;
   c->str = str;

   return c;
}

comb_t * Expect(tag_t tag, char * str, char * err)
{
   comb_t * c = malloc(sizeof(comb_t));
   
   c->type = C_EXPECT;
   c->tag = tag;
   c->s2.str = str;
   c->s2.err = err;

   return c;
}

comb_t * Comb(ast_t * (*comb)(input_t *, struct comb_t *, struct comb_t *),
               comb_t * cl1, comb_t * cl2)
{
   closure_t * clos = malloc(sizeof(closure_t));

   clos->comb = comb;
   clos->cl1 = cl1;
   clos->cl2 = cl2;

   comb_t * c = malloc(sizeof(comb_t));

   c->type = C_COMB;
   c->tag = T_COMB;
   c->clos = clos;

   return c;
}

#define str_insert(s, c) \
   do { \
      if (i == alloc) { \
         s = realloc(s, alloc + 10); \
         alloc += 10; \
      } \
      str[i++] = c; \
   } while (0)

ast_t * integer(input_t * in)
{
   int start = in->start;
   char c;
   int i = 0, alloc = 0;
   char * str = NULL;
   ast_t * a;
   
   skip_whitespace(in);
   
   if ((c = read1(in)) >= '1' && c <= '9')
   {
      str_insert(str, c);

      start = in->start;
      while (isdigit(c = read1(in)))
      {
         str_insert(str, c);
         start = in->start;
      }      
      in->start = start;

      str_insert(str, '\0');
      
      a = new_ast(T_INT, NULL, NULL);
      a->l = atol(str);
      free(str);

      return a;
   } else if (c == '0')
   {
      str_insert(str, c);
      
      str_insert(str, '\0');
      a = new_ast(T_INT, NULL, NULL);
      a->l = atol(str);
      free(str);

      return a;
   }

   in->start = start;
   return NULL;
}

comb_t * Integer()
{
   comb_t * c = malloc(sizeof(comb_t));
   
   c->type = C_LIT;
   c->tag = T_INT;
   c->lit = integer;

   return c;
}

comb_t * forward(comb_t ** comb)
{
   comb_t * c = malloc(sizeof(comb_t));
   
   c->type = C_FORWARD;
   c->tag = T_FORWARD;
   c->forward = comb;

   return c;
}
