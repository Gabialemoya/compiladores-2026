%{
#include <stdio.h>
#include <stdlib.h>
#include "../lexico/lexico.h"

static int funcion_actual = -1;
static int salto_main     = -1;
static int fn_inicio[MAX_TS];
static int fn_param[MAX_TS];
%}

%define parse.error verbose
%define parse.trace

%code requires {
    #include "../intermedio/tercetos.h"
}

%union {
    int        indice_ts;   /* lo carga yylex para ID, CTE, CADENA */
    Referencia ref;         /* lo propagan las reglas              */
    int        operador;    /* op_comparacion y op_logico          */
}

%token <indice_ts> ID CTE CADENA
%type  <ref>       expresion termino factor invocacion

%token OP_ASIG OP_IGUAL
%token OP_SUMA OP_RESTA OP_MUL OP_DIV
%token OP_MENOR OP_MENOR_IGUAL OP_MAYOR OP_MAYOR_IGUAL OP_DISTINTO
%token PUNTO_Y_COMA PAR_ABRE PAR_CIERRA LLAVE_ABRE LLAVE_CIERRA
%token IF ELSE VAR LOOP UNTIL OR AND WRITE MAIN RETURN

%start programa

%%

programa            : lista_funciones funcion_main
                    | funcion_main
                    ;

lista_funciones     : lista_funciones definicion_funcion
                    | definicion_funcion
                    ;

definicion_funcion  : ID PAR_ABRE ID PAR_CIERRA
                        {
                            funcion_actual = $1;
                            fn_param[$1]   = $3;
                            fn_inicio[$1]  = proximo();
                        }
                      cuerpo
                        { funcion_actual = -1; }
                    ;

funcion_main        : MAIN PAR_ABRE PAR_CIERRA
                        { completar_op1(salto_main, proximo()); }
                      cuerpo
                    ;

cuerpo              : LLAVE_ABRE declaraciones lista_sentencias LLAVE_CIERRA
                    ;

declaraciones       : declaraciones declaracion
                    | %empty
                    ;

declaracion         : VAR ID PUNTO_Y_COMA
                    ;

lista_sentencias    : lista_sentencias sentencia
                    | sentencia
                    ;

sentencia           : asignacion
                    | seleccion
                    | iteracion
                    | salida
                    | retorno
                    | invocacion PUNTO_Y_COMA
                    ;

asignacion          : ID OP_ASIG expresion PUNTO_Y_COMA
                        { emitir(T_ASIG, ref_ts($1), $3, TIPO_NINGUNO); }
                    ;

retorno             : RETURN expresion PUNTO_Y_COMA
                        {
                            emitir(T_ASIG, ref_ts(funcion_actual), $2, TIPO_NINGUNO);
                            emitir(T_RET, ref_vacia(), ref_vacia(), TIPO_NINGUNO);
                        }
                    ;

salida              : WRITE PAR_ABRE expresion PAR_CIERRA PUNTO_Y_COMA
                        { emitir(T_WRITE, $3, ref_vacia(), TIPO_NINGUNO); }
                    | WRITE PAR_ABRE CADENA PAR_CIERRA PUNTO_Y_COMA
                        { emitir(T_WRITE_STR, ref_ts($3), ref_vacia(), TIPO_NINGUNO); }
                    ;

seleccion           : IF PAR_ABRE condiciones PAR_CIERRA bloque
                    | IF PAR_ABRE condiciones PAR_CIERRA bloque ELSE bloque
                    ;

iteracion           : LOOP bloque UNTIL PAR_ABRE condiciones PAR_CIERRA PUNTO_Y_COMA
                    ;

bloque              : LLAVE_ABRE lista_sentencias LLAVE_CIERRA
                    ;

condiciones         : condiciones op_logico condicion
                    | condicion
                    ;

condicion           : expresion op_comparacion expresion
                    | PAR_ABRE condiciones PAR_CIERRA
                    ;

expresion           : expresion OP_SUMA termino
                        { $$ = ref_ter(emitir(T_SUMA, $1, $3, TIPO_RACIONAL)); }
                    | expresion OP_RESTA termino
                        { $$ = ref_ter(emitir(T_RESTA, $1, $3, TIPO_RACIONAL)); }
                    | termino
                        { $$ = $1; }
                    ;

termino             : termino OP_MUL factor
                        { $$ = ref_ter(emitir(T_MUL, $1, $3, TIPO_RACIONAL)); }
                    | termino OP_DIV factor
                        { $$ = ref_ter(emitir(T_DIV, $1, $3, TIPO_RACIONAL)); }
                    | factor
                        { $$ = $1; }
                    ;

factor              : ID
                        { $$ = ref_ts($1); }
                    | CTE
                        { $$ = ref_ts($1); }
                    | invocacion
                        { $$ = $1; }
                    | PAR_ABRE expresion PAR_CIERRA
                        { $$ = $2; }
                    ;

invocacion          : ID PAR_ABRE expresion PAR_CIERRA
                        {
                            emitir(T_ASIG, ref_ts(fn_param[$1]), $3, TIPO_NINGUNO);
                            emitir(T_CALL, ref_dir(fn_inicio[$1]), ref_vacia(), TIPO_NINGUNO);
                            $$ = ref_ts($1);
                        }
                    ;

op_comparacion      : OP_MENOR
                    | OP_MENOR_IGUAL
                    | OP_MAYOR
                    | OP_MAYOR_IGUAL
                    | OP_IGUAL
                    | OP_DISTINTO
                    ;

op_logico           : AND
                    | OR
                    ;

%%

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
    salto_main = emitir(T_BI, ref_vacia(), ref_vacia(), TIPO_NINGUNO);
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
