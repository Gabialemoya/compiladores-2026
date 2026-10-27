Spec — Código intermedio de RG (tercetos)
Depende de: `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md` · `specs/04-analizador-sintactico/spec.md`
Produce: `src/intermedio/` · las acciones semánticas de `parser.y` · `out/tercetos.txt`
---
1. Alcance
Traducir el programa fuente a una lista de tercetos. La lista es el insumo de la etapa de generación de assembler y el artefacto que pide la consigna general 12.
No se ejecuta nada. La acción de una regla anota la operación, no la resuelve. Si la regla de la suma calculara el resultado, el compilador estaría compilando y ejecutando a la vez.
1.1 Fuera de alcance
La traducción de cada terceto a assembler y las rutinas de aritmética racional.
Los controles semánticos E6, E7 y E9.
---
2. Plan de la etapa
La etapa se construye en pasos. Cada uno se implementa, se prueba y se registra en la bitácora antes de pasar al siguiente.
Paso	Qué agrega	Estado
1	Infraestructura: módulo de tercetos y ampliación del valor semántico	completado
2	Acciones de expresiones y asignación	completado
3	Acciones de `write` y `return`	completado
4	Acciones de invocación de funciones	en curso
5	Acciones de `if` y `if … else`	pendiente
6	Acciones de `loop … until`	pendiente
7	Condiciones compuestas con `and` y `or`	pendiente
8	Revisión del archivo completo contra los programas de prueba	pendiente
Las secciones 3 a 6 corresponden al paso 1. Las acciones por regla se agregan a partir del paso 2.
---
3. Decisiones de esta etapa
#	Decisión	Valor
CI1	Representación intermedia	Tercetos
CI2	Identidad del terceto	Su índice en la lista. Es a la vez su temporario y su destino de salto
CI3	Temporarios	Uno por terceto que produce valor. Dirección estática fija (F.5 punto 17)
CI4	Línea del fuente	Cada terceto la guarda. La exige F.2 punto 10
CI5	Saltos	Backpatching sobre índices de terceto. No existen etiquetas
CI6	Ubicación del módulo	`src/intermedio/`, separado de `parser.y`
---
4. Formato del terceto
Campo	Contenido
`operador`	Ver §5
`op1`	Referencia
`op2`	Referencia, o vacía en los unarios
`tipo`	`TIPO_RACIONAL`, `TIPO_ENTERO` o `TIPO_NINGUNO`
`linea`	Línea del programa fuente
El índice no es un campo: es la posición en el arreglo.
4.1 Referencias
Un operando no es un número suelto: lleva su clase, porque el generador de assembler emite código distinto según a qué apunte.
Clase	Apunta a
`REF_VACIO`	Nada. El `op2` de un unario
`REF_TS`	Entrada de la tabla de símbolos: variable, constante o cadena
`REF_TER`	Resultado de otro terceto, o sea su temporario
`REF_DIR`	Índice de terceto, como destino de un salto
4.2 Tipo del resultado
`TIPO_ENTERO` existe porque F.1 punto 5 define el resultado de toda comparación como el entero `0` o `−1`, no como un racional. Es el campo que le dice al generador de assembler qué rutina corresponde a cada operando.
---
5. Operadores
Grupo	Operadores	op1	op2	tipo
Aritméticos	`T_SUMA` `T_RESTA` `T_MUL` `T_DIV`	operando	operando	RACIONAL
Asignación	`T_ASIG`	destino	origen	NINGUNO
Comparación	`T_IGUAL` `T_DISTINTO` `T_MENOR` `T_MENOR_IGUAL` `T_MAYOR` `T_MAYOR_IGUAL`	operando	operando	ENTERO
Saltos	`T_BI`	`REF_DIR`	vacío	NINGUNO
	`T_BF`	condición	`REF_DIR`	NINGUNO
Salida	`T_WRITE`	expresión	vacío	NINGUNO
	`T_WRITE_STR`	`REF_TS` de la cadena	vacío	NINGUNO
Funciones	`T_CALL`	`REF_DIR` de la entrada	vacío	NINGUNO
	`T_RET`	vacío	vacío	NINGUNO
No hay operador de etiqueta: el destino de un salto es el índice del terceto.
---
6. El módulo `src/intermedio/`
6.1 Estructuras — `tercetos.h`
```c
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
```
6.2 Funciones
Firma	Qué hace
`Referencia ref_vacia(void)`	Construye una referencia vacía
`Referencia ref_ts(int indice)`	Referencia a la tabla de símbolos
`Referencia ref_ter(int indice)`	Referencia al resultado de un terceto
`Referencia ref_dir(int indice)`	Referencia a un índice de terceto
`int emitir(OperadorTerceto op, Referencia op1, Referencia op2, TipoResultado tipo)`	Agrega el terceto y devuelve su índice. La línea la toma de `linea_actual`
`int proximo(void)`	Índice que tendrá el próximo terceto, sin emitirlo
`void completar_op1(int indice, int destino)`	Rellena un destino pendiente en `op1`
`void completar_op2(int indice, int destino)`	Rellena un destino pendiente en `op2`
`void imprimir_tercetos(const char* ruta)`	Vuelca la lista al archivo
Son dos funciones de completado porque el destino va en lugares distintos: en `T_BI` está en `op1` y en `T_BF` en `op2`.
`emitir` controla el límite de `MAX_TERCETOS`: si se supera, informa, levanta `hay_errores` y devuelve `-1`.
6.3 Valor semántico ampliado
El `%union` de `parser.y` pasa a tener tres campos:
```
%code requires {
    #include "../intermedio/tercetos.h"
}

%union {
    int        indice_ts;   /* lo carga yylex para ID, CTE, CADENA */
    Referencia ref;         /* lo propagan las reglas              */
    int        operador;    /* op_comparacion y op_logico          */
}
```
El bloque `%code requires` es necesario: sin él, el tipo `Referencia` no se copia a `parser.tab.h` y el léxico no compila, porque ese header es el que incluye.
En el paso 1 no se declaran los `%type`. Declararlos sin escribir las acciones haría que Bison aplique su acción por defecto `$$ = $1` entre campos de tipos distintos, y eso no compila. Los `%type` entran en el paso 2, junto con las acciones.
`yylex` no cambia: sigue cargando solo `indice_ts`.
---
7. Formato del archivo de salida
`out/tercetos.txt`, una línea por terceto:
```
#     OPERADOR      OP1                  OP2                  TIPO       LINEA
0     BI            DIR:1                -                    -          1
1     ASIG          TS:a                 TS:1/2               -          3
2     SUMA          TS:a                 TS:b                 RACIONAL   5
3     WRITE         TER:2                -                    -          6
```
Las referencias a la tabla de símbolos se imprimen con el nombre del símbolo, no con el índice: el archivo se lee a mano durante la corrección y el nombre es lo que permite contrastarlo contra el programa fuente.
El archivo se escribe al terminar `yyparse`, haya o no errores.
---
8. Compilación
El módulo se suma a la línea de compilación:
```
bison -d --report=all -o src/parser/parser.tab.c src/parser/parser.y
gcc -Wall -Wextra -o compilador src/parser/parser.tab.c src/lexico/lexico.c src/intermedio/tercetos.c
```
---
9. Criterios de aceptación del paso 1
Bison genera el parser sin conflictos, igual que antes.
Compila sin warnings con `-Wall -Wextra`.
Los 7 programas de `tests/correctos/` siguen terminando en compilación exitosa.
Los programas con error siguen reportando lo mismo que antes.
Se crea `out/tercetos.txt` con la línea de encabezado y ninguna fila, porque todavía no hay acciones que emitan.
El último punto es el que confirma que la infraestructura quedó conectada: el archivo existe y está vacío.
---
10. Paso 2 — Acciones de expresiones y asignación
10.1 Declaración de tipos de no terminales
Los no terminales de expresión propagan una `Referencia`:
```
%type <ref> expresion termino factor
```
10.2 Reglas y acciones
Regla	Producción	Acción
19	`asignacion : ID OP_ASIG expresion PUNTO_Y_COMA`	`emitir(T_ASIG, ref_ts($1), $3, TIPO_NINGUNO);`
31	`expresion : expresion OP_SUMA termino`	`$$ = ref_ter(emitir(T_SUMA, $1, $3, TIPO_RACIONAL));`
32	`expresion : expresion OP_RESTA termino`	`$$ = ref_ter(emitir(T_RESTA, $1, $3, TIPO_RACIONAL));`
33	`expresion : termino`	`$$ = $1;`
34	`termino : termino OP_MUL factor`	`$$ = ref_ter(emitir(T_MUL, $1, $3, TIPO_RACIONAL));`
35	`termino : termino OP_DIV factor`	`$$ = ref_ter(emitir(T_DIV, $1, $3, TIPO_RACIONAL));`
36	`termino : factor`	`$$ = $1;`
37	`factor : ID`	`$$ = ref_ts($1);`
38	`factor : CTE`	`$$ = ref_ts($1);`
39	`factor : invocacion`	`$$ = ref_vacia();` (se completa en el paso 4)
40	`factor : PAR_ABRE expresion PAR_CIERRA`	`$$ = $2;`
En la regla 39 (`factor : invocacion`) se devuelve provisoriamente `ref_vacia()` para evitar que la acción por defecto de Bison intente asignar un no terminal aún sin tipo (`invocacion`, que corresponde al paso 4) sobre `factor` de tipo `Referencia`.
---
11. Paso 3 — Acciones de `write` y `return`
11.1 Contexto de función actual
Para que `return expresion;` pueda fijar el valor de retorno de la función actual (R11) manteniendo `T_RET` sin operandos (§5), `parser.y` mantiene el índice en la tabla de símbolos de la función que se está reconociendo (`funcion_actual`), capturado en el encabezado de `definicion_funcion`:
```
definicion_funcion : ID PAR_ABRE ID PAR_CIERRA { funcion_actual = $1; } cuerpo
```
11.2 Reglas y acciones
Regla	Producción	Acción
20	`retorno : RETURN expresion PUNTO_Y_COMA`	`emitir(T_ASIG, ref_ts(funcion_actual), $2, TIPO_NINGUNO); emitir(T_RET, ref_vacia(), ref_vacia(), TIPO_NINGUNO);`
21	`salida : WRITE PAR_ABRE expresion PAR_CIERRA PUNTO_Y_COMA`	`emitir(T_WRITE, $3, ref_vacia(), TIPO_NINGUNO);`
22	`salida : WRITE PAR_ABRE CADENA PAR_CIERRA PUNTO_Y_COMA`	`emitir(T_WRITE_STR, ref_ts($3), ref_vacia(), TIPO_NINGUNO);`
---
12. Paso 4 — Acciones de invocación de funciones
12.1 Salto inicial a `main` y registro de funciones
En la gramática de RG, las definiciones de funciones (`lista_funciones`) anteceden a `funcion_main`. Para que la ejecución comience siempre en `main` y salte por encima de las funciones de usuario (§7, terceto 0 `BI`):
- Antes de iniciar `yyparse`, se emite el terceto `0` con `T_BI` y destino pendiente (`salto_main`).
- Al comenzar `funcion_main` (`MAIN PAR_ABRE PAR_CIERRA`), se completa el `op1` de `salto_main` con `proximo()` mediante `completar_op1(salto_main, proximo())`.
- En el encabezado de cada `definicion_funcion` (`ID PAR_ABRE ID PAR_CIERRA`), además de `funcion_actual = $1`, se registran para esa función:
  - `fn_param[$1] = $3` (el índice en TS de su parámetro formal).
  - `fn_inicio[$1] = proximo()` (el índice del primer terceto de su cuerpo).
12.2 Declaración de tipo y acciones
`invocacion` pasa a tener tipo `<ref>`:
```
%type <ref> expresion termino factor invocacion
```
Regla	Producción	Acción
5	`definicion_funcion : ID PAR_ABRE ID PAR_CIERRA { ... } cuerpo`	Guarda `funcion_actual = $1`, `fn_param[$1] = $3` y `fn_inicio[$1] = proximo()` antes de `cuerpo`
6	`funcion_main : MAIN PAR_ABRE PAR_CIERRA { ... } cuerpo`	`completar_op1(salto_main, proximo());` antes de `cuerpo`
18	`sentencia : invocacion PUNTO_Y_COMA`	Sin acción adicional (la invocación ya emitió sus tercetos)
39	`factor : invocacion`	`$$ = $1;`
41	`invocacion : ID PAR_ABRE expresion PAR_CIERRA`	`emitir(T_ASIG, ref_ts(fn_param[$1]), $3, TIPO_NINGUNO); emitir(T_CALL, ref_dir(fn_inicio[$1]), ref_vacia(), TIPO_NINGUNO); $$ = ref_ts($1);`
