%{
#include "src/lexico.h"

int yyparse(void);

%}

%union { int indice_ts; }

%error-verbose

%token <indice_ts> ID CTE CADENA
%token OP_ASIG OP_IGUAL OP_SUMA OP_RESTA OP_MUL OP_DIV
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

definicion_funcion  : ID PAR_ABRE ID PAR_CIERRA cuerpo
                    ;

funcion_main        : MAIN PAR_ABRE PAR_CIERRA cuerpo
                    ;

cuerpo              : LLAVE_ABRE declaraciones lista_sentencias LLAVE_CIERRA
                    ;

declaraciones       : declaraciones declaracion
                    | /* vacio */
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
                    ;

retorno             : RETURN expresion PUNTO_Y_COMA
                    ;

salida              : WRITE PAR_ABRE expresion PAR_CIERRA PUNTO_Y_COMA
                    | WRITE PAR_ABRE CADENA PAR_CIERRA PUNTO_Y_COMA
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
                    | expresion OP_RESTA termino
                    | termino
                    ;

termino             : termino OP_MUL factor
                    | termino OP_DIV factor
                    | factor
                    ;

factor              : ID
                    | CTE
                    | invocacion
                    | PAR_ABRE expresion PAR_CIERRA
                    ;

invocacion          : ID PAR_ABRE expresion PAR_CIERRA
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <archivo.rg>\n", argv[0]);
        return 1;
    }
    
    fuente = fopen(argv[1], "r");
    if (!fuente) {
        printf("Error de apertura\n");
        return 1;
    }

    yyparse();
    
    fclose(fuente);

    mostrarTS("out/ts.txt");

    if (!hay_errores) {
        printf(" - Compilacion EXITOSA - \n");
    } else {
        printf(" - Compilacion completa con ERRORES - \n");
    }

    return 0;
}
