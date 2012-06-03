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

typedef struct
{
   char * input;
   int alloc;
   int length;
   int start;
} input_t;

typedef enum 
{
   C_COMB, C_MATCH, C_EXPECT, C_LIT, C_FORWARD
} ctag_t;

typedef enum
{
   T_NULL, T_INT, T_MUL, T_ADD, T_SUB, T_DIV, T_FORWARD, T_COMB
} tag_t;

typedef struct ast_t
{
   tag_t typ;
   struct ast_t * child;
   struct ast_t * next;
   char * str;
   long l;
} ast_t;

struct comb_t;

typedef struct
{
   ast_t * (*comb)(input_t *, struct comb_t *, struct comb_t *);
   struct comb_t * cl1;
   struct comb_t * cl2;
} closure_t;

typedef struct
{
   char * str;
   char * err;
} str2_t;

typedef struct comb_t
{
    ctag_t type;
    tag_t tag;
    union
    {  
       closure_t * clos;
       char * str;
       str2_t s2;
       ast_t * (*lit)(input_t *);
       struct comb_t ** forward;
    };
} comb_t;

comb_t * forward(comb_t ** comb);

comb_t * Integer();

ast_t * match(input_t * in, tag_t tag, char * str);

ast_t * expect(input_t * in, tag_t tag, char * str, char * err);

ast_t * comb_or(input_t * in, comb_t * c1, comb_t * c2);

ast_t * comb_and(input_t * in, comb_t * c1, comb_t * c2);

ast_t * comb_laloop(input_t * in, comb_t * c1, comb_t * c2);

comb_t * Match(tag_t tag, char * str);

comb_t * Expect(tag_t tag, char * str, char * err);

comb_t * Comb(ast_t * (*comb)(input_t *, struct comb_t *, struct comb_t *),
               comb_t * cl1, comb_t * cl2);

ast_t * parse(input_t * in, comb_t * c);

#define   And(x, y) Comb(comb_and, x, y)
#define    Or(x, y) Comb(comb_or, x, y)
#define Lloop(x, y) Comb(comb_laloop, x, y)

#define Combinator(x) \
   comb_t * x; \
   x = forward(&x);
