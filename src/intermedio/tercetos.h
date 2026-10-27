#ifndef TERCETOS_H
#define TERCETOS_H

#define MAX_TERCETOS 2000

typedef enum { REF_VACIO = 0, REF_TS, REF_TER, REF_DIR } ClaseRef;

typedef struct {
    ClaseRef clase;
    int      valor;
} Referencia;

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

Referencia ref_vacia(void);
Referencia ref_ts(int indice);
Referencia ref_ter(int indice);
Referencia ref_dir(int indice);
int emitir(OperadorTerceto op, Referencia op1, Referencia op2, TipoResultado tipo);
int proximo(void);
void completar_op1(int indice, int destino);
void completar_op2(int indice, int destino);
void imprimir_tercetos(const char *ruta);

#endif
