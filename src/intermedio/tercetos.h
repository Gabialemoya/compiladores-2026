#ifndef TERCETOS_H
#define TERCETOS_H

#define MAX_TERCETOS 2000

typedef enum { REF_VACIO = 0, REF_TS, REF_TER, REF_DIR } ClaseRef;

typedef struct {
    ClaseRef clase;
    int      valor;
} Referencia;

/* Saltos de una condición que todavía no tienen destino.
   Caer al terceto siguiente es verdadero. */
#define MAX_SALTOS_COND 32

typedef struct {
    int falso[MAX_SALTOS_COND];
    int nfalso;
    int verdadero[MAX_SALTOS_COND];
    int nverdadero;
} Cond;

typedef enum { TIPO_NINGUNO = 0, TIPO_RACIONAL, TIPO_ENTERO } TipoResultado;

typedef enum {
    T_SUMA = 1, T_RESTA, T_MUL, T_DIV,
    T_ASIG,
    T_IGUAL, T_DISTINTO, T_MENOR, T_MENOR_IGUAL, T_MAYOR, T_MAYOR_IGUAL,
    T_BI, T_BF,
    T_WRITE, T_WRITE_STR,
    T_CALL, T_RET
} OperadorTerceto;

typedef struct {
    OperadorTerceto operador;
    Referencia      op1;
    Referencia      op2;
    TipoResultado   tipo;
    int             linea;
} Terceto;

extern Terceto tercetos[MAX_TERCETOS];
extern int     cant_tercetos;

Referencia apilar(int ptr);
Referencia apilar_ti(int ti);
Referencia sin_operando(void);

int colocar(OperadorTerceto op, Referencia op1, Referencia op2, TipoResultado tipo);

int nro_terceto(void);
void completar(int t, int destino);

void apilar_nro(int nro);
int desapilar(void);

void imprimir_tercetos(const char *ruta);

#endif
