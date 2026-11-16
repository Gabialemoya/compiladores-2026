#include "tercetos.h"
#include "../lexico/lexico.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

Terceto tercetos[MAX_TERCETOS];
int cant_tercetos = 0;

#define MAX_PILA_NRO 64
static int pila_nro[MAX_PILA_NRO];
static int tope_pila_nro = 0;

static Referencia ref_nueva(ClaseRef clase, int valor) {
    Referencia r;
    r.clase = clase;
    r.valor = valor;
    return r;
}

Referencia apilar(int ptr) {
    return ref_nueva(REF_TS, ptr);
}

Referencia apilar_ti(int ti) {
    return ref_nueva(REF_TER, ti);
}

Referencia sin_operando(void) {
    return ref_nueva(REF_VACIO, 0);
}

static int crear(OperadorTerceto op, Referencia op1, Referencia op2, TipoResultado tipo) {
    Terceto *t;
    int linea = linea_anterior > 0 ? linea_anterior : linea_actual;

    if (cant_tercetos >= MAX_TERCETOS) {
        printf("Linea %d: Error: lista de tercetos llena\n", linea);
        hay_errores = 1;
        return -1;
    }

    t = &tercetos[cant_tercetos];
    t->operador = op;
    t->op1 = op1;
    t->op2 = op2;
    t->tipo = tipo;
    t->linea = linea;
    return cant_tercetos++;
}

int colocar(OperadorTerceto op, Referencia op1, Referencia op2, TipoResultado tipo) {
    return crear(op, op1, op2, tipo);
}

int crear_terceto(OperadorTerceto op, Referencia op1, Referencia op2, TipoResultado tipo) {
    return crear(op, op1, op2, tipo);
}

int nro_terceto(void) {
    return cant_tercetos;
}

void completar(int t, int destino) {
    Referencia *hueco;

    if (t < 0 || t >= cant_tercetos)
        return;
    /* El ? de un BF está en op2. En BI y CALL está en op1. */
    if (tercetos[t].operador == T_BF)
        hueco = &tercetos[t].op2;
    else
        hueco = &tercetos[t].op1;
    hueco->clase = REF_DIR;
    hueco->valor = destino;
}

void apilar_nro(int nro) {
    if (tope_pila_nro >= MAX_PILA_NRO) {
        printf("Linea %d: Error: pila de saltos llena\n", linea_actual);
        hay_errores = 1;
        return;
    }
    pila_nro[tope_pila_nro++] = nro;
}

int desapilar(void) {
    if (tope_pila_nro <= 0)
        return -1;
    return pila_nro[--tope_pila_nro];
}

static const char *nombre_operador(OperadorTerceto op) {
    switch (op) {
    case T_SUMA:         return "SUMA";
    case T_RESTA:        return "RESTA";
    case T_MUL:          return "MUL";
    case T_DIV:          return "DIV";
    case T_ASIG:         return "ASIG";
    case T_IGUAL:        return "IGUAL";
    case T_DISTINTO:     return "DISTINTO";
    case T_MENOR:        return "MENOR";
    case T_MENOR_IGUAL:  return "MENOR_IGUAL";
    case T_MAYOR:        return "MAYOR";
    case T_MAYOR_IGUAL:  return "MAYOR_IGUAL";
    case T_BI:           return "BI";
    case T_BF:           return "BF";
    case T_WRITE:        return "WRITE";
    case T_WRITE_STR:    return "WRITE_STR";
    case T_CALL:         return "CALL";
    case T_RET:          return "RET";
    default:             return "?";
    }
}

static const char *nombre_tipo(TipoResultado tipo) {
    switch (tipo) {
    case TIPO_RACIONAL: return "RACIONAL";
    case TIPO_ENTERO:   return "ENTERO";
    case TIPO_NINGUNO:  return "-";
    default:            return "-";
    }
}

static void formatear_ref(char *buf, size_t n, Referencia ref) {
    switch (ref.clase) {
    case REF_TS:
        if (ref.valor >= 0 && ref.valor < ts_size)
            snprintf(buf, n, "TS:%s", ts[ref.valor].nombre);
        else
            snprintf(buf, n, "TS:?");
        break;
    case REF_TER:
        snprintf(buf, n, "TER:%d", ref.valor);
        break;
    case REF_DIR:
        snprintf(buf, n, "DIR:%d", ref.valor);
        break;
    default:
        snprintf(buf, n, "-");
        break;
    }
}

static void asegurar_out(void) {
#ifdef _WIN32
    (void)_mkdir("out");
#else
    (void)mkdir("out", 0755);
#endif
}

void imprimir_tercetos(const char *ruta) {
    FILE *out;
    int i;
    char op1[MAX_LEXEMA + 8];
    char op2[MAX_LEXEMA + 8];

    asegurar_out();
    out = fopen(ruta, "w");
    if (!out) {
        printf("Error al crear archivo de tercetos.\n");
        return;
    }

    fprintf(out, "%-6s%-14s%-21s%-21s%-11s%s\n",
            "#", "OPERADOR", "OP1", "OP2", "TIPO", "LINEA");

    for (i = 0; i < cant_tercetos; i++) {
        formatear_ref(op1, sizeof op1, tercetos[i].op1);
        formatear_ref(op2, sizeof op2, tercetos[i].op2);
        fprintf(out, "%-6d%-14s%-21s%-21s%-11s%d\n",
                i,
                nombre_operador(tercetos[i].operador),
                op1,
                op2,
                nombre_tipo(tercetos[i].tipo),
                tercetos[i].linea);
    }

    fclose(out);
}
