#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#if defined(__has_include)
#  if __has_include("parser.tab.h")
#    include "parser.tab.h"
#  else
#    include "../parser/parser.tab.h"
#  endif
#else
#  include "parser.tab.h"
#endif
#include "lexico.h"

/* Numeración interna del lenguaje (01-diseno §4). No son los códigos de Bison. */
enum {
    COD_ID = 10,
    COD_CTE = 20,
    COD_OP_ASIG = 30,
    COD_OP_IGUAL = 40,
    COD_OP_SUMA = 50,
    COD_OP_RESTA = 60,
    COD_OP_MUL = 70,
    COD_OP_DIV = 80,
    COD_OP_MENOR = 90,
    COD_OP_MENOR_IGUAL = 100,
    COD_OP_MAYOR = 110,
    COD_OP_MAYOR_IGUAL = 120,
    COD_OP_DISTINTO = 130,
    COD_PUNTO_Y_COMA = 140,
    COD_PAR_ABRE = 150,
    COD_PAR_CIERRA = 160,
    COD_LLAVE_ABRE = 170,
    COD_LLAVE_CIERRA = 180,
    COD_IF = 190,
    COD_ELSE = 200,
    COD_VAR = 210,
    COD_LOOP = 220,
    COD_UNTIL = 230,
    COD_OR = 240,
    COD_AND = 250,
    COD_WRITE = 270,
    COD_MAIN = 280,
    COD_RETURN = 290,
    COD_CADENA = 300
};

enum {
    COL_LETRA = 0, COL_DIGITO = 1, COL_IGUAL = 2, COL_SUMA = 3, COL_RESTA = 4,
    COL_MUL = 5, COL_DIV = 6, COL_MENOR = 7, COL_MAYOR = 8, COL_PYC = 9,
    COL_PAR_A = 10, COL_PAR_C = 11, COL_LLA_A = 12, COL_LLA_C = 13,
    COL_COMILLA = 14, COL_SPACE = 15, COL_TAB = 16, COL_EOL = 17
};

FILE *fuente = NULL;
int linea_actual = 1;
int hay_errores = 0;

Simbolo ts[MAX_TS];
int ts_size = 0;

static char lexema[MAX_LEXEMA];
static int pos_lexema = 0;
static int char_actual = 0;
static int token_actual = -1;
static FILE *out_tokens = NULL;
static int listado_cerrado = 0;

static void inicio_id(void);
static void continuar_id(void);
static void fin_id(void);
static void inicio_cte(void);
static void continuar_cte(void);
static void fin_cte(void);
static void inicio_cadena(void);
static void continuar_cadena(void);
static void fin_cadena(void);
static void err_accion(void);
static void nada(void);

static int get_evento(int c) {
    if (isalpha(c)) return COL_LETRA;
    if (isdigit(c)) return COL_DIGITO;
    if (c == '=') return COL_IGUAL;
    if (c == '+') return COL_SUMA;
    if (c == '-') return COL_RESTA;
    if (c == '*') return COL_MUL;
    if (c == '/') return COL_DIV;
    if (c == '<') return COL_MENOR;
    if (c == '>') return COL_MAYOR;
    if (c == ';') return COL_PYC;
    if (c == '(') return COL_PAR_A;
    if (c == ')') return COL_PAR_C;
    if (c == '{') return COL_LLA_A;
    if (c == '}') return COL_LLA_C;
    if (c == '"') return COL_COMILLA;
    if (c == ' ') return COL_SPACE;
    if (c == '\r') return COL_SPACE;
    if (c == '\t') return COL_TAB;
    if (c == '\n') return COL_EOL;
    return -1;
}

static const char *token_a_nombre(int token) {
    switch (token) {
        case COD_ID: return "ID";
        case COD_CTE: return "CTE";
        case COD_OP_ASIG: return "OP_ASIG";
        case COD_OP_IGUAL: return "OP_IGUAL";
        case COD_OP_SUMA: return "OP_SUMA";
        case COD_OP_RESTA: return "OP_RESTA";
        case COD_OP_MUL: return "OP_MUL";
        case COD_OP_DIV: return "OP_DIV";
        case COD_OP_MENOR: return "OP_MENOR";
        case COD_OP_MENOR_IGUAL: return "OP_MENOR_IGUAL";
        case COD_OP_MAYOR: return "OP_MAYOR";
        case COD_OP_MAYOR_IGUAL: return "OP_MAYOR_IGUAL";
        case COD_OP_DISTINTO: return "OP_DISTINTO";
        case COD_PUNTO_Y_COMA: return "PUNTO_Y_COMA";
        case COD_PAR_ABRE: return "PAR_ABRE";
        case COD_PAR_CIERRA: return "PAR_CIERRA";
        case COD_LLAVE_ABRE: return "LLAVE_ABRE";
        case COD_LLAVE_CIERRA: return "LLAVE_CIERRA";
        case COD_IF: return "IF";
        case COD_ELSE: return "ELSE";
        case COD_VAR: return "VAR";
        case COD_LOOP: return "LOOP";
        case COD_UNTIL: return "UNTIL";
        case COD_OR: return "OR";
        case COD_AND: return "AND";
        case COD_WRITE: return "WRITE";
        case COD_MAIN: return "MAIN";
        case COD_RETURN: return "RETURN";
        case COD_CADENA: return "CADENA";
        default: return "UNKNOWN";
    }
}

/* Único lugar donde conviven la numeración interna y la de Bison. */
static int traducir_token(int codigo_interno) {
    switch (codigo_interno) {
        case COD_ID: return ID;
        case COD_CTE: return CTE;
        case COD_OP_ASIG: return OP_ASIG;
        case COD_OP_IGUAL: return OP_IGUAL;
        case COD_OP_SUMA: return OP_SUMA;
        case COD_OP_RESTA: return OP_RESTA;
        case COD_OP_MUL: return OP_MUL;
        case COD_OP_DIV: return OP_DIV;
        case COD_OP_MENOR: return OP_MENOR;
        case COD_OP_MENOR_IGUAL: return OP_MENOR_IGUAL;
        case COD_OP_MAYOR: return OP_MAYOR;
        case COD_OP_MAYOR_IGUAL: return OP_MAYOR_IGUAL;
        case COD_OP_DISTINTO: return OP_DISTINTO;
        case COD_PUNTO_Y_COMA: return PUNTO_Y_COMA;
        case COD_PAR_ABRE: return PAR_ABRE;
        case COD_PAR_CIERRA: return PAR_CIERRA;
        case COD_LLAVE_ABRE: return LLAVE_ABRE;
        case COD_LLAVE_CIERRA: return LLAVE_CIERRA;
        case COD_IF: return IF;
        case COD_ELSE: return ELSE;
        case COD_VAR: return VAR;
        case COD_LOOP: return LOOP;
        case COD_UNTIL: return UNTIL;
        case COD_OR: return OR;
        case COD_AND: return AND;
        case COD_WRITE: return WRITE;
        case COD_MAIN: return MAIN;
        case COD_RETURN: return RETURN;
        case COD_CADENA: return CADENA;
        default: return 0;
    }
}

static void asegurar_out(void) {
#ifdef _WIN32
    (void)_mkdir("out");
#else
    (void)mkdir("out", 0755);
#endif
}

int agregar_ts(const char *nombre, const char *tipo, const char *valor, int longitud) {
    int i;
    for (i = 0; i < ts_size; i++) {
        if (strcmp(ts[i].nombre, nombre) == 0 && strcmp(ts[i].tipo, tipo) == 0)
            return i;
    }
    if (ts_size >= MAX_TS) {
        printf("Linea %d: Error: tabla de simbolos llena\n", linea_actual);
        hay_errores = 1;
        return -1;
    }
    strcpy(ts[ts_size].nombre, nombre);
    strcpy(ts[ts_size].tipo, tipo);
    if (valor) strcpy(ts[ts_size].valor, valor);
    else ts[ts_size].valor[0] = '\0';
    ts[ts_size].longitud = longitud;
    return ts_size++;
}

static void asegurar_listado(void) {
    if (out_tokens || listado_cerrado) return;
    asegurar_out();
    out_tokens = fopen("out/tokens.txt", "w");
    if (!out_tokens)
        printf("Error al crear archivo de tokens en out/tokens.txt.\n");
}

static void cerrar_listado(void) {
    if (!out_tokens) {
        listado_cerrado = 1;
        return;
    }
    fclose(out_tokens);
    out_tokens = NULL;
    listado_cerrado = 1;
}

static void agregar_listado(int token) {
    asegurar_listado();
    if (!out_tokens) return;
    fprintf(out_tokens, "%s\n", token_a_nombre(token));
    fflush(out_tokens);
}

static void devolver_caracter(int c) {
    if (c == EOF) return;
    ungetc(c, fuente);
    if (c == '\n') linea_actual--;
}

static int leer_entero(const char *desde, const char *hasta, long long *out) {
    int signo = 1;
    long long valor = 0;
    if (desde >= hasta) return 0;
    if (*desde == '-') {
        signo = -1;
        desde++;
    }
    if (desde >= hasta || !isdigit((unsigned char)*desde)) return 0;
    while (desde < hasta && isdigit((unsigned char)*desde)) {
        valor = valor * 10 + (*desde - '0');
        desde++;
    }
    if (desde != hasta) return 0;
    *out = signo * valor;
    return 1;
}

static long long mcd(long long a, long long b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        long long resto = a % b;
        a = b;
        b = resto;
    }
    return a == 0 ? 1 : a;
}

static void inicio_id(void) {
    pos_lexema = 0;
    lexema[pos_lexema++] = (char)char_actual;
}

static void continuar_id(void) {
    if (pos_lexema < MAX_LEXEMA - 1) lexema[pos_lexema++] = (char)char_actual;
}

static void fin_id(void) {
    lexema[pos_lexema] = '\0';
    if (strcmp(lexema, "if") == 0) token_actual = COD_IF;
    else if (strcmp(lexema, "else") == 0) token_actual = COD_ELSE;
    else if (strcmp(lexema, "loop") == 0) token_actual = COD_LOOP;
    else if (strcmp(lexema, "until") == 0) token_actual = COD_UNTIL;
    else if (strcmp(lexema, "var") == 0) token_actual = COD_VAR;
    else if (strcmp(lexema, "or") == 0) token_actual = COD_OR;
    else if (strcmp(lexema, "and") == 0) token_actual = COD_AND;
    else if (strcmp(lexema, "write") == 0) token_actual = COD_WRITE;
    else if (strcmp(lexema, "main") == 0) token_actual = COD_MAIN;
    else if (strcmp(lexema, "return") == 0) token_actual = COD_RETURN;
    else {
        token_actual = COD_ID;
        yylval.indice_ts = agregar_ts(lexema, "racional", "", pos_lexema);
    }
}

static void inicio_cte(void) {
    pos_lexema = 0;
    lexema[pos_lexema++] = (char)char_actual;
}

static void continuar_cte(void) {
    if (pos_lexema < MAX_LEXEMA - 1) lexema[pos_lexema++] = (char)char_actual;
}

static void fin_cte(void) {
    long long numerador = 0;
    long long denominador = 0;
    long long divisor;
    const char *barra;
    char normal[MAX_LEXEMA];
    int longitud;

    lexema[pos_lexema] = '\0';
    barra = strchr(lexema, '/');
    if (!barra
        || !leer_entero(lexema, barra, &numerador)
        || !leer_entero(barra + 1, lexema + pos_lexema, &denominador)) {
        printf("Linea %d: Error lexico E2: constante mal formada\n", linea_actual);
        hay_errores = 1;
        token_actual = -1;
        return;
    }
    if (denominador == 0) {
        printf("Linea %d: Error lexico E3: denominador cero\n", linea_actual);
        hay_errores = 1;
        token_actual = -1;
        return;
    }

    if (denominador < 0) {
        numerador = -numerador;
        denominador = -denominador;
    }
    divisor = mcd(numerador, denominador);
    numerador /= divisor;
    denominador /= divisor;

    snprintf(normal, sizeof normal, "%lld/%lld", numerador, denominador);
    strncpy(lexema, normal, MAX_LEXEMA - 1);
    lexema[MAX_LEXEMA - 1] = '\0';
    longitud = (int)strlen(lexema);
    pos_lexema = longitud;

    token_actual = COD_CTE;
    yylval.indice_ts = agregar_ts(lexema, "racional", lexema, longitud);
}

static void inicio_cadena(void) {
    pos_lexema = 0;
}

static void continuar_cadena(void) {
    if (pos_lexema < MAX_LEXEMA - 1) lexema[pos_lexema++] = (char)char_actual;
}

static void fin_cadena(void) {
    lexema[pos_lexema] = '\0';
    token_actual = COD_CADENA;
    yylval.indice_ts = agregar_ts(lexema, "cadena", lexema, pos_lexema);
}

static void err_accion(void) {}
static void nada(void) {}

static int nuevo_estado[23][18] = {
    /* 0 */ {3, 1, 4, 6, 7, 8, 9, 11, 14, 16, 17, 18, 19, 20, 21, 0, 0, 0},
    /* 1 */ {-2, 1, -2, -2, -2, -2, 2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2},
    /* 2 */ {-2, 22, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2},
    /* 3 */ {3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 4 */ {-1, -1, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 5 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 6 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 7 */ {-1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 8 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 9 */ {-1, -1, -1, -1, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 10 */ {10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 0},
    /* 11 */ {-1, -1, 12, -1, -1, -1, -1, -1, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 12 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 13 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 14 */ {-1, -1, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 15 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 16 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 17 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 18 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 19 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 20 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 21 */ {21, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -2, -2, -2},
    /* 22 */ {-1, 22, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
};

static int matriz_tokens[23][18] = {
    /* 0 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 1 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 2 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 3 */ {-1, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
    /* 4 */ {30, 30, -1, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},
    /* 5 */ {40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40},
    /* 6 */ {50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50},
    /* 7 */ {60, -1, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60},
    /* 8 */ {70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70},
    /* 9 */ {80, 80, 80, 80, 80, 80, -1, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80},
    /* 10 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* 11 */ {90, 90, -1, 90, 90, 90, 90, 90, -1, 90, 90, 90, 90, 90, 90, 90, 90, 90},
    /* 12 */ {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
    /* 13 */ {130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130},
    /* 14 */ {110, 110, -1, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110},
    /* 15 */ {120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120},
    /* 16 */ {140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140},
    /* 17 */ {150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150},
    /* 18 */ {160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160},
    /* 19 */ {170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170},
    /* 20 */ {180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180},
    /* 21 */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 300, -1, -1, -1},
    /* 22 */ {20, -1, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20}
};

static void (*proceso[23][18])(void) = {
    /* 0 */ {inicio_id, inicio_cte, nada, nada, inicio_cte, nada, nada, nada, nada, nada, nada, nada, nada, nada, inicio_cadena, nada, nada, nada},
    /* 1 */ {err_accion, continuar_cte, err_accion, err_accion, err_accion, err_accion, continuar_cte, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion},
    /* 2 */ {err_accion, continuar_cte, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion},
    /* 3 */ {continuar_id, fin_id, fin_id, fin_id, fin_id, fin_id, fin_id, fin_id, fin_id, fin_id, fin_id, fin_id, fin_id, fin_id, fin_id, fin_id, fin_id, fin_id},
    /* 4 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 5 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 6 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 7 */ {nada, continuar_cte, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 8 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 9 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 10 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 11 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 12 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 13 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 14 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 15 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 16 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 17 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 18 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 19 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 20 */ {nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada, nada},
    /* 21 */ {continuar_cadena, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, err_accion, fin_cadena, err_accion, err_accion, err_accion},
    /* 22 */ {fin_cte, continuar_cte, fin_cte, fin_cte, fin_cte, fin_cte, fin_cte, fin_cte, fin_cte, fin_cte, fin_cte, fin_cte, fin_cte, fin_cte, fin_cte, fin_cte, fin_cte, fin_cte}
};

static void informar_transicion_invalida(int estado) {
    if (estado == 1 || estado == 2)
        printf("Linea %d: Error lexico E2: constante mal formada\n", linea_actual);
    else if (estado == 21)
        printf("Linea %d: Error lexico E4: cadena mal formada\n", linea_actual);
    else
        printf("Linea %d: Error lexico: transicion invalida\n", linea_actual);
    hay_errores = 1;
}

int yylex(void) {
    int estado = 0;

    if (!fuente) return 0;

    asegurar_listado();

    while (1) {
        int c = fgetc(fuente);
        int columna;
        int nuevo;
        int hacer_unread;

        if (c == EOF) {
            if (estado == 0 || estado == 10) {
                cerrar_listado();
                return 0;
            }
            columna = COL_SPACE;
            char_actual = ' ';
        } else {
            char_actual = c;
            columna = get_evento(c);
            if (columna < 0) {
                printf("Linea %d: Error lexico E1: caracter fuera del alfabeto\n", linea_actual);
                hay_errores = 1;
                estado = 0;
                continue;
            }
        }

        if (c == '\n') linea_actual++;

        token_actual = matriz_tokens[estado][columna];
        (*proceso[estado][columna])();
        nuevo = nuevo_estado[estado][columna];

        if (nuevo == -2) {
            informar_transicion_invalida(estado);
            devolver_caracter(c);
            estado = 0;
            continue;
        }

        if (nuevo == -1) {
            hacer_unread = 1;
            if (estado == 21 && columna == COL_COMILLA) hacer_unread = 0;
            if (estado == 10 && columna == COL_EOL) hacer_unread = 0;
            if (c == EOF) hacer_unread = 0;

            if (hacer_unread) devolver_caracter(c);

            if (token_actual == -1) {
                estado = 0;
                continue;
            }

            agregar_listado(token_actual);
            return traducir_token(token_actual);
        }

        estado = nuevo;
    }
}

void yyerror(const char *msg) {
    (void)msg;
    printf("Linea %d: Error sintactico E5: sentencia mal formada\n", linea_actual);
    hay_errores = 1;
}

void mostrarTS(const char *filepath) {
    FILE *out;
    int i;

    asegurar_out();
    out = fopen(filepath, "w");
    if (!out) {
        printf("Error al crear archivo de tabla de simbolos.\n");
        return;
    }
    fprintf(out, "NOMBRE | TIPO | VALOR | LONGITUD\n");
    for (i = 0; i < ts_size; i++)
        fprintf(out, "%s | %s | %s | %d\n", ts[i].nombre, ts[i].tipo, ts[i].valor, ts[i].longitud);
    fclose(out);
}
