#include "assembler.h"
#include "../intermedio/tercetos.h"
#include "../lexico/lexico.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/stat.h>
#endif

static const char *scratch[] = {
    "sa_lo", "sa_hi", "sb_lo", "sb_hi", "sc_lo", "sc_hi", "sd_lo", "sd_hi",
    "g_lo", "g_hi", "dg_lo", "dg_hi", "bg_lo", "bg_hi",
    "p1_lo", "p1_hi", "p2_lo", "p2_hi", "den_lo", "den_hi",
    "nr_lo", "nr_hi", "nd_lo", "nd_hi",
    "quot_lo", "quot_hi", "rem_lo", "rem_hi",
    "dvd_lo", "dvd_hi", "dvs_lo", "dvs_hi",
    "dest_ptr", "linea_op", "aviso_op", "signo_op",
    "a1_lo", "a1_hi", "c1_lo", "c1_hi", "b1_lo", "b1_hi", "d1_lo", "d1_hi",
    "ma_lo", "ma_hi", "mb_lo", "mb_hi",
    "ta_lo", "ta_hi", "tb_lo", "tb_hi", "pr_lo", "pr_hi",
    "p_lo", "p_hi", "q_lo", "q_hi",
    NULL
};

static void asegurar_out(void) {
#ifdef _WIN32
    (void)_mkdir("out");
#else
    (void)mkdir("out", 0755);
#endif
}

static int solo_letras(const char *s) {
    if (*s == '\0')
        return 0;
    for (; *s; s++) {
        if ((*s < 'a' || *s > 'z') && (*s < 'A' || *s > 'Z'))
            return 0;
    }
    return 1;
}

static int parse_par(const char *s, long *num, long *den) {
    char *fin;
    long n;
    long d;

    if (s[0] == '\0')
        return 0;
    n = strtol(s, &fin, 10);
    if (fin == s || *fin != '/')
        return 0;
    d = strtol(fin + 1, &fin, 10);
    if (fin == s || *fin != '\0')
        return 0;
    *num = n;
    *den = d;
    return 1;
}

static void etiqueta_ts(char *buf, size_t n, int indice) {
    const char *nombre;
    long num;
    long den;

    if (indice < 0 || indice >= ts_size) {
        snprintf(buf, n, "ts_mal");
        return;
    }

    nombre = ts[indice].nombre;
    if (strcmp(ts[indice].tipo, "cadena") == 0) {
        snprintf(buf, n, "str%d", indice);
        return;
    }
    if (nombre[0] == '@') {
        snprintf(buf, n, "%s", nombre + 1);
        return;
    }
    if (solo_letras(nombre)) {
        snprintf(buf, n, "%s", nombre);
        return;
    }

    num = 0;
    den = 1;
    if (!parse_par(ts[indice].valor, &num, &den))
        parse_par(nombre, &num, &den);
    if (num < 0)
        snprintf(buf, n, "cte_m%ld_%ld", -num, den);
    else
        snprintf(buf, n, "cte_%ld_%ld", num, den);
}

static void dir_dato(char *buf, size_t n, Referencia ref) {
    if (ref.clase == REF_TER)
        snprintf(buf, n, "ter%d", ref.valor);
    else if (ref.clase == REF_TS)
        etiqueta_ts(buf, n, ref.valor);
    else if (ref.clase == REF_DIR)
        snprintf(buf, n, "t%d", ref.valor);
    else
        snprintf(buf, n, "0");
}

static const char *nombre_operador(OperadorTerceto op) {
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
    default:            return "?";
    }
}

static int codigo_comparacion(OperadorTerceto op) {
    switch (op) {
    case T_IGUAL:        return 0;
    case T_DISTINTO:     return 1;
    case T_MENOR:        return 2;
    case T_MENOR_IGUAL:  return 3;
    case T_MAYOR:        return 4;
    case T_MAYOR_IGUAL:  return 5;
    default:             return 0;
    }
}

static const char *rutina_aritmetica(OperadorTerceto op) {
    switch (op) {
    case T_SUMA:  return "suma_rac";
    case T_RESTA: return "resta_rac";
    case T_MUL:   return "mul_rac";
    case T_DIV:   return "div_rac";
    default:      return "suma_rac";
    }
}

static void emitir_cadena(FILE *out, const char *s) {
    for (; *s; s++) {
        if (*s == '"')
            fputs("\"\"", out);
        else
            fputc(*s, out);
    }
}

static void emitir_simbolo(FILE *out, int indice) {
    char etiq[MAX_LEXEMA + 16];
    long num = 0;
    long den = 1;

    etiqueta_ts(etiq, sizeof etiq, indice);
    if (strcmp(ts[indice].tipo, "cadena") == 0) {
        fprintf(out, "%s: db \"", etiq);
        emitir_cadena(out, ts[indice].valor);
        fprintf(out, "\", 0\n");
        return;
    }

    if (ts[indice].valor[0] != '\0')
        parse_par(ts[indice].valor, &num, &den);
    fprintf(out, "%s: dd %ld\n", etiq, num);
    fprintf(out, "dd %ld\n", den);
}

static void emitir_datos(FILE *out) {
    int i;

    fputs("section .data\n", out);
    for (i = 0; i < ts_size; i++)
        emitir_simbolo(out, i);
    for (i = 0; i < cant_tercetos; i++) {
        if (tercetos[i].tipo == TIPO_RACIONAL)
            fprintf(out, "ter%d: dd 0\ndd 1\n", i);
        else if (tercetos[i].tipo == TIPO_ENTERO)
            fprintf(out, "ter%d: dd 0\n", i);
    }
    for (i = 0; scratch[i] != NULL; i++)
        fprintf(out, "%s: dd 0\n", scratch[i]);
    fputs("fmt_rac: db \"%d/%d\", 10, 0\n", out);
    fputs("fmt_str: db \"%s\", 10, 0\n", out);
    fputs("fmt_a1: db \"Linea %d: Advertencia: perdida de precision\", 10, 0\n", out);
    fputs("fmt_e8: db \"Linea %d: Error: denominador cero\", 10, 0\n", out);
}

static void emitir_aritmetica(FILE *out, int i) {
    char op1[MAX_LEXEMA + 16];
    char op2[MAX_LEXEMA + 16];

    dir_dato(op1, sizeof op1, tercetos[i].op1);
    dir_dato(op2, sizeof op2, tercetos[i].op2);
    fprintf(out, "    push %d\n", tercetos[i].linea);
    fprintf(out, "    push ter%d\n", i);
    fprintf(out, "    push %s\n", op2);
    fprintf(out, "    push %s\n", op1);
    fprintf(out, "    call %s\n", rutina_aritmetica(tercetos[i].operador));
    fputs("    add esp, 16\n", out);
}

static void emitir_asignacion(FILE *out, int i) {
    char origen[MAX_LEXEMA + 16];
    char destino[MAX_LEXEMA + 16];

    dir_dato(destino, sizeof destino, tercetos[i].op1);
    dir_dato(origen, sizeof origen, tercetos[i].op2);
    fprintf(out, "    mov eax, [%s]\n", origen);
    fprintf(out, "    mov [%s], eax\n", destino);
    fprintf(out, "    mov eax, [%s+4]\n", origen);
    fprintf(out, "    mov [%s+4], eax\n", destino);
}

static void emitir_comparacion(FILE *out, int i) {
    char op1[MAX_LEXEMA + 16];
    char op2[MAX_LEXEMA + 16];

    dir_dato(op1, sizeof op1, tercetos[i].op1);
    dir_dato(op2, sizeof op2, tercetos[i].op2);
    fprintf(out, "    push %d\n", codigo_comparacion(tercetos[i].operador));
    fprintf(out, "    push ter%d\n", i);
    fprintf(out, "    push %s\n", op2);
    fprintf(out, "    push %s\n", op1);
    fputs("    call comparar\n", out);
    fputs("    add esp, 16\n", out);
}

static void emitir_salida(FILE *out, int i) {
    char op[MAX_LEXEMA + 16];

    dir_dato(op, sizeof op, tercetos[i].op1);
    if (tercetos[i].operador == T_WRITE_STR) {
        fprintf(out, "    push %s\n", op);
        fputs("    push fmt_str\n", out);
        fputs("    call _printf\n", out);
        fputs("    add esp, 8\n", out);
        return;
    }
    fprintf(out, "    push dword [%s+4]\n", op);
    fprintf(out, "    push dword [%s]\n", op);
    fputs("    push fmt_rac\n", out);
    fputs("    call _printf\n", out);
    fputs("    add esp, 12\n", out);
}

static void emitir_terceto(FILE *out, int i) {
    Terceto *t = &tercetos[i];

    fprintf(out, "; %d %s\n", i, nombre_operador(t->operador));
    fprintf(out, "t%d:\n", i);

    switch (t->operador) {
    case T_SUMA:
    case T_RESTA:
    case T_MUL:
    case T_DIV:
        emitir_aritmetica(out, i);
        break;
    case T_ASIG:
        emitir_asignacion(out, i);
        break;
    case T_IGUAL:
    case T_DISTINTO:
    case T_MENOR:
    case T_MENOR_IGUAL:
    case T_MAYOR:
    case T_MAYOR_IGUAL:
        emitir_comparacion(out, i);
        break;
    case T_BI:
        fprintf(out, "    jmp t%d\n", t->op1.valor);
        break;
    case T_BF:
        fprintf(out, "    cmp dword [ter%d], 0\n", t->op1.valor);
        fprintf(out, "    je t%d\n", t->op2.valor);
        break;
    case T_WRITE:
    case T_WRITE_STR:
        emitir_salida(out, i);
        break;
    case T_CALL:
        fprintf(out, "    call t%d\n", t->op1.valor);
        break;
    case T_RET:
        fputs("    ret\n", out);
        break;
    default:
        break;
    }
}

static FILE *abrir_runtime(void) {
    FILE *in;
    char ruta[512];
    char exe[MAX_PATH];
    char *barra;
    DWORD len;

    in = fopen("src/assembler/runtime.asm", "rb");
    if (in)
        return in;
    in = fopen("src\\assembler\\runtime.asm", "rb");
    if (in)
        return in;

    len = GetModuleFileNameA(NULL, exe, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
        return NULL;
    barra = strrchr(exe, '\\');
    if (!barra)
        barra = strrchr(exe, '/');
    if (!barra)
        return NULL;
    *barra = '\0';
    snprintf(ruta, sizeof ruta, "%s\\src\\assembler\\runtime.asm", exe);
    return fopen(ruta, "rb");
}

static int copiar_runtime(FILE *out) {
    FILE *in;
    char buf[4096];
    size_t n;

    in = abrir_runtime();
    if (!in)
        return 0;
    while ((n = fread(buf, 1, sizeof buf, in)) > 0)
        fwrite(buf, 1, n, out);
    fclose(in);
    return 1;
}

void genera_cod_asm(void) {
    FILE *out;
    int i;

    if (hay_errores)
        return;

    asegurar_out();
    out = fopen("out/programa.asm", "w");
    if (!out) {
        printf("Error al crear archivo assembler.\n");
        hay_errores = 1;
        return;
    }

    emitir_datos(out);
    fputs("\nsection .text\n", out);
    fputs("global _main\n", out);
    fputs("extern _printf\n", out);
    fputs("extern _exit\n\n", out);
    fputs("_main:\n", out);

    for (i = 0; i < cant_tercetos; i++)
        emitir_terceto(out, i);

    fprintf(out, "t%d:\n", cant_tercetos);
    fputs("    xor eax, eax\n", out);
    fputs("    ret\n\n", out);

    if (!copiar_runtime(out)) {
        printf("Error al crear archivo assembler.\n");
        hay_errores = 1;
    }

    fclose(out);
}
