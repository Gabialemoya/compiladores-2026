/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_SRC_PARSER_PARSER_TAB_H_INCLUDED
# define YY_YY_SRC_PARSER_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 15 "src/parser/parser.y"

    #include "../intermedio/tercetos.h"

#line 53 "src/parser/parser.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ID = 258,                      /* ID  */
    CTE = 259,                     /* CTE  */
    CADENA = 260,                  /* CADENA  */
    OP_ASIG = 261,                 /* OP_ASIG  */
    OP_IGUAL = 262,                /* OP_IGUAL  */
    OP_SUMA = 263,                 /* OP_SUMA  */
    OP_RESTA = 264,                /* OP_RESTA  */
    OP_MUL = 265,                  /* OP_MUL  */
    OP_DIV = 266,                  /* OP_DIV  */
    OP_MENOR = 267,                /* OP_MENOR  */
    OP_MENOR_IGUAL = 268,          /* OP_MENOR_IGUAL  */
    OP_MAYOR = 269,                /* OP_MAYOR  */
    OP_MAYOR_IGUAL = 270,          /* OP_MAYOR_IGUAL  */
    OP_DISTINTO = 271,             /* OP_DISTINTO  */
    PUNTO_Y_COMA = 272,            /* PUNTO_Y_COMA  */
    PAR_ABRE = 273,                /* PAR_ABRE  */
    PAR_CIERRA = 274,              /* PAR_CIERRA  */
    LLAVE_ABRE = 275,              /* LLAVE_ABRE  */
    LLAVE_CIERRA = 276,            /* LLAVE_CIERRA  */
    IF = 277,                      /* IF  */
    ELSE = 278,                    /* ELSE  */
    VAR = 279,                     /* VAR  */
    LOOP = 280,                    /* LOOP  */
    UNTIL = 281,                   /* UNTIL  */
    OR = 282,                      /* OR  */
    AND = 283,                     /* AND  */
    WRITE = 284,                   /* WRITE  */
    MAIN = 285,                    /* MAIN  */
    RETURN = 286                   /* RETURN  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 19 "src/parser/parser.y"

    int        indice_ts;   /* lo carga yylex para ID, CTE, CADENA */
    Referencia ref;         /* lo propagan las reglas              */
    int        operador;    /* op_comparacion y op_logico          */

#line 107 "src/parser/parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_SRC_PARSER_PARSER_TAB_H_INCLUDED  */
