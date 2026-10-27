%{
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

%token OP_ASIG OP_IGUAL
%token OP_SUMA OP_RESTA OP_MUL OP_DIV
%token OP_MENOR OP_MENOR_IGUAL OP_MAYOR OP_MAYOR_IGUAL OP_DISTINTO
%token PUNTO_Y_COMA PAR_ABRE PAR_CIERRA LLAVE_ABRE LLAVE_CIERRA
%token IF ELSE VAR LOOP UNTIL OR AND WRITE MAIN RETURN

%type <ref> expresion termino factor invocacion

%start programa

%%

programa            : lista_funciones funcion_main
                    | funcion_main
                    ;

lista_funciones     : lista_funciones definicion_funcion
                    | definicion_funcion
                    ;

definicion_funcion  : ID PAR_ABRE ID PAR_CIERRA
                        { registrar_funcion($1, $3); }
                      cuerpo
                    ;

funcion_main        : MAIN PAR_ABRE PAR_CIERRA
                        { funcion_actual = "main"; }
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
                        { (void)$1; }
                    ;

asignacion          : ID OP_ASIG expresion PUNTO_Y_COMA
                        { emitir(T_ASIG, ref_ts($1), $3, TIPO_NINGUNO); }
                    ;

retorno             : RETURN expresion PUNTO_Y_COMA
                        {
                            emitir(T_ASIG, ref_ts(celda_retorno_de(funcion_actual)), $2, TIPO_NINGUNO);
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
                    ;

termino             : termino OP_MUL factor
                        { $$ = ref_ter(emitir(T_MUL, $1, $3, TIPO_RACIONAL)); }
                    | termino OP_DIV factor
                        { $$ = ref_ter(emitir(T_DIV, $1, $3, TIPO_RACIONAL)); }
                    | factor
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
                        { $$ = emitir_llamada($1, $3); }
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
