#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexico.h"

#define MAX_LEXEMA 256
#define MAX_TS 1000

#ifdef MODO_PRUEBA
typedef union {
    int indice_ts;
} YYSTYPE;
YYSTYPE yylval;
#else
#include "parser.tab.h"
#endif

// Columnas
enum {
    COL_LETRA = 0, COL_DIGITO = 1, COL_IGUAL = 2, COL_SUMA = 3, COL_RESTA = 4,
    COL_MUL = 5, COL_DIV = 6, COL_MENOR = 7, COL_MAYOR = 8, COL_PYC = 9,
    COL_PAR_A = 10, COL_PAR_C = 11, COL_LLA_A = 12, COL_LLA_C = 13,
    COL_COMILLA = 14, COL_SPACE = 15, COL_TAB = 16, COL_EOL = 17
};

// Tokens (01-diseno §4)
enum Tokens {
    TK_ID = 10, TK_CTE = 20, TK_OP_ASIG = 30, TK_OP_IGUAL = 40, TK_OP_SUMA = 50, TK_OP_RESTA = 60,
    TK_OP_MUL = 70, TK_OP_DIV = 80, TK_OP_MENOR = 90, TK_OP_MENOR_IGUAL = 100, TK_OP_MAYOR = 110,
    TK_OP_MAYOR_IGUAL = 120, TK_OP_DISTINTO = 130, TK_PUNTO_Y_COMA = 140, TK_PAR_ABRE = 150,
    TK_PAR_CIERRA = 160, TK_LLAVE_ABRE = 170, TK_LLAVE_CIERRA = 180, TK_IF = 190, TK_ELSE = 200,
    TK_VAR = 210, TK_LOOP = 220, TK_UNTIL = 230, TK_OR = 240, TK_AND = 250, TK_WRITE = 270,
    TK_MAIN = 280, TK_RETURN = 290, TK_CADENA = 300
};

int linea_actual = 1;
char lexema[MAX_LEXEMA];
int pos_lexema = 0;
int hay_errores = 0;
int char_actual = 0;
FILE *fuente = NULL;
int token_actual = -1;

typedef struct {
    char nombre[MAX_LEXEMA];
    char tipo[20];
    char valor[MAX_LEXEMA]; 
    int longitud;
} Simbolo;

Simbolo ts[MAX_TS];
int ts_size = 0;

int get_evento(int c) {
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

const char* token_to_string(int token) {
    switch(token) {
        case TK_ID: return "ID";
        case TK_CTE: return "CTE";
        case TK_OP_ASIG: return "OP_ASIG";
        case TK_OP_IGUAL: return "OP_IGUAL";
        case TK_OP_SUMA: return "OP_SUMA";
        case TK_OP_RESTA: return "OP_RESTA";
        case TK_OP_MUL: return "OP_MUL";
        case TK_OP_DIV: return "OP_DIV";
        case TK_OP_MENOR: return "OP_MENOR";
        case TK_OP_MENOR_IGUAL: return "OP_MENOR_IGUAL";
        case TK_OP_MAYOR: return "OP_MAYOR";
        case TK_OP_MAYOR_IGUAL: return "OP_MAYOR_IGUAL";
        case TK_OP_DISTINTO: return "OP_DISTINTO";
        case TK_PUNTO_Y_COMA: return "PUNTO_Y_COMA";
        case TK_PAR_ABRE: return "PAR_ABRE";
        case TK_PAR_CIERRA: return "PAR_CIERRA";
        case TK_LLAVE_ABRE: return "LLAVE_ABRE";
        case TK_LLAVE_CIERRA: return "LLAVE_CIERRA";
        case TK_IF: return "IF";
        case TK_ELSE: return "ELSE";
        case TK_VAR: return "VAR";
        case TK_LOOP: return "LOOP";
        case TK_UNTIL: return "UNTIL";
        case TK_OR: return "OR";
        case TK_AND: return "AND";
        case TK_WRITE: return "WRITE";
        case TK_MAIN: return "MAIN";
        case TK_RETURN: return "RETURN";
        case TK_CADENA: return "CADENA";
        default: return "UNKNOWN";
    }
}

int traducir_token(int interno) {
#ifdef MODO_PRUEBA
    return interno;
#else
    switch(interno) {
        case TK_ID: return ID;
        case TK_CTE: return CTE;
        case TK_OP_ASIG: return OP_ASIG;
        case TK_OP_IGUAL: return OP_IGUAL;
        case TK_OP_SUMA: return OP_SUMA;
        case TK_OP_RESTA: return OP_RESTA;
        case TK_OP_MUL: return OP_MUL;
        case TK_OP_DIV: return OP_DIV;
        case TK_OP_MENOR: return OP_MENOR;
        case TK_OP_MENOR_IGUAL: return OP_MENOR_IGUAL;
        case TK_OP_MAYOR: return OP_MAYOR;
        case TK_OP_MAYOR_IGUAL: return OP_MAYOR_IGUAL;
        case TK_OP_DISTINTO: return OP_DISTINTO;
        case TK_PUNTO_Y_COMA: return PUNTO_Y_COMA;
        case TK_PAR_ABRE: return PAR_ABRE;
        case TK_PAR_CIERRA: return PAR_CIERRA;
        case TK_LLAVE_ABRE: return LLAVE_ABRE;
        case TK_LLAVE_CIERRA: return LLAVE_CIERRA;
        case TK_IF: return IF;
        case TK_ELSE: return ELSE;
        case TK_VAR: return VAR;
        case TK_LOOP: return LOOP;
        case TK_UNTIL: return UNTIL;
        case TK_OR: return OR;
        case TK_AND: return AND;
        case TK_WRITE: return WRITE;
        case TK_MAIN: return MAIN;
        case TK_RETURN: return RETURN;
        case TK_CADENA: return CADENA;
        default: return 0;
    }
#endif
}

void yyerror(const char* msg) {
    printf("Linea %d: Error sintactico E5: sentencia mal formada\n", linea_actual);
    hay_errores = 1;
}

int agregar_ts(const char* nombre, const char* tipo, const char* valor, int longitud) {
    for (int i = 0; i < ts_size; i++) {
        if (strcmp(ts[i].nombre, nombre) == 0 && strcmp(ts[i].tipo, tipo) == 0) {
            return i;
        }
    }
    strcpy(ts[ts_size].nombre, nombre);
    strcpy(ts[ts_size].tipo, tipo);
    if (valor) strcpy(ts[ts_size].valor, valor);
    else ts[ts_size].valor[0] = '\0';
    ts[ts_size].longitud = longitud;
    ts_size++;
    return ts_size - 1;
}

void inicio_id() { pos_lexema = 0; lexema[pos_lexema++] = char_actual; }
void continuar_id() { if (pos_lexema < MAX_LEXEMA-1) lexema[pos_lexema++] = char_actual; }
void fin_id() {
    lexema[pos_lexema] = '\0';
    if (strcmp(lexema, "if") == 0) token_actual = TK_IF;
    else if (strcmp(lexema, "else") == 0) token_actual = TK_ELSE;
    else if (strcmp(lexema, "loop") == 0) token_actual = TK_LOOP;
    else if (strcmp(lexema, "until") == 0) token_actual = TK_UNTIL;
    else if (strcmp(lexema, "var") == 0) token_actual = TK_VAR;
    else if (strcmp(lexema, "or") == 0) token_actual = TK_OR;
    else if (strcmp(lexema, "and") == 0) token_actual = TK_AND;
    else if (strcmp(lexema, "write") == 0) token_actual = TK_WRITE;
    else if (strcmp(lexema, "main") == 0) token_actual = TK_MAIN;
    else if (strcmp(lexema, "return") == 0) token_actual = TK_RETURN;
    else {
        token_actual = TK_ID;
        int indice = agregar_ts(lexema, "racional", "", pos_lexema);
        yylval.indice_ts = indice;
    }
}
void inicio_cte() { pos_lexema = 0; lexema[pos_lexema++] = char_actual; }
void continuar_cte() { if (pos_lexema < MAX_LEXEMA-1) lexema[pos_lexema++] = char_actual; }
void fin_cte() {
    lexema[pos_lexema] = '\0';
    char *slash = strchr(lexema, '/');
    int is_zero = 1;
    if (slash) {
        for (char *p = slash + 1; *p; p++) {
            if (*p != '0') {
                is_zero = 0;
                break;
            }
        }
    }
    if (slash && is_zero) {
        printf("Linea %d: Error lexico E3: denominador cero\n", linea_actual);
        hay_errores = 1;
        token_actual = -1;
    } else {
        token_actual = TK_CTE;
        char normalizado[MAX_LEXEMA];
        strcpy(normalizado, lexema);
        if (slash) {
            long num = atol(lexema);
            long den = atol(slash + 1);
            if (den < 0) {
                num = -num;
                den = -den;
            }
            long a = num < 0 ? -num : num;
            long b = den < 0 ? -den : den;
            while (b != 0) {
                long temp = b;
                b = a % b;
                a = temp;
            }
            long mcd = a;
            if (mcd > 1) {
                num /= mcd;
                den /= mcd;
            }
            sprintf(normalizado, "%ld/%ld", num, den);
        }
        int indice = agregar_ts(normalizado, "racional", normalizado, strlen(normalizado));
        yylval.indice_ts = indice;
    }
}
void inicio_cadena() { pos_lexema = 0; }
void continuar_cadena() { if (pos_lexema < MAX_LEXEMA-1) lexema[pos_lexema++] = char_actual; }
void fin_cadena() {
    lexema[pos_lexema] = '\0';
    token_actual = TK_CADENA;
    int indice = agregar_ts(lexema, "cadena", lexema, pos_lexema);
    yylval.indice_ts = indice;
}
void err_accion() {}
void nada() {}

int nuevo_estado[23][18] = {
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

int matriz_tokens[23][18] = {
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

void (*proceso[23][18])(void) = {
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

int yylex(void) {
    int estado = 0;
    while (1) {
        int c = fgetc(fuente);
        if (c == EOF) {
            if (estado == 0) return 0; 
            if (estado == 10) c = '\n';
            else c = ' ';
        }
        char_actual = c;
        int col = get_evento(char_actual);
        if (col == -1) {
            printf("Linea %d: Error lexico E1: caracter fuera del alfabeto\n", linea_actual);
            hay_errores = 1;
            estado = 0;
            continue;
        }

        if (char_actual == '\n') linea_actual++;

        int token_temp = matriz_tokens[estado][col];
        token_actual = token_temp;

        (*proceso[estado][col])();
        int nuevo = nuevo_estado[estado][col];

        if (nuevo == -2) {
            if (estado == 1 || estado == 2) {
                printf("Linea %d: Error lexico E2: constante mal formada\n", linea_actual);
            } else if (estado == 21) {
                printf("Linea %d: Error lexico E4: cadena mal formada\n", linea_actual);
            } else {
                printf("Linea %d: Error lexico: transicion invalida\n", linea_actual);
            }
            hay_errores = 1;
            
            if (char_actual != EOF) {
                ungetc(char_actual, fuente);
                if (char_actual == '\n') linea_actual--;
            }

            estado = 0;
            continue;
        }

        if (nuevo == -1) {
            int hacer_unread = 1;
            if (estado == 21 && col == COL_COMILLA) hacer_unread = 0;
            if (estado == 10 && col == COL_EOL) hacer_unread = 0;

            if (hacer_unread) {
                if (char_actual != EOF) { 
                    ungetc(char_actual, fuente);
                    if (char_actual == '\n') linea_actual--;
                }
            }

            if (token_actual != -1) {
                return traducir_token(token_actual);
            } else {
                estado = 0;
                continue;
            }
        }
        estado = nuevo;
    }
}

void mostrarTS(const char* filepath) {
    FILE* out = fopen(filepath, "w");
    if (!out) {
        printf("Error al crear archivo de tabla de simbolos.\n");
        return;
    }
    fprintf(out, "NOMBRE | TIPO | VALOR | LONGITUD\n");
    for(int i = 0; i < ts_size; i++) {
        fprintf(out, "%s | %s | %s | %d\n", ts[i].nombre, ts[i].tipo, ts[i].valor, ts[i].longitud);
    }
    fclose(out);
}

#ifdef MODO_PRUEBA
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

    FILE* out_tokens = fopen("out/tokens.txt", "w");
    if (!out_tokens) {
        printf("Error al crear archivo de tokens en out/tokens.txt.\n");
        fclose(fuente);
        return 1;
    }

    int tok;
    while ((tok = yylex()) != 0) {
        fprintf(out_tokens, "%s\n", token_to_string(token_actual));
        printf("%s\n", token_to_string(token_actual));
    }
    
    fclose(fuente);
    fclose(out_tokens);

    mostrarTS("out/ts.txt");

    if (!hay_errores) {
        printf(" - Compilacion AL EXITOSA - \n");
    } else {
        printf(" - Analisis Lexico completo con ERRORES - \n");
    }

    return 0;
}
#endif
