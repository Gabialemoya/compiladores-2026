/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "src/parser/parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../intermedio/tercetos.h"
#include "../lexico/lexico.h"

/* Nombre de la función cuyo cuerpo se está traduciendo. */
static const char *funcion_actual = "main";

#define MAX_FUNCIONES 64
#define MAX_PENDIENTES 256

typedef struct {
    int nombre;
    int parametro;
    int entrada;
} InfoFuncion;

typedef struct {
    int nombre;
    int asig_arg;
    int call;
    int asig_ret;
} LlamadaPendiente;

static InfoFuncion funciones[MAX_FUNCIONES];
static int cant_funciones = 0;
static LlamadaPendiente pendientes[MAX_PENDIENTES];
static int cant_pendientes = 0;
static int cant_tmp = 0;

/* Celda estática del valor de retorno. T_RET no lleva operandos. */
static int celda_retorno_de(const char *nombre) {
    char celda[MAX_LEXEMA];

    snprintf(celda, sizeof celda, "@ret_%s", nombre);
    return agregar_ts(celda, "racional", "", (int)strlen(celda));
}

static int nuevo_temporal(void) {
    char nombre[MAX_LEXEMA];

    snprintf(nombre, sizeof nombre, "@tmp%d", cant_tmp++);
    return agregar_ts(nombre, "racional", "", (int)strlen(nombre));
}

static int buscar_funcion(int nombre) {
    int i;

    for (i = 0; i < cant_funciones; i++) {
        if (funciones[i].nombre == nombre)
            return i;
    }
    return -1;
}

static void poner_ref(int indice, int operando, Referencia ref) {
    if (indice < 0 || indice >= cant_tercetos)
        return;
    if (operando == 1)
        tercetos[indice].op1 = ref;
    else
        tercetos[indice].op2 = ref;
}

static void completar_llamada(int asig_arg, int call, int asig_ret,
                              int parametro, int entrada, int celda) {
    poner_ref(asig_arg, 1, ref_ts(parametro));
    completar_op1(call, entrada);
    poner_ref(asig_ret, 2, ref_ts(celda));
}

static void registrar_funcion(int nombre, int parametro) {
    int i;
    int pos;
    int entrada;
    int celda;

    if (nombre < 0 || nombre >= ts_size)
        return;

    funcion_actual = ts[nombre].nombre;
    celda = celda_retorno_de(funcion_actual);
    entrada = proximo();

    pos = buscar_funcion(nombre);
    if (pos < 0) {
        if (cant_funciones >= MAX_FUNCIONES) {
            printf("Linea %d: Error: demasiadas funciones\n", linea_actual);
            hay_errores = 1;
            return;
        }
        pos = cant_funciones++;
    }

    funciones[pos].nombre = nombre;
    funciones[pos].parametro = parametro;
    funciones[pos].entrada = entrada;

    for (i = 0; i < cant_pendientes; i++) {
        if (pendientes[i].nombre != nombre)
            continue;
        completar_llamada(pendientes[i].asig_arg, pendientes[i].call,
                          pendientes[i].asig_ret, parametro, entrada, celda);
        pendientes[i].nombre = -1;
    }
}

/* Copia el argumento al parámetro, salta a la entrada y conserva el retorno
   en un temporal. Si la función todavía no se vio, los tres tercetos quedan
   pendientes y se completan al registrar el encabezado. */
static Referencia emitir_llamada(int nombre, Referencia argumento) {
    int pos;
    int temporal;
    int asig_arg;
    int call;
    int asig_ret;
    int celda;

    temporal = nuevo_temporal();
    if (nombre < 0 || nombre >= ts_size || temporal < 0)
        return ref_vacia();

    pos = buscar_funcion(nombre);
    if (pos >= 0) {
        celda = celda_retorno_de(ts[nombre].nombre);
        emitir(T_ASIG, ref_ts(funciones[pos].parametro), argumento, TIPO_NINGUNO);
        emitir(T_CALL, ref_dir(funciones[pos].entrada), ref_vacia(), TIPO_NINGUNO);
        emitir(T_ASIG, ref_ts(temporal), ref_ts(celda), TIPO_NINGUNO);
        return ref_ts(temporal);
    }

    asig_arg = emitir(T_ASIG, ref_vacia(), argumento, TIPO_NINGUNO);
    call = emitir(T_CALL, ref_vacia(), ref_vacia(), TIPO_NINGUNO);
    asig_ret = emitir(T_ASIG, ref_ts(temporal), ref_vacia(), TIPO_NINGUNO);

    if (cant_pendientes >= MAX_PENDIENTES) {
        printf("Linea %d: Error: demasiadas llamadas pendientes\n", linea_actual);
        hay_errores = 1;
        return ref_ts(temporal);
    }

    pendientes[cant_pendientes].nombre = nombre;
    pendientes[cant_pendientes].asig_arg = asig_arg;
    pendientes[cant_pendientes].call = call;
    pendientes[cant_pendientes].asig_ret = asig_ret;
    cant_pendientes++;
    return ref_ts(temporal);
}

/* Una condición cae al terceto siguiente si es verdadera.
   Los BF de falso[] saltan si es falsa. Los BI de verdadero[] saltan si es verdadera. */
static Cond cond_vacia(void) {
    Cond c;
    c.nfalso = 0;
    c.nverdadero = 0;
    return c;
}

static Cond agregar_salto(Cond c, int verdadero, int salto) {
    if (verdadero) {
        if (c.nverdadero >= MAX_SALTOS_COND) {
            printf("Linea %d: Error: condicion demasiado compleja\n", linea_actual);
            hay_errores = 1;
            return c;
        }
        c.verdadero[c.nverdadero++] = salto;
    } else {
        if (c.nfalso >= MAX_SALTOS_COND) {
            printf("Linea %d: Error: condicion demasiado compleja\n", linea_actual);
            hay_errores = 1;
            return c;
        }
        c.falso[c.nfalso++] = salto;
    }
    return c;
}

static void completar_falsos(Cond c, int destino) {
    int i;
    for (i = 0; i < c.nfalso; i++)
        completar_op2(c.falso[i], destino);
}

static void completar_verdaderos(Cond c, int destino) {
    int i;
    for (i = 0; i < c.nverdadero; i++)
        completar_op1(c.verdadero[i], destino);
}

static Cond condicion_simple(int operador, Referencia izq, Referencia der) {
    int cmp = emitir((OperadorTerceto)operador, izq, der, TIPO_ENTERO);
    int salto = emitir(T_BF, ref_ter(cmp), ref_vacia(), TIPO_NINGUNO);
    return agregar_salto(cond_vacia(), 0, salto);
}

/* and: si la izquierda es verdadera, seguir con la derecha. */
static Cond preparar_and(Cond izq) {
    completar_verdaderos(izq, proximo());
    izq.nverdadero = 0;
    return izq;
}

/* or: si la izquierda es verdadera, saltear la derecha.
   Si es falsa, los BF quedan apuntando al inicio de la derecha. */
static Cond preparar_or(Cond izq) {
    int salto = emitir(T_BI, ref_vacia(), ref_vacia(), TIPO_NINGUNO);

    izq = agregar_salto(izq, 1, salto);
    completar_falsos(izq, proximo());
    izq.nfalso = 0;
    return izq;
}

static Cond preparar_logico(Cond izq, int es_and) {
    if (es_and)
        return preparar_and(izq);
    return preparar_or(izq);
}

static Cond juntar_falsos(Cond a, Cond b) {
    int i;
    for (i = 0; i < b.nfalso; i++)
        a = agregar_salto(a, 0, b.falso[i]);
    return a;
}

static Cond juntar_verdaderos(Cond a, Cond b) {
    int i;
    for (i = 0; i < b.nverdadero; i++)
        a = agregar_salto(a, 1, b.verdadero[i]);
    return a;
}

static Cond combinar_logico(Cond izq, int es_and, Cond der) {
    Cond r;

    if (es_and) {
        r = juntar_falsos(izq, der);
        r.nverdadero = 0;
        return juntar_verdaderos(r, der);
    }
    r = der;
    return juntar_verdaderos(r, izq);
}

#define MAX_SALTOS 64
static Cond pila_cond[MAX_SALTOS];
static int pila_bi[MAX_SALTOS];
static int tope_saltos = 0;

static void abrir_if(Cond condicion) {
    if (tope_saltos >= MAX_SALTOS) {
        printf("Linea %d: Error: anidamiento de if demasiado profundo\n", linea_actual);
        hay_errores = 1;
        return;
    }
    completar_verdaderos(condicion, proximo());
    condicion.nverdadero = 0;
    pila_cond[tope_saltos] = condicion;
    pila_bi[tope_saltos] = -1;
    tope_saltos++;
}

static void abrir_else(void) {
    int salto;

    if (tope_saltos <= 0)
        return;
    salto = emitir(T_BI, ref_vacia(), ref_vacia(), TIPO_NINGUNO);
    completar_falsos(pila_cond[tope_saltos - 1], proximo());
    pila_cond[tope_saltos - 1].nfalso = 0;
    pila_bi[tope_saltos - 1] = salto;
}

static void cerrar_if(void) {
    if (tope_saltos <= 0)
        return;
    tope_saltos--;
    if (pila_bi[tope_saltos] >= 0)
        completar_op1(pila_bi[tope_saltos], proximo());
    else
        completar_falsos(pila_cond[tope_saltos], proximo());
}

/* until: la condición verdadera cae afuera. La falsa vuelve al cuerpo. */
static void repetir_si_falso(Cond condicion, int inicio) {
    completar_verdaderos(condicion, proximo());
    completar_falsos(condicion, inicio);
}

/* El terceto 0 salta al cuerpo de main. Las funciones quedan delante y solo se entra con CALL. */
static int salto_entrada = -1;

static void abrir_programa(void) {
    salto_entrada = emitir(T_BI, ref_vacia(), ref_vacia(), TIPO_NINGUNO);
}

static void comenzar_main(void) {
    funcion_actual = "main";
    if (salto_entrada >= 0)
        completar_op1(salto_entrada, proximo());
}

#line 377 "src/parser/parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_ID = 3,                         /* ID  */
  YYSYMBOL_CTE = 4,                        /* CTE  */
  YYSYMBOL_CADENA = 5,                     /* CADENA  */
  YYSYMBOL_OP_ASIG = 6,                    /* OP_ASIG  */
  YYSYMBOL_OP_IGUAL = 7,                   /* OP_IGUAL  */
  YYSYMBOL_OP_SUMA = 8,                    /* OP_SUMA  */
  YYSYMBOL_OP_RESTA = 9,                   /* OP_RESTA  */
  YYSYMBOL_OP_MUL = 10,                    /* OP_MUL  */
  YYSYMBOL_OP_DIV = 11,                    /* OP_DIV  */
  YYSYMBOL_OP_MENOR = 12,                  /* OP_MENOR  */
  YYSYMBOL_OP_MENOR_IGUAL = 13,            /* OP_MENOR_IGUAL  */
  YYSYMBOL_OP_MAYOR = 14,                  /* OP_MAYOR  */
  YYSYMBOL_OP_MAYOR_IGUAL = 15,            /* OP_MAYOR_IGUAL  */
  YYSYMBOL_OP_DISTINTO = 16,               /* OP_DISTINTO  */
  YYSYMBOL_PUNTO_Y_COMA = 17,              /* PUNTO_Y_COMA  */
  YYSYMBOL_PAR_ABRE = 18,                  /* PAR_ABRE  */
  YYSYMBOL_PAR_CIERRA = 19,                /* PAR_CIERRA  */
  YYSYMBOL_LLAVE_ABRE = 20,                /* LLAVE_ABRE  */
  YYSYMBOL_LLAVE_CIERRA = 21,              /* LLAVE_CIERRA  */
  YYSYMBOL_IF = 22,                        /* IF  */
  YYSYMBOL_ELSE = 23,                      /* ELSE  */
  YYSYMBOL_VAR = 24,                       /* VAR  */
  YYSYMBOL_LOOP = 25,                      /* LOOP  */
  YYSYMBOL_UNTIL = 26,                     /* UNTIL  */
  YYSYMBOL_OR = 27,                        /* OR  */
  YYSYMBOL_AND = 28,                       /* AND  */
  YYSYMBOL_WRITE = 29,                     /* WRITE  */
  YYSYMBOL_MAIN = 30,                      /* MAIN  */
  YYSYMBOL_RETURN = 31,                    /* RETURN  */
  YYSYMBOL_YYACCEPT = 32,                  /* $accept  */
  YYSYMBOL_programa = 33,                  /* programa  */
  YYSYMBOL_entrada = 34,                   /* entrada  */
  YYSYMBOL_lista_funciones = 35,           /* lista_funciones  */
  YYSYMBOL_definicion_funcion = 36,        /* definicion_funcion  */
  YYSYMBOL_37_1 = 37,                      /* $@1  */
  YYSYMBOL_funcion_main = 38,              /* funcion_main  */
  YYSYMBOL_39_2 = 39,                      /* $@2  */
  YYSYMBOL_cuerpo = 40,                    /* cuerpo  */
  YYSYMBOL_declaraciones = 41,             /* declaraciones  */
  YYSYMBOL_declaracion = 42,               /* declaracion  */
  YYSYMBOL_lista_sentencias = 43,          /* lista_sentencias  */
  YYSYMBOL_sentencia = 44,                 /* sentencia  */
  YYSYMBOL_asignacion = 45,                /* asignacion  */
  YYSYMBOL_retorno = 46,                   /* retorno  */
  YYSYMBOL_salida = 47,                    /* salida  */
  YYSYMBOL_seleccion = 48,                 /* seleccion  */
  YYSYMBOL_49_3 = 49,                      /* $@3  */
  YYSYMBOL_fin_seleccion = 50,             /* fin_seleccion  */
  YYSYMBOL_51_4 = 51,                      /* $@4  */
  YYSYMBOL_iteracion = 52,                 /* iteracion  */
  YYSYMBOL_53_5 = 53,                      /* @5  */
  YYSYMBOL_bloque = 54,                    /* bloque  */
  YYSYMBOL_condiciones = 55,               /* condiciones  */
  YYSYMBOL_56_6 = 56,                      /* @6  */
  YYSYMBOL_condicion = 57,                 /* condicion  */
  YYSYMBOL_expresion = 58,                 /* expresion  */
  YYSYMBOL_termino = 59,                   /* termino  */
  YYSYMBOL_factor = 60,                    /* factor  */
  YYSYMBOL_invocacion = 61,                /* invocacion  */
  YYSYMBOL_op_comparacion = 62,            /* op_comparacion  */
  YYSYMBOL_op_logico = 63                  /* op_logico  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   128

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  32
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  32
/* YYNRULES -- Number of rules.  */
#define YYNRULES  58
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  111

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   286


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   338,   338,   339,   342,   346,   347,   351,   350,   356,
     355,   360,   363,   364,   367,   370,   371,   374,   375,   376,
     377,   378,   379,   383,   387,   394,   396,   401,   400,   407,
     406,   410,   415,   414,   420,   424,   423,   427,   430,   432,
     436,   438,   440,   443,   445,   447,   450,   452,   454,   456,
     460,   464,   465,   466,   467,   468,   469,   472,   473
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "ID", "CTE", "CADENA",
  "OP_ASIG", "OP_IGUAL", "OP_SUMA", "OP_RESTA", "OP_MUL", "OP_DIV",
  "OP_MENOR", "OP_MENOR_IGUAL", "OP_MAYOR", "OP_MAYOR_IGUAL",
  "OP_DISTINTO", "PUNTO_Y_COMA", "PAR_ABRE", "PAR_CIERRA", "LLAVE_ABRE",
  "LLAVE_CIERRA", "IF", "ELSE", "VAR", "LOOP", "UNTIL", "OR", "AND",
  "WRITE", "MAIN", "RETURN", "$accept", "programa", "entrada",
  "lista_funciones", "definicion_funcion", "$@1", "funcion_main", "$@2",
  "cuerpo", "declaraciones", "declaracion", "lista_sentencias",
  "sentencia", "asignacion", "retorno", "salida", "seleccion", "$@3",
  "fin_seleccion", "$@4", "iteracion", "@5", "bloque", "condiciones", "@6",
  "condicion", "expresion", "termino", "factor", "invocacion",
  "op_comparacion", "op_logico", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-89)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -89,    13,     0,   -89,    -1,    27,     0,   -89,   -89,    18,
      30,   -89,   -89,    43,   -89,   -89,    51,    51,   -89,   -89,
     -89,     2,    41,    67,    49,   -89,    93,    52,   -89,    19,
     -89,   -89,   -89,   -89,   -89,   -89,    72,    52,    52,    62,
      74,    92,    33,    97,   -89,    52,     1,    76,   -89,   -89,
     -89,   -89,   -89,     6,    69,    62,    71,   -89,    88,   -89,
       3,    94,    99,    73,    75,    52,    52,   -89,    52,    52,
     -89,   -89,    86,    60,   -89,   -89,   -89,   -89,   -89,   -89,
     -89,   -89,   -89,   -89,    52,    32,   101,   104,   105,   -89,
      76,    76,   -89,   -89,   -89,    92,    62,    34,   -89,    62,
     -89,   -89,    70,   -89,    89,   -89,   -89,   106,    92,   -89,
     -89
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       4,     0,     0,     1,     0,     0,     0,     6,     3,     0,
       0,     5,     2,     0,     9,     7,     0,     0,    13,    10,
       8,     0,     0,     0,     0,    32,     0,     0,    12,     0,
      16,    17,    21,    20,    18,    19,     0,     0,     0,     0,
       0,     0,     0,    46,    47,     0,     0,    42,    45,    48,
      11,    15,    22,     0,     0,     0,     0,    37,     0,    14,
       0,     0,     0,     0,     0,     0,     0,    24,     0,     0,
      23,    50,     0,     0,    27,    58,    57,    35,    55,    51,
      52,    53,    54,    56,     0,     0,     0,     0,     0,    49,
      40,    41,    43,    44,    39,     0,     0,    38,    34,     0,
      26,    25,    31,    36,     0,    29,    28,     0,     0,    33,
      30
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -89,   -89,   -89,   -89,   118,   -89,   119,   -89,   109,   -89,
     -89,    68,   -25,   -89,   -89,   -89,   -89,   -89,   -89,   -89,
     -89,   -89,   -88,   -53,   -89,    31,   -26,    44,    38,   -21,
     -89,   -89
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     2,     6,     7,    17,     8,    16,    19,    21,
      28,    29,    30,    31,    32,    33,    34,    95,   106,   108,
      35,    41,    61,    56,    96,    57,    58,    47,    48,    49,
      84,    77
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      36,    46,    72,     4,    51,    22,    22,   102,    36,    65,
      66,    53,    54,     3,    65,    66,    63,     9,    67,    64,
     110,    13,    22,    70,    23,    23,    24,    25,    25,    73,
       5,    26,    26,    27,    27,    22,    43,    44,    62,    36,
      50,    23,    65,    66,    25,    10,   104,    37,    26,    14,
      27,    45,    40,    98,    23,    43,    44,    25,    97,    38,
      51,    26,    15,    27,    36,    43,    44,    78,    65,    66,
      45,    18,    79,    80,    81,    82,    83,    65,    66,    89,
      55,    65,    66,    65,    66,    39,    68,    69,    71,    52,
      74,    59,    88,   105,    89,    78,    65,    66,    75,    76,
      79,    80,    81,    82,    83,    94,    92,    93,   107,    90,
      91,    42,    60,    75,    76,    38,    75,    76,    87,    99,
      86,   100,   101,   109,    11,    12,    20,   103,    85
};

static const yytype_int8 yycheck[] =
{
      21,    27,    55,     3,    29,     3,     3,    95,    29,     8,
       9,    37,    38,     0,     8,     9,    42,    18,    17,    45,
     108,     3,     3,    17,    22,    22,    24,    25,    25,    55,
      30,    29,    29,    31,    31,     3,     3,     4,     5,    60,
      21,    22,     8,     9,    25,    18,    99,     6,    29,    19,
      31,    18,     3,    21,    22,     3,     4,    25,    84,    18,
      85,    29,    19,    31,    85,     3,     4,     7,     8,     9,
      18,    20,    12,    13,    14,    15,    16,     8,     9,    19,
      18,     8,     9,     8,     9,    18,    10,    11,    19,    17,
      19,    17,    19,    23,    19,     7,     8,     9,    27,    28,
      12,    13,    14,    15,    16,    19,    68,    69,    19,    65,
      66,    18,    20,    27,    28,    18,    27,    28,    19,    18,
      26,    17,    17,    17,     6,     6,    17,    96,    60
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    33,    34,     0,     3,    30,    35,    36,    38,    18,
      18,    36,    38,     3,    19,    19,    39,    37,    20,    40,
      40,    41,     3,    22,    24,    25,    29,    31,    42,    43,
      44,    45,    46,    47,    48,    52,    61,     6,    18,    18,
       3,    53,    18,     3,     4,    18,    58,    59,    60,    61,
      21,    44,    17,    58,    58,    18,    55,    57,    58,    17,
      20,    54,     5,    58,    58,     8,     9,    17,    10,    11,
      17,    19,    55,    58,    19,    27,    28,    63,     7,    12,
      13,    14,    15,    16,    62,    43,    26,    19,    19,    19,
      59,    59,    60,    60,    19,    49,    56,    58,    21,    18,
      17,    17,    54,    57,    55,    23,    50,    19,    51,    17,
      54
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    32,    33,    33,    34,    35,    35,    37,    36,    39,
      38,    40,    41,    41,    42,    43,    43,    44,    44,    44,
      44,    44,    44,    45,    46,    47,    47,    49,    48,    51,
      50,    50,    53,    52,    54,    56,    55,    55,    57,    57,
      58,    58,    58,    59,    59,    59,    60,    60,    60,    60,
      61,    62,    62,    62,    62,    62,    62,    63,    63
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     2,     0,     2,     1,     0,     6,     0,
       5,     4,     2,     0,     3,     2,     1,     1,     1,     1,
       1,     1,     2,     4,     3,     5,     5,     0,     7,     0,
       3,     0,     0,     8,     3,     0,     4,     1,     3,     3,
       3,     3,     1,     3,     3,     1,     1,     1,     1,     3,
       4,     1,     1,     1,     1,     1,     1,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 4: /* entrada: %empty  */
#line 343 "src/parser/parser.y"
                        { abrir_programa(); }
#line 1774 "src/parser/parser.tab.c"
    break;

  case 7: /* $@1: %empty  */
#line 351 "src/parser/parser.y"
                        { registrar_funcion((yyvsp[-3].indice_ts), (yyvsp[-1].indice_ts)); }
#line 1780 "src/parser/parser.tab.c"
    break;

  case 9: /* $@2: %empty  */
#line 356 "src/parser/parser.y"
                        { comenzar_main(); }
#line 1786 "src/parser/parser.tab.c"
    break;

  case 22: /* sentencia: invocacion PUNTO_Y_COMA  */
#line 380 "src/parser/parser.y"
                        { (void)(yyvsp[-1].ref); }
#line 1792 "src/parser/parser.tab.c"
    break;

  case 23: /* asignacion: ID OP_ASIG expresion PUNTO_Y_COMA  */
#line 384 "src/parser/parser.y"
                        { emitir(T_ASIG, ref_ts((yyvsp[-3].indice_ts)), (yyvsp[-1].ref), TIPO_NINGUNO); }
#line 1798 "src/parser/parser.tab.c"
    break;

  case 24: /* retorno: RETURN expresion PUNTO_Y_COMA  */
#line 388 "src/parser/parser.y"
                        {
                            emitir(T_ASIG, ref_ts(celda_retorno_de(funcion_actual)), (yyvsp[-1].ref), TIPO_NINGUNO);
                            emitir(T_RET, ref_vacia(), ref_vacia(), TIPO_NINGUNO);
                        }
#line 1807 "src/parser/parser.tab.c"
    break;

  case 25: /* salida: WRITE PAR_ABRE expresion PAR_CIERRA PUNTO_Y_COMA  */
#line 395 "src/parser/parser.y"
                        { emitir(T_WRITE, (yyvsp[-2].ref), ref_vacia(), TIPO_NINGUNO); }
#line 1813 "src/parser/parser.tab.c"
    break;

  case 26: /* salida: WRITE PAR_ABRE CADENA PAR_CIERRA PUNTO_Y_COMA  */
#line 397 "src/parser/parser.y"
                        { emitir(T_WRITE_STR, ref_ts((yyvsp[-2].indice_ts)), ref_vacia(), TIPO_NINGUNO); }
#line 1819 "src/parser/parser.tab.c"
    break;

  case 27: /* $@3: %empty  */
#line 401 "src/parser/parser.y"
                        { abrir_if((yyvsp[-1].cond)); }
#line 1825 "src/parser/parser.tab.c"
    break;

  case 29: /* $@4: %empty  */
#line 407 "src/parser/parser.y"
                        { abrir_else(); }
#line 1831 "src/parser/parser.tab.c"
    break;

  case 30: /* fin_seleccion: ELSE $@4 bloque  */
#line 409 "src/parser/parser.y"
                        { cerrar_if(); }
#line 1837 "src/parser/parser.tab.c"
    break;

  case 31: /* fin_seleccion: %empty  */
#line 411 "src/parser/parser.y"
                        { cerrar_if(); }
#line 1843 "src/parser/parser.tab.c"
    break;

  case 32: /* @5: %empty  */
#line 415 "src/parser/parser.y"
                        { (yyval.indice) = proximo(); }
#line 1849 "src/parser/parser.tab.c"
    break;

  case 33: /* iteracion: LOOP @5 bloque UNTIL PAR_ABRE condiciones PAR_CIERRA PUNTO_Y_COMA  */
#line 417 "src/parser/parser.y"
                        { repetir_si_falso((yyvsp[-2].cond), (yyvsp[-6].indice)); }
#line 1855 "src/parser/parser.tab.c"
    break;

  case 35: /* @6: %empty  */
#line 424 "src/parser/parser.y"
                        { (yyval.cond) = preparar_logico((yyvsp[-1].cond), (yyvsp[0].operador)); }
#line 1861 "src/parser/parser.tab.c"
    break;

  case 36: /* condiciones: condiciones op_logico @6 condicion  */
#line 426 "src/parser/parser.y"
                        { (yyval.cond) = combinar_logico((yyvsp[-1].cond), (yyvsp[-2].operador), (yyvsp[0].cond)); }
#line 1867 "src/parser/parser.tab.c"
    break;

  case 38: /* condicion: expresion op_comparacion expresion  */
#line 431 "src/parser/parser.y"
                        { (yyval.cond) = condicion_simple((yyvsp[-1].operador), (yyvsp[-2].ref), (yyvsp[0].ref)); }
#line 1873 "src/parser/parser.tab.c"
    break;

  case 39: /* condicion: PAR_ABRE condiciones PAR_CIERRA  */
#line 433 "src/parser/parser.y"
                        { (yyval.cond) = (yyvsp[-1].cond); }
#line 1879 "src/parser/parser.tab.c"
    break;

  case 40: /* expresion: expresion OP_SUMA termino  */
#line 437 "src/parser/parser.y"
                        { (yyval.ref) = ref_ter(emitir(T_SUMA, (yyvsp[-2].ref), (yyvsp[0].ref), TIPO_RACIONAL)); }
#line 1885 "src/parser/parser.tab.c"
    break;

  case 41: /* expresion: expresion OP_RESTA termino  */
#line 439 "src/parser/parser.y"
                        { (yyval.ref) = ref_ter(emitir(T_RESTA, (yyvsp[-2].ref), (yyvsp[0].ref), TIPO_RACIONAL)); }
#line 1891 "src/parser/parser.tab.c"
    break;

  case 43: /* termino: termino OP_MUL factor  */
#line 444 "src/parser/parser.y"
                        { (yyval.ref) = ref_ter(emitir(T_MUL, (yyvsp[-2].ref), (yyvsp[0].ref), TIPO_RACIONAL)); }
#line 1897 "src/parser/parser.tab.c"
    break;

  case 44: /* termino: termino OP_DIV factor  */
#line 446 "src/parser/parser.y"
                        { (yyval.ref) = ref_ter(emitir(T_DIV, (yyvsp[-2].ref), (yyvsp[0].ref), TIPO_RACIONAL)); }
#line 1903 "src/parser/parser.tab.c"
    break;

  case 46: /* factor: ID  */
#line 451 "src/parser/parser.y"
                        { (yyval.ref) = ref_ts((yyvsp[0].indice_ts)); }
#line 1909 "src/parser/parser.tab.c"
    break;

  case 47: /* factor: CTE  */
#line 453 "src/parser/parser.y"
                        { (yyval.ref) = ref_ts((yyvsp[0].indice_ts)); }
#line 1915 "src/parser/parser.tab.c"
    break;

  case 48: /* factor: invocacion  */
#line 455 "src/parser/parser.y"
                        { (yyval.ref) = (yyvsp[0].ref); }
#line 1921 "src/parser/parser.tab.c"
    break;

  case 49: /* factor: PAR_ABRE expresion PAR_CIERRA  */
#line 457 "src/parser/parser.y"
                        { (yyval.ref) = (yyvsp[-1].ref); }
#line 1927 "src/parser/parser.tab.c"
    break;

  case 50: /* invocacion: ID PAR_ABRE expresion PAR_CIERRA  */
#line 461 "src/parser/parser.y"
                        { (yyval.ref) = emitir_llamada((yyvsp[-3].indice_ts), (yyvsp[-1].ref)); }
#line 1933 "src/parser/parser.tab.c"
    break;

  case 51: /* op_comparacion: OP_MENOR  */
#line 464 "src/parser/parser.y"
                                        { (yyval.operador) = T_MENOR; }
#line 1939 "src/parser/parser.tab.c"
    break;

  case 52: /* op_comparacion: OP_MENOR_IGUAL  */
#line 465 "src/parser/parser.y"
                                        { (yyval.operador) = T_MENOR_IGUAL; }
#line 1945 "src/parser/parser.tab.c"
    break;

  case 53: /* op_comparacion: OP_MAYOR  */
#line 466 "src/parser/parser.y"
                                        { (yyval.operador) = T_MAYOR; }
#line 1951 "src/parser/parser.tab.c"
    break;

  case 54: /* op_comparacion: OP_MAYOR_IGUAL  */
#line 467 "src/parser/parser.y"
                                        { (yyval.operador) = T_MAYOR_IGUAL; }
#line 1957 "src/parser/parser.tab.c"
    break;

  case 55: /* op_comparacion: OP_IGUAL  */
#line 468 "src/parser/parser.y"
                                        { (yyval.operador) = T_IGUAL; }
#line 1963 "src/parser/parser.tab.c"
    break;

  case 56: /* op_comparacion: OP_DISTINTO  */
#line 469 "src/parser/parser.y"
                                        { (yyval.operador) = T_DISTINTO; }
#line 1969 "src/parser/parser.tab.c"
    break;

  case 57: /* op_logico: AND  */
#line 472 "src/parser/parser.y"
                                        { (yyval.operador) = 1; }
#line 1975 "src/parser/parser.tab.c"
    break;

  case 58: /* op_logico: OR  */
#line 473 "src/parser/parser.y"
                                        { (yyval.operador) = 0; }
#line 1981 "src/parser/parser.tab.c"
    break;


#line 1985 "src/parser/parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 476 "src/parser/parser.y"


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: %s <archivo.rg>\n", argv[0]);
        return 1;
    }

    fuente = fopen(argv[1], "r");
    if (!fuente) {
        printf("Error de apertura\n");
        return 1;
    }

    yydebug = 1;
    yyparse();
    fclose(fuente);

    if (!hay_errores)
        printf(" - Compilacion EXITOSA - \n");
    else
        printf(" - Compilacion completa con ERRORES - \n");

    mostrarTS("out/ts.txt");
    imprimir_tercetos("out/tercetos.txt");
    return hay_errores ? 1 : 0;
}
