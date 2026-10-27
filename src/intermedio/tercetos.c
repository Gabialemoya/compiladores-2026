#include <stdio.h>
#include "tercetos.h"
#include "../lexico/lexico.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

Terceto tercetos[MAX_TERCETOS];
int     cant_tercetos = 0;

Referencia ref_vacia(void) {
    Referencia r = { REF_VACIO, 0 };
    return r;
}

Referencia ref_ts(int indice) {
    Referencia r = { REF_TS, indice };
    return r;
}

Referencia ref_ter(int indice) {
    Referencia r = { REF_TER, indice };
    return r;
}

Referencia ref_dir(int indice) {
    Referencia r = { REF_DIR, indice };
    return r;
}

int emitir(OperadorTerceto op, Referencia op1, Referencia op2, TipoResultado tipo) {
    if (cant_tercetos >= MAX_TERCETOS) {
        printf("Linea %d: Error: se supero el limite de tercetos (%d)\n", linea_actual, MAX_TERCETOS);
        hay_errores = 1;
        return -1;
    }
    tercetos[cant_tercetos].operador = op;
    tercetos[cant_tercetos].op1      = op1;
    tercetos[cant_tercetos].op2      = op2;
    tercetos[cant_tercetos].tipo     = tipo;
    tercetos[cant_tercetos].linea    = linea_actual;
    return cant_tercetos++;
}

int proximo(void) {
    return cant_tercetos;
}

void completar_op1(int indice, int destino) {
    if (indice >= 0 && indice < cant_tercetos) {
        tercetos[indice].op1 = ref_dir(destino);
    }
}

void completar_op2(int indice, int destino) {
    if (indice >= 0 && indice < cant_tercetos) {
        tercetos[indice].op2 = ref_dir(destino);
    }
}

static const char* nombre_operador(OperadorTerceto op) {
    switch (op) {
        case T_SUMA:        return "SUMA";
        case T_RESTA:       return "RESTA";
        case T_MUL:         return "MUL";
        case T_DIV:         return "DIV";
        case T_ASIG:        return "ASIG";
        case T_IGUAL:       return "IGUAL";
        case T_DISTINTO:    return "DISTINTO";
        case T_MENOR:       return "MENOR";
        case T_MENOR_IGUAL: return "MENOR_IGUAL";
        case T_MAYOR:       return "MAYOR";
        case T_MAYOR_IGUAL: return "MAYOR_IGUAL";
        case T_BI:          return "BI";
        case T_BF:          return "BF";
        case T_WRITE:       return "WRITE";
        case T_WRITE_STR:   return "WRITE_STR";
        case T_CALL:        return "CALL";
        case T_RET:         return "RET";
        default:            return "UNKNOWN";
    }
}

static const char* nombre_tipo(TipoResultado tipo) {
    switch (tipo) {
        case TIPO_RACIONAL: return "RACIONAL";
        case TIPO_ENTERO:   return "ENTERO";
        case TIPO_NINGUNO:
        default:            return "-";
    }
}

static void formatear_referencia(Referencia ref, char* buf, size_t tam) {
    switch (ref.clase) {
        case REF_VACIO:
            snprintf(buf, tam, "-");
            break;
        case REF_TS:
            if (ref.valor >= 0 && ref.valor < ts_size) {
                snprintf(buf, tam, "TS:%s", ts[ref.valor].nombre);
            } else {
                snprintf(buf, tam, "TS:?");
            }
            break;
        case REF_TER:
            snprintf(buf, tam, "TER:%d", ref.valor);
            break;
        case REF_DIR:
            snprintf(buf, tam, "DIR:%d", ref.valor);
            break;
        default:
            snprintf(buf, tam, "-");
            break;
    }
}

void imprimir_tercetos(const char* ruta) {
    FILE* out;
    int i;
    char s_op1[MAX_LEXEMA + 8];
    char s_op2[MAX_LEXEMA + 8];

#ifdef _WIN32
    (void)_mkdir("out");
#else
    (void)mkdir("out", 0755);
#endif

    out = fopen(ruta, "w");
    if (!out) {
        printf("Error al crear archivo de tercetos en %s.\n", ruta);
        return;
    }

    fprintf(out, "#     OPERADOR      OP1                  OP2                  TIPO       LINEA\n");
    for (i = 0; i < cant_tercetos; i++) {
        formatear_referencia(tercetos[i].op1, s_op1, sizeof(s_op1));
        formatear_referencia(tercetos[i].op2, s_op2, sizeof(s_op2));
        fprintf(out, "%-6d%-14s%-21s%-21s%-11s%d\n",
                i,
                nombre_operador(tercetos[i].operador),
                s_op1,
                s_op2,
                nombre_tipo(tercetos[i].tipo),
                tercetos[i].linea);
    }

    fclose(out);
}

