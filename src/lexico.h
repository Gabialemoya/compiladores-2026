#ifndef LEXICO_H
#define LEXICO_H

#include <stdio.h>

extern FILE *fuente;
extern int linea_actual;
extern int hay_errores;
extern int token_actual;

int yylex(void);
void yyerror(const char* msg);
int agregar_ts(const char* nombre, const char* tipo, const char* valor, int longitud);
void mostrarTS(const char* filepath);
const char* token_to_string(int token);
int traducir_token(int interno);

#endif // LEXICO_H
