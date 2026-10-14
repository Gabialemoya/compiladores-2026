# Spec — Analizador sintáctico de RG

**Depende de:** `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md` · `specs/03-analizador-lexico/spec-etapa-2.md`
**Produce:** `src/parser/`

Integra el analizador léxico con Bison y valida que el programa fuente pertenezca al lenguaje RG.

Bison toma la gramática libre de contexto, en forma LALR, y genera el analizador. El archivo de entrada es `parser.y`.

---

## 1. Alcance e interfaz

### 1.1 Objetivo

Reconocer la estructura del programa fuente aplicando las reglas gramaticales de `01-diseno` §6. El parser pide tokens a `yylex` (spec-etapa-2) y reduce hasta el axioma. Si no llega al axioma, el programa no pertenece al lenguaje.

### 1.2 `main`, `yyparse` y `yylex`

`main` abre el fuente y llama a `yyparse`. `yyparse` pide cada token a `yylex` y comprueba si la secuencia es una construcción válida de la gramática. `yylex` tiene que llamarse así: es el nombre que `yyparse` invoca.

`yylex` devuelve el código del token. Ante fin de archivo devuelve `0`. Si el token es `ID`, `CTE` o `CADENA`, deja en `yylval` el índice de la tabla de símbolos. Las acciones leen ese valor.


| Retorno de `yyparse` | Significado                                                        |
| -------------------- | ------------------------------------------------------------------ |
| `0`                  | El léxico entregó el fin de archivo y el sintáctico aceptó         |
| `1`                  | Hubo un error. `yyerrok` restablece el analizador a su modo normal |


La estrategia de recuperación queda fuera de esta etapa (§1.4).

### 1.3 Entrada

- Los tokens que entrega `yylex`, uno por llamada, hasta recibir `0`.
- El valor semántico en `yylval` para `ID`, `CTE` y `CADENA`.



### 1.4 Salida

1. **Veredicto**: el programa pertenece o no al lenguaje.
2. **Errores sintácticos** reportados por `yyerror`, con la línea del fuente.
3. **Punto de enganche** para las acciones semánticas de etapas posteriores.



### 1.5 Fuera de alcance

Queda fuera de esta spec, y se especifica en etapas posteriores:

- El contenido de las acciones semánticas: qué emite cada regla, el formato del terceto y sus operadores.
- La estrategia de recuperación de errores.
- La traducción a assembler.

Los controles semánticos R1, R2 y R7 también quedan para después. Están definidos en `01-diseno` §11; el error de cada una, en §12:


| Regla | Definición                                          | Error |
| ----- | --------------------------------------------------- | ----- |
| R1    | Usar un `ID` no declarado es error semántico        | E6    |
| R2    | Declarar dos veces el mismo `ID` es error semántico | E7    |
| R7    | Llamada recursiva directa: error semántico          | E9    |


El reconocimiento de tokens ya está especificado en `specs/03-analizador-lexico/`.

Las acciones se escriben físicamente dentro de las reglas de §5, pero **su contenido** se define aparte. Esta spec deja el esqueleto; la etapa de código intermedio lo llena.

### 1.6 Responsabilidades

- Declarar los tokens de forma que Bison genere los códigos que `yylex` retorna.
- Declarar el valor semántico.
- Expresar las producciones de `01-diseno` §6 sin alterarlas.
- Proveer el `main` que invoca `yyparse`.
- Dejar el parser libre de conflictos, o con los conflictos documentados y justificados.

---



## 2. Decisiones propias de esta fase


| #   | Decisión                    | Valor                                                                                                                   |
| --- | --------------------------- | ----------------------------------------------------------------------------------------------------------------------- |
| S1  | Herramienta                 | Bison, en C. La consigna general 1 exige integración con YACC                                                           |
| S2  | Lenguaje de las acciones    | C. Consigna general 2                                                                                                   |
| S3  | Generador de léxico         | **Ninguno.** No se usa Flex: `yylex` es manual (spec-etapa-2, Y1)                                                       |
| S4  | Códigos de token            | Los asigna Bison y los publica como `#define` en `parser.tab.h`. `yylex` traduce con `traducir_token` (spec-etapa-2 §3) |
| S5  | Valor semántico             | `%union` con un entero: índice en la tabla de símbolos (spec-etapa-2, Y4)                                               |
| S6  | Precedencia y asociatividad | **No se declaran.** Quedan codificadas en la jerarquía `expresion` → `termino` → `factor`                               |
| S7  | Axioma                      | `programa`, declarado con `%start`                                                                                      |
| S8  | Mensajes de error           | Modo detallado de Bison, para que `yyerror` reciba qué se esperaba                                                      |


---



## 3. Estructura de `parser.y`

El archivo tiene cuatro secciones. Los delimitadores son los de Bison.


| Sección       | Delimitador                           | Contenido en RG                                                                            |
| ------------- | ------------------------------------- | ------------------------------------------------------------------------------------------ |
| Declaraciones | `%{` … `%}`                           | Includes del léxico, prototipos de `yylex` y `yyerror`, variables globales                 |
| Tokens        | a continuación, antes del primer `%%` | `%union`, `%token`, `%start`                                                               |
| Reglas        | entre `%%` y `%%`                     | Las producciones de `01-diseno` §6                                                         |
| Código        | después del segundo `%%`              | `main` y las funciones auxiliares que invoquen las acciones. `yyerror` la provee el léxico |


Convenciones de las reglas:

- Un no terminal se arma con letras, punto, `_` y dígitos, no empieza con dígito, y puede ir en mayúsculas o minúsculas.
- Un terminal tiene que estar declarado con `%token`. Uno declarado y no usado produce advertencia de Bison.
- Varias alternativas del mismo lado izquierdo se separan con `|`.
- Toda regla termina en `;`.

---



## 4. Declaraciones



### 4.1 Valor semántico

```
%union {
    int indice_ts;
}
```

Un solo campo: todos los tokens con valor transportan un índice de la tabla de símbolos. No se transporta el lexema.

### 4.2 Tokens

```
%token <indice_ts> ID CTE CADENA

%token OP_ASIG OP_IGUAL
%token OP_SUMA OP_RESTA OP_MUL OP_DIV
%token OP_MENOR OP_MENOR_IGUAL OP_MAYOR OP_MAYOR_IGUAL OP_DISTINTO
%token PUNTO_Y_COMA PAR_ABRE PAR_CIERRA LLAVE_ABRE LLAVE_CIERRA
%token IF ELSE VAR LOOP UNTIL OR AND WRITE MAIN RETURN

%start programa
```

Son 29 tokens. 

`%start` nombra el axioma. Si se omite, Bison toma el no terminal de la primera regla. En RG se declara (S7).

La lista de tokens y la gramática de §5 coinciden exactamente.

---



## 5. Gramática

Traducción literal de `01-diseno` §6 a la sintaxis de Bison. **Ninguna regla se modifica, reordena ni corrige.**

El número de la izquierda es el que Bison asigna en `parser.output` y el que imprime la traza (`Reducing stack by rule N`). La regla 0 es `$accept: programa $end`, que agrega el generador y no forma parte de esta gramática.

```
 1  programa            : lista_funciones funcion_main
 2                      | funcion_main
                        ;

 3  lista_funciones     : lista_funciones definicion_funcion
 4                      | definicion_funcion
                        ;

 5  definicion_funcion  : ID PAR_ABRE ID PAR_CIERRA cuerpo
                        ;

 6  funcion_main        : MAIN PAR_ABRE PAR_CIERRA cuerpo
                        ;

 7  cuerpo              : LLAVE_ABRE declaraciones lista_sentencias LLAVE_CIERRA
                        ;

 8  declaraciones       : declaraciones declaracion
 9                      | %empty
                        ;

10  declaracion         : VAR ID PUNTO_Y_COMA
                        ;

11  lista_sentencias    : lista_sentencias sentencia
12                      | sentencia
                        ;

13  sentencia           : asignacion
14                      | seleccion
15                      | iteracion
16                      | salida
17                      | retorno
18                      | invocacion PUNTO_Y_COMA
                        ;

19  asignacion          : ID OP_ASIG expresion PUNTO_Y_COMA
                        ;

20  retorno             : RETURN expresion PUNTO_Y_COMA
                        ;

21  salida              : WRITE PAR_ABRE expresion PAR_CIERRA PUNTO_Y_COMA
22                      | WRITE PAR_ABRE CADENA PAR_CIERRA PUNTO_Y_COMA
                        ;

23  seleccion           : IF PAR_ABRE condiciones PAR_CIERRA bloque
24                      | IF PAR_ABRE condiciones PAR_CIERRA bloque ELSE bloque
                        ;

25  iteracion           : LOOP bloque UNTIL PAR_ABRE condiciones PAR_CIERRA PUNTO_Y_COMA
                        ;

26  bloque              : LLAVE_ABRE lista_sentencias LLAVE_CIERRA
                        ;

27  condiciones         : condiciones op_logico condicion
28                      | condicion
                        ;

29  condicion           : expresion op_comparacion expresion
30                      | PAR_ABRE condiciones PAR_CIERRA
                        ;

31  expresion           : expresion OP_SUMA termino
32                      | expresion OP_RESTA termino
33                      | termino
                        ;

34  termino             : termino OP_MUL factor
35                      | termino OP_DIV factor
36                      | factor
                        ;

37  factor              : ID
38                      | CTE
39                      | invocacion
40                      | PAR_ABRE expresion PAR_CIERRA
                        ;

41  invocacion          : ID PAR_ABRE expresion PAR_CIERRA
                        ;

42  op_comparacion      : OP_MENOR
43                      | OP_MENOR_IGUAL
44                      | OP_MAYOR
45                      | OP_MAYOR_IGUAL
46                      | OP_IGUAL
47                      | OP_DISTINTO
                        ;

48  op_logico           : AND
49                      | OR
                        ;
```



### 5.1 Notas de traducción


| Punto                   | Nota                                                                                                         |
| ----------------------- | ------------------------------------------------------------------------------------------------------------ |
| `%empty`                | Es la forma en que Bison escribe la producción vacía de `declaraciones`.                                     |
| `main` contra funciones | `funcion_main` usa el terminal `MAIN` y `definicion_funcion` usa `ID`, de modo que no se superponen          |
| `ID` seguido de `(`     | Va a `invocacion`; `ID` seguido de cualquier otra cosa reduce a `factor`. Un símbolo de anticipación alcanza |
| Llaves obligatorias     | Todos los cuerpos son `bloque` con llaves, así que no aparece el problema del `else` colgante                |
| `CADENA`                | Solo aparece en `salida`, nunca como `factor`. Así `"texto" + 1/1` lo rechaza la sintaxis                    |
| Declaraciones           | Solo en `cuerpo`, no en `bloque`: se declaran al inicio de cada función, nunca dentro de un `if` o un `loop` |




### 5.2 Acciones

Una acción se ejecuta cuando la regla se reconoce. Es código C entre `{` y `}`, y la regla sigue cerrando con `;`.


| Pseudovariable      | Qué es                                                      |
| ------------------- | ----------------------------------------------------------- |
| `$1`, `$2`, `$3`, … | Valor del 1.º, 2.º, 3.º… símbolo del lado derecho, ya leído |
| `$$`                | Valor que devuelve el lado izquierdo                        |


En `A: B C D ;`, `$1` es el valor de `B`, `$2` el de `C` y `$3` el de `D`. Si la regla no asigna `$$`, el valor es el del primer símbolo.

En RG ese valor es `indice_ts` (S5). El contenido de cada acción —qué terceto emite— se especifica en la etapa de código intermedio. Acá las acciones quedan vacías.

---



## 6. Conflictos

Una gramática es ambigua si alguna cadena puede estructurarse de dos o más formas. Bison lo detecta al generar el parser e informa la cantidad de conflictos. Igual produce el analizador, eligiendo un paso:


| Conflicto     | Elección por defecto                       |
| ------------- | ------------------------------------------ |
| Shift/reduce  | Shift                                      |
| Reduce/reduce | La regla que aparece antes en la gramática |


Una acción en medio de una regla también puede provocar un conflicto. En ese caso, resolverlo con precedencia produce un analizador incorrecto: hay que revisar la regla.

Generar siempre el informe y revisarlo antes de dar la etapa por cerrada. `parser.output` lista los estados con sus reglas punteadas. Un conflicto aparece como una celda con dos acciones posibles.

### 6.1 Criterio

Cero conflictos es el objetivo. Si queda alguno, documentar en la bitácora cuál es, por qué se acepta y cómo lo resuelve Bison por omisión.

---



## 7. Punto de entrada

```
main:
    si fopen del fuente falla: informar y salir
    yyparse()
    fclose
    si no hubo errores
        informar compilación exitosa
    si no
        informar compilación con errores
    exportar tabla de símbolos        /* consigna general 9 */
```

`main` abre el fuente una vez, llama a `yyparse` una vez y cierra. `yyparse` es quien recorre la entrada, pidiendo tokens a `yylex`. `yylex` no se define en `parser.y`: está en `src/lexico/lexico.c`.

`yyerror` la provee el léxico (spec-etapa-2 §9.2) e informa **E5** con la línea.

---



## 8. Estructura y compilación

```
src/parser/parser.y        gramática, tokens, acciones y main
src/lexico/lexico.c        yylex, matrices y acciones semánticas del AL
src/lexico/lexico.h        cabecera del léxico
```

`lexico.h` existe porque el parser y el léxico se compilan por separado. `yyparse` llama a `yylex` y a `yyerror`, y el `main` de `parser.y` abre `fuente`, mira `hay_errores` y exporta la tabla con `mostrarTS`. Esas definiciones están en `lexico.c`; la cabecera las publica para que `parser.y` pueda usarlas sin ver el autómata. Está escrita a mano: Bison solo genera `parser.tab.h` y no se usa Flex (S3). Declara `Simbolo`, `MAX_LEXEMA`, `MAX_TS`, las globales `fuente`, `linea_actual`, `hay_errores`, `ts` y `ts_size`, y los prototipos `yylex`, `yyerror`, `agregar_ts` y `mostrarTS`.

```
win_bison -d --report=all -o src/parser/parser.tab.c src/parser/parser.y
gcc src/parser/parser.tab.c src/lexico/lexico.c -o compilador.exe
```


| Salida          | Contenido                                  |
| --------------- | ------------------------------------------ |
| `parser.tab.c`  | El parser generado                         |
| `parser.tab.h`  | Los `#define` de los tokens (`-d`)         |
| `parser.output` | Estados LALR y conflictos (`--report=all`) |


`lexico.c` incluye `parser.tab.h`. La dependencia es en un solo sentido: el léxico conoce los códigos del parser.

**No enlazar Flex.** Generaría su propio `yylex` y pisaría el del grupo (S3).

---



## 9. Casos de prueba de esta fase



### 9.1 Programas correctos

Los siete de `tests/correctos/` deben parsear completos, llegando al axioma y sin errores.


| Caso             | Qué ejercita de la gramática                                               |
| ---------------- | -------------------------------------------------------------------------- |
| `p1_suma`        | Asignaciones, `write` de expresión y de literal                            |
| `p2_normalizar`  | Constantes y salida                                                        |
| `p3_loop_until`  | `iteracion` con condición al final                                         |
| `p4_if_else`     | `seleccion` con `else`, `condiciones` con `and`, `or` y paréntesis         |
| `p5_funcion`     | `definicion_funcion`, `invocacion` como sentencia y como factor, `retorno` |
| `p6_overflow`    | Iteración con acumulación                                                  |
| `p7_condiciones` | `seleccion` sin `else`, condiciones entre paréntesis unidas por `and`      |




### 9.2 Programas con error sintáctico

`tests/errores-lexicos-sintacticos/es1_sentencia.rg` debe ser rechazado, con la línea correcta informada.

Verificar además que los programas con **error léxico** (`el1_caracter`, `el2_constante`) no interrumpan el parsing: el léxico informa y sigue entregando tokens.