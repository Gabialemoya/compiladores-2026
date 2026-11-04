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
    int        indice;      /* inicio del cuerpo de un loop            */
    Cond       cond;        /* saltos de una condicion                  */
}

%token <indice_ts> ID CTE CADENA

%token OP_ASIG OP_IGUAL
%token OP_SUMA OP_RESTA OP_MUL OP_DIV
%token OP_MENOR OP_MENOR_IGUAL OP_MAYOR OP_MAYOR_IGUAL OP_DISTINTO
%token PUNTO_Y_COMA PAR_ABRE PAR_CIERRA LLAVE_ABRE LLAVE_CIERRA
%token IF ELSE VAR LOOP UNTIL OR AND WRITE MAIN RETURN

%type <ref> expresion termino factor invocacion
%type <cond> condiciones condicion
%type <operador> op_comparacion op_logico

%start programa

%%

programa            : entrada lista_funciones funcion_main
                    | entrada funcion_main
                    ;

entrada             : %empty
                        { abrir_programa(); }
                    ;

lista_funciones     : lista_funciones definicion_funcion
                    | definicion_funcion
                    ;

definicion_funcion  : ID PAR_ABRE ID PAR_CIERRA
                        { registrar_funcion($1, $3); }
                      cuerpo
                    ;

funcion_main        : MAIN PAR_ABRE PAR_CIERRA
                        { comenzar_main(); }
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

seleccion           : IF PAR_ABRE condiciones PAR_CIERRA
                        { abrir_if($3); }
                      bloque
                      fin_seleccion
                    ;

fin_seleccion       : ELSE
                        { abrir_else(); }
                      bloque
                        { cerrar_if(); }
                    | %empty
                        { cerrar_if(); }
                    ;

iteracion           : LOOP
                        { $<indice>$ = proximo(); }
                      bloque UNTIL PAR_ABRE condiciones PAR_CIERRA PUNTO_Y_COMA
                        { repetir_si_falso($6, $<indice>2); }
                    ;

bloque              : LLAVE_ABRE lista_sentencias LLAVE_CIERRA
                    ;

condiciones         : condiciones op_logico
                        { $<cond>$ = preparar_logico($1, $2); }
                      condicion
                        { $$ = combinar_logico($<cond>3, $2, $4); }
                    | condicion
                    ;

condicion           : expresion op_comparacion expresion
                        { $$ = condicion_simple($2, $1, $3); }
                    | PAR_ABRE condiciones PAR_CIERRA
                        { $$ = $2; }
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

op_comparacion      : OP_MENOR          { $$ = T_MENOR; }
                    | OP_MENOR_IGUAL    { $$ = T_MENOR_IGUAL; }
                    | OP_MAYOR          { $$ = T_MAYOR; }
                    | OP_MAYOR_IGUAL    { $$ = T_MAYOR_IGUAL; }
                    | OP_IGUAL          { $$ = T_IGUAL; }
                    | OP_DISTINTO       { $$ = T_DISTINTO; }
                    ;

op_logico           : AND               { $$ = 1; }
                    | OR                { $$ = 0; }
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
