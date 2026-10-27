#ifndef LEXICO_H
#define LEXICO_H

#include <stdio.h>

#define MAX_LEXEMA 256
#define MAX_TS 1000

typedef struct {
    char nombre[MAX_LEXEMA];
    char tipo[20];
    char valor[MAX_LEXEMA];
    int longitud;
} Simbolo;

/* Los abre el main del parser, antes de yyparse. */
extern FILE *fuente;
extern int linea_actual;
/* Línea del token anterior al lookahead. La acción corre con el
   lookahead ya leído, y para entonces linea_actual es la de ese token. */
extern int linea_anterior;
extern int hay_errores;

extern Simbolo ts[MAX_TS];
extern int ts_size;

int yylex(void);
void yyerror(const char *msg);

/* Devuelve el índice de la entrada, nueva o ya existente. */
int agregar_ts(const char *nombre, const char *tipo, const char *valor, int longitud);
void mostrarTS(const char *filepath);

#endif
