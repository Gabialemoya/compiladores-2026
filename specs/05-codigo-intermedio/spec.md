# Spec — Código intermedio de RG (tercetos)

**Depende de:** `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md` · `specs/04-analizador-sintactico/spec.md`
**Produce:** `src/intermedio/` · las acciones semánticas de `parser.y` · `out/tercetos.txt`

---

## 1. Alcance

Traducir el programa fuente a una lista de tercetos. La lista es el insumo de la etapa de generación de assembler y el artefacto que pide la **consigna general 12**.

**No se ejecuta nada.** La acción de una regla anota la operación, no la resuelve. Si la regla de la suma calculara el resultado, el compilador estaría compilando y ejecutando a la vez.

### 1.1 Fuera de alcance

- La traducción de cada terceto a assembler y las rutinas de aritmética racional.
- Los controles semánticos E6, E7 y E9.

---

## 2. Plan de la etapa

La etapa se construye en pasos. Cada uno se implementa, se prueba y se registra en la bitácora antes de pasar al siguiente.

| Paso | Qué agrega | Estado |
| ---: | --- | --- |
| 1 | Infraestructura: módulo de tercetos y ampliación del valor semántico | **hecho** |
| 2 | Acciones de expresiones y asignación | **hecho** |
| 3 | Acciones de `write` y `return` | **hecho** |
| 4 | Acciones de invocación de funciones | **hecho** |
| 5 | Acciones de `if` y `if … else`, con condiciones compuestas con `and` y `or` | **hecho** |
| 6 | Acciones de `loop … until` | pendiente |
| 7 | Condiciones compuestas con `and` y `or` | unido al paso 5 |
| 8 | Revisión del archivo completo contra los programas de prueba | pendiente |

Las secciones 3 a 6 corresponden al **paso 1**. Las acciones por regla se agregan a partir del paso 2.

---

## 3. Decisiones de esta etapa

| # | Decisión | Valor |
| --- | --- | --- |
| CI1 | Representación intermedia | Tercetos |
| CI2 | Identidad del terceto | Su índice en la lista. Es a la vez su temporario y su destino de salto |
| CI3 | Temporarios | Uno por terceto que produce valor. Dirección estática fija (F.5 punto 17) |
| CI4 | Línea del fuente | Cada terceto la guarda. La exige F.2 punto 10 |
| CI5 | Saltos | Backpatching sobre índices de terceto. No existen etiquetas |
| CI6 | Ubicación del módulo | `src/intermedio/`, separado de `parser.y` |

---

## 4. Formato del terceto

| Campo | Contenido |
| --- | --- |
| `operador` | Ver §5 |
| `op1` | Referencia |
| `op2` | Referencia, o vacía en los unarios |
| `tipo` | `TIPO_RACIONAL`, `TIPO_ENTERO` o `TIPO_NINGUNO` |
| `linea` | Línea del programa fuente |

El **índice** no es un campo: es la posición en el arreglo.

### 4.1 Referencias

Un operando no es un número suelto: lleva su clase, porque el generador de assembler emite código distinto según a qué apunte.

| Clase | Apunta a |
| --- | --- |
| `REF_VACIO` | Nada. El `op2` de un unario |
| `REF_TS` | Entrada de la tabla de símbolos: variable, constante o cadena |
| `REF_TER` | Resultado de otro terceto, o sea su temporario |
| `REF_DIR` | Índice de terceto, como destino de un salto |

### 4.2 Tipo del resultado

`TIPO_ENTERO` existe porque **F.1 punto 5** define el resultado de toda comparación como el entero `0` o `−1`, no como un racional. Es el campo que le dice al generador de assembler qué rutina corresponde a cada operando.

---

## 5. Operadores

| Grupo | Operadores | op1 | op2 | tipo |
| --- | --- | --- | --- | --- |
| Aritméticos | `T_SUMA` `T_RESTA` `T_MUL` `T_DIV` | operando | operando | RACIONAL |
| Asignación | `T_ASIG` | destino | origen | NINGUNO |
| Comparación | `T_IGUAL` `T_DISTINTO` `T_MENOR` `T_MENOR_IGUAL` `T_MAYOR` `T_MAYOR_IGUAL` | operando | operando | ENTERO |
| Saltos | `T_BI` | `REF_DIR` | vacío | NINGUNO |
| | `T_BF` | condición | `REF_DIR` | NINGUNO |
| Salida | `T_WRITE` | expresión | vacío | NINGUNO |
| | `T_WRITE_STR` | `REF_TS` de la cadena | vacío | NINGUNO |
| Funciones | `T_CALL` | `REF_DIR` de la entrada | vacío | NINGUNO |
| | `T_RET` | vacío | vacío | NINGUNO |

No hay operador de etiqueta: el destino de un salto es el índice del terceto.

---

## 6. El módulo `src/intermedio/`

### 6.1 Estructuras — `tercetos.h`

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

### 6.2 Funciones

| Firma | Qué hace |
| --- | --- |
| `Referencia ref_vacia(void)` | Construye una referencia vacía |
| `Referencia ref_ts(int indice)` | Referencia a la tabla de símbolos |
| `Referencia ref_ter(int indice)` | Referencia al resultado de un terceto |
| `Referencia ref_dir(int indice)` | Referencia a un índice de terceto |
| `int emitir(OperadorTerceto op, Referencia op1, Referencia op2, TipoResultado tipo)` | Agrega el terceto y **devuelve su índice**. La línea es la de `linea_anterior`: el lookahead ya dejó `linea_actual` en el token siguiente. Si todavía no hay token anterior, usa `linea_actual` |
| `int proximo(void)` | Índice que tendrá el próximo terceto, sin emitirlo |
| `void completar_op1(int indice, int destino)` | Rellena un destino pendiente en `op1` |
| `void completar_op2(int indice, int destino)` | Rellena un destino pendiente en `op2` |
| `void imprimir_tercetos(const char* ruta)` | Vuelca la lista al archivo |

Son dos funciones de completado porque el destino va en lugares distintos: en `T_BI` está en `op1` y en `T_BF` en `op2`.

`emitir` controla el límite de `MAX_TERCETOS`: si se supera, informa, levanta `hay_errores` y devuelve `-1`.

### 6.3 Valor semántico ampliado

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

**En el paso 1 no se declaran los `%type`.** Declararlos sin escribir las acciones haría que Bison aplique su acción por defecto `$$ = $1` entre campos de tipos distintos, y eso no compila. Los `%type` entran en el paso 2, junto con las acciones.

`yylex` no cambia: sigue cargando solo `indice_ts`. El paso 5 suma dos campos, `cond` y `salto` (§13.1).

---

## 7. Formato del archivo de salida

`out/tercetos.txt`, una línea por terceto:

```
#     OPERADOR      OP1                  OP2                  TIPO       LINEA
0     BI            DIR:1                -                    -          1
1     ASIG          TS:a                 TS:1/2               -          3
2     SUMA          TS:a                 TS:b                 RACIONAL   5
3     WRITE         TER:2                -                    -          6
```

Las referencias a la tabla de símbolos se imprimen **con el nombre del símbolo**, no con el índice: el archivo se lee a mano durante la corrección y el nombre es lo que permite contrastarlo contra el programa fuente.

El archivo se escribe al terminar `yyparse`, haya o no errores.

---

## 8. Compilación

El módulo se suma a la línea de compilación:

```
bison -d --report=all -o src/parser/parser.tab.c src/parser/parser.y
gcc -Wall -Wextra -o compilador src/parser/parser.tab.c src/lexico/lexico.c src/intermedio/tercetos.c
```

---

## 9. Criterios de aceptación del paso 1

- Bison genera el parser **sin conflictos**, igual que antes.
- Compila sin warnings con `-Wall -Wextra`.
- Los **7 programas** de `tests/correctos/` siguen terminando en compilación exitosa.
- Los programas con error siguen reportando lo mismo que antes.
- Se crea `out/tercetos.txt` con la línea de encabezado y **ninguna fila**, porque todavía no hay acciones que emitan.

El último punto es el que confirma que la infraestructura quedó conectada: el archivo existe y está vacío.

---

## 10. Acciones del paso 2

```
%type <ref> expresion termino factor
```

Una operación anota un terceto y devuelve `ref_ter` de su índice. Un operando suelto no anota nada: devuelve la referencia que ya tiene.

| Regla | Qué emite | Qué devuelve |
| --- | --- | --- |
| `asignacion` | `T_ASIG`, destino `ref_ts` del `ID`, origen la expresión, `TIPO_NINGUNO` | nada |
| `expresion` `+` o `-` `termino` | `T_SUMA` o `T_RESTA`, `TIPO_RACIONAL` | `ref_ter` |
| `expresion : termino` | nada | la referencia del término |
| `termino` `*` o `/` `factor` | `T_MUL` o `T_DIV`, `TIPO_RACIONAL` | `ref_ter` |
| `termino : factor` | nada | la referencia del factor |
| `factor` `ID` o `CTE` | nada | `ref_ts` |
| `factor : ( expresion )` | nada | la referencia de la expresión |
| `factor : invocacion` | nada | `ref_vacia`, hasta el paso 4 |

La expresión argumento de una invocación sí emite sus operaciones. El resultado de la llamada queda vacío, así que `r = cuadrado(3/2)` se ve como un `T_ASIG` con origen `-`. El paso 4 reemplaza esa referencia vacía.

No se emite la inicialización en `0/1` de R3. No hay un operador distinto de `T_ASIG` para eso, y el ejemplo de §7 no la muestra.

### 10.1 Criterios de aceptación

- Bison genera el parser sin conflictos. Compila sin warnings con `-Wall -Wextra`.
- Los 7 programas de `tests/correctos/` siguen en compilación exitosa.
- Los programas con error informan lo mismo que antes.
- `p1_suma` produce, en este orden, `ASIG a = 1/2` (línea 5), `ASIG b = 1/3` (línea 6), `SUMA a + b` (línea 7) y `ASIG c` con el resultado de esa suma (línea 7).
- No aparecen `WRITE`, saltos, comparaciones ni `CALL`.

---

## 11. Acciones del paso 3

| Regla | Qué emite |
| --- | --- |
| `write( expresion )` | `T_WRITE`, op1 la expresión, op2 vacío, `TIPO_NINGUNO` |
| `write( cadena )` | `T_WRITE_STR`, op1 `ref_ts` de la cadena, op2 vacío, `TIPO_NINGUNO` |
| `return expresion` | `T_ASIG` del valor a `@ret_<funcion>`, y después `T_RET` con los dos operandos vacíos |

`T_RET` no transporta el racional: §5 lo deja vacío. R11 igual exige fijar el valor de retorno, y eso es el `T_ASIG` previo. La celda es una entrada de la tabla, `@ret_` más el nombre de la función (`@ret_main` en `main`). Se crea la primera vez que aparece un `return` de esa función.

El nombre de la función se toma al reconocer el encabezado, antes del cuerpo, porque el `return` se reduce adentro.

### 11.1 Criterios de aceptación

- Bison genera el parser sin conflictos. Compila sin warnings con `-Wall -Wextra`.
- Los 7 programas de `tests/correctos/` siguen en compilación exitosa.
- Los programas con error informan lo mismo que antes.
- En `p1_suma`, después de los cuatro tercetos del paso 2, siguen `WRITE_STR` de `Resultado` (línea 8) y `WRITE` de `c` (línea 9).
- En `p5_funcion`, `return x * x` queda como `MUL`, `ASIG` a `@ret_cuadrado` y `RET`, los tres en la línea 2.
- No aparecen saltos, comparaciones ni `CALL`.

---

## 12. Acciones del paso 4

`%type <ref> invocacion`. `factor : invocacion` propaga esa referencia.

Cada llamada emite tres tercetos, en este orden:

| Terceto | op1 | op2 |
| --- | --- | --- |
| `T_ASIG` | parámetro de la función | argumento |
| `T_CALL` | `REF_DIR` de la entrada | vacío |
| `T_ASIG` | `@tmpN` | `@ret_<funcion>` |

La invocación devuelve `ref_ts` de `@tmpN`. El temporal existe porque `@ret_<funcion>` es una sola celda: `cuadrado(2/3) + cuadrado(1/9)` haría las dos sumas sobre el mismo lugar si el resultado no se copiara antes de la segunda llamada.

La entrada es el índice que va a tener el primer terceto del cuerpo. Se anota al reconocer el encabezado, junto con el parámetro. Una llamada recursiva ya conoce esa entrada. Una llamada a una función que todavía no apareció deja los tres tercetos pendientes y se completan cuando llega su encabezado.

### 12.1 Criterios de aceptación

- Bison genera el parser sin conflictos. Compila sin warnings con `-Wall -Wextra`.
- Los programas de `tests/correctos/` siguen en compilación exitosa.
- Los programas con error informan lo mismo que antes.
- En `p5_funcion`, `r = cuadrado(3/2)` queda como `ASIG` de `3/2` en `x`, `CALL` a `DIR:0`, copia de `@ret_cuadrado` a `@tmp0` y `ASIG` de ese temporal en `r`. Los cuatro en la línea 7.
- `write(cuadrado(2/3) + 1/9)` suma `@tmp1` con `1/9`.
- No aparecen saltos ni comparaciones.


---

## 13. Acciones del paso 5

Este paso incluye el 7: `if`, `if … else` y las condiciones con `and` y `or` van juntos, porque los dos programas de prueba con `if` (`p4_if_else` y `p7_condiciones`) usan condiciones compuestas.

### 13.1 Valor semántico

```
%code requires {
    #include "../intermedio/tercetos.h"

    typedef struct {
        int verdadero;
        int falso;
        int fin;
    } Condicion;
}

%union {
    int        indice_ts;
    Referencia ref;
    int        operador;
    Condicion  cond;    /* condicion y condiciones          */
    int        salto;   /* BI del else, pendiente           */
}

%type <operador> op_comparacion op_logico
%type <cond>     condiciones condicion
```

`op_comparacion` devuelve el operador de terceto (`T_MENOR`, `T_MAYOR`, …). `op_logico` devuelve el código de token, `AND` u `OR`: no hay operador de terceto para ellos, porque `and` y `or` no anotan una operación sino saltos.

`Condicion` va en `%code requires` por el mismo motivo que `Referencia` (§6.3): el `%union` lo usa y `parser.tab.h` lo tiene que conocer.

### 13.2 Saltos pendientes

Una condición se traduce antes que el bloque, así que no sabe adónde salta. Devuelve los saltos que dejó sin destino, en dos listas (CI5):

| Campo | Contiene | Destino final |
| --- | --- | --- |
| `falso` | Los `T_BF` que saltan cuando la condición no se cumple | El `else`, o lo que sigue al `if` |
| `verdadero` | Los `T_BI` que saltan cuando se cumple, sin evaluar el resto | El primer terceto del bloque del `if` |
| `fin` | Índice del terceto que sigue a la condición | — |

`verdadero` solo la llenan los `or`. Una condición sin `or` llega al bloque por caída, sin saltar.

Las listas se encadenan en `parser.y` con un arreglo `siguiente[MAX_TERCETOS]`: cada lista es el índice de su primer salto, `-1` si está vacía. Completar una lista rellena `op2` en cada `T_BF` y `op1` en cada `T_BI` (§6.2).

### 13.3 Reglas

| Regla | Qué emite | Qué devuelve |
| --- | --- | --- |
| `condicion : expresion op_comparacion expresion` | La comparación, `TIPO_ENTERO`, y un `T_BF` sobre ella con destino pendiente | `falso` con ese `T_BF`, `verdadero` vacía |
| `condicion : ( condiciones )` | nada | la condición de adentro |
| `condiciones : condicion` | nada | la condición |
| `condiciones op_logico` … (acción intermedia), `and` | nada. Completa `verdadero` de la izquierda con `proximo()` | `falso` de la izquierda, `verdadero` vacía |
| `condiciones op_logico` … (acción intermedia), `or` | Un `T_BI` pendiente. Después completa `falso` de la izquierda con `proximo()` | `verdadero` de la izquierda más ese `T_BI`, `falso` vacía |
| `condiciones op_logico condicion` | nada | las dos listas unidas con las de la derecha |
| `if ( c ) bloque` | nada | completa `verdadero` con `c.fin` y `falso` con `proximo()` |
| `if ( c ) bloque else` … (acción intermedia) | Un `T_BI` pendiente, para saltar el `else` | completa `verdadero` con `c.fin` y `falso` con `proximo()` |
| `if ( c ) bloque else bloque` | nada | completa el `T_BI` con `proximo()` |
| `op_comparacion` | nada | `T_MENOR`, `T_MENOR_IGUAL`, `T_MAYOR`, `T_MAYOR_IGUAL`, `T_IGUAL` o `T_DISTINTO` |
| `op_logico` | nada | `AND` u `OR` |

En un `and`, si la izquierda se cumple se evalúa la derecha, que empieza en `proximo()`; si no se cumple, toda la condición es falsa. En un `or`, si la izquierda se cumple toda la condición es verdadera: el `T_BI` salta al bloque. Si no se cumple, se evalúa la derecha, que empieza después de ese `T_BI`.

Si el `if` es la última sentencia, el destino es `cant_tercetos`, un índice sin terceto: el fin del programa.

### 13.4 Dónde van las acciones

- **No hay acción intermedia después de `)` en el `if`.** Las dos alternativas de `seleccion` empiezan con `IF ( condiciones ) bloque`. Dos acciones intermedias en el mismo lugar son dos no terminales vacíos distintos, y Bison no puede elegir cuál reducir: conflicto reducción/reducción. Por eso la condición trae `fin`, y la lista `verdadero` se completa al reducir el `if`.
- **La acción del `else` va después de `ELSE`.** Ahí solo queda una alternativa, así que no hay conflicto. El `T_BI` tiene que emitirse antes del bloque del `else`.
- **La acción de `and` y `or` va entre `op_logico` y `condicion`.** El `T_BI` del `or` tiene que quedar antes de la segunda comparación, y los destinos que apuntan a la segunda condición se conocen recién ahí.

Las tres son acciones intermedias: no cambian las producciones de `04-analizador-sintactico` §5.

`loop … until` usa `condiciones` pero todavía no tiene acciones: en `p3` y `p6` el `T_BF` del `until` queda con destino `-` hasta el paso 6.

### 13.5 Criterios de aceptación

- Bison genera el parser sin conflictos ni warnings. Compila sin warnings con `-Wall -Wextra`.
- Los 8 programas de `tests/correctos/` siguen en compilación exitosa.
- Los programas con error informan lo mismo que antes.
- Ningún `T_BF` ni `T_BI` de un `if` queda con destino `-`.
- `p7_condiciones` (`and`, `if` sin `else`):

```
#     OPERADOR      OP1                  OP2                  TIPO       LINEA
0     BI            DIR:1                -                    -          1
1     ASIG          TS:s                 TS:0/1               -          4
2     ASIG          TS:n                 TS:1/1               -          5
3     MAYOR         TS:n                 TS:40/1              ENTERO     6
4     BF            TER:3                DIR:8                -          6
5     DISTINTO      TS:s                 TS:n                 ENTERO     6
6     BF            TER:5                DIR:8                -          6
7     WRITE         TS:s                 -                    -          7
```

- `p4_if_else` (`and` y `or`, los dos con `else`):

```
#     OPERADOR      OP1                  OP2                  TIPO       LINEA
0     BI            DIR:1                -                    -          1
1     ASIG          TS:a                 TS:3/4               -          5
2     ASIG          TS:b                 TS:1/2               -          6
3     MAYOR         TS:a                 TS:b                 ENTERO     7
4     BF            TER:3                DIR:9                -          7
5     DISTINTO      TS:a                 TS:1/1               ENTERO     7
6     BF            TER:5                DIR:9                -          7
7     ASIG          TS:r                 TS:1/1               -          8
8     BI            DIR:10               -                    -          9
9     ASIG          TS:r                 TS:0/1               -          10
10    MENOR         TS:a                 TS:0/1               ENTERO     12
11    BF            TER:10               DIR:13               -          12
12    BI            DIR:15               -                    -          12
13    MENOR_IGUAL   TS:b                 TS:1/2               ENTERO     12
14    BF            TER:13               DIR:17               -          12
15    WRITE         TS:r                 -                    -          13
16    BI            DIR:18               -                    -          14
17    WRITE         TS:0/1               -                    -          15
```



---

## 13. Acciones del paso 5

Este paso incluye el 7: `if`, `if … else` y las condiciones con `and` y `or` van juntos, porque los dos programas de prueba con `if` (`p4_if_else` y `p7_condiciones`) usan condiciones compuestas.

### 13.1 Valor semántico

```
%code requires {
    #include "../intermedio/tercetos.h"

    typedef struct {
        int verdadero;
        int falso;
        int fin;
    } Condicion;
}

%union {
    int        indice_ts;
    Referencia ref;
    int        operador;
    Condicion  cond;    /* condicion y condiciones          */
    int        salto;   /* BI del else, pendiente           */
}

%type <operador> op_comparacion op_logico
%type <cond>     condiciones condicion
```

`op_comparacion` devuelve el operador de terceto (`T_MENOR`, `T_MAYOR`, …). `op_logico` devuelve el código de token, `AND` u `OR`: no hay operador de terceto para ellos, porque `and` y `or` no anotan una operación sino saltos.

`Condicion` va en `%code requires` por el mismo motivo que `Referencia` (§6.3): el `%union` lo usa y `parser.tab.h` lo tiene que conocer.

### 13.2 Saltos pendientes

Una condición se traduce antes que el bloque, así que no sabe adónde salta. Devuelve los saltos que dejó sin destino, en dos listas (CI5):

| Campo | Contiene | Destino final |
| --- | --- | --- |
| `falso` | Los `T_BF` que saltan cuando la condición no se cumple | El `else`, o lo que sigue al `if` |
| `verdadero` | Los `T_BI` que saltan cuando se cumple, sin evaluar el resto | El primer terceto del bloque del `if` |
| `fin` | Índice del terceto que sigue a la condición | — |

`verdadero` solo la llenan los `or`. Una condición sin `or` llega al bloque por caída, sin saltar.

Las listas se encadenan en `parser.y` con un arreglo `siguiente[MAX_TERCETOS]`: cada lista es el índice de su primer salto, `-1` si está vacía. Completar una lista rellena `op2` en cada `T_BF` y `op1` en cada `T_BI` (§6.2).

### 13.3 Reglas

| Regla | Qué emite | Qué devuelve |
| --- | --- | --- |
| `condicion : expresion op_comparacion expresion` | La comparación, `TIPO_ENTERO`, y un `T_BF` sobre ella con destino pendiente | `falso` con ese `T_BF`, `verdadero` vacía |
| `condicion : ( condiciones )` | nada | la condición de adentro |
| `condiciones : condicion` | nada | la condición |
| `condiciones op_logico` … (acción intermedia), `and` | nada. Completa `verdadero` de la izquierda con `proximo()` | `falso` de la izquierda, `verdadero` vacía |
| `condiciones op_logico` … (acción intermedia), `or` | Un `T_BI` pendiente. Después completa `falso` de la izquierda con `proximo()` | `verdadero` de la izquierda más ese `T_BI`, `falso` vacía |
| `condiciones op_logico condicion` | nada | las dos listas unidas con las de la derecha |
| `if ( c ) bloque` | nada | completa `verdadero` con `c.fin` y `falso` con `proximo()` |
| `if ( c ) bloque else` … (acción intermedia) | Un `T_BI` pendiente, para saltar el `else` | completa `verdadero` con `c.fin` y `falso` con `proximo()` |
| `if ( c ) bloque else bloque` | nada | completa el `T_BI` con `proximo()` |
| `op_comparacion` | nada | `T_MENOR`, `T_MENOR_IGUAL`, `T_MAYOR`, `T_MAYOR_IGUAL`, `T_IGUAL` o `T_DISTINTO` |
| `op_logico` | nada | `AND` u `OR` |

En un `and`, si la izquierda se cumple se evalúa la derecha, que empieza en `proximo()`; si no se cumple, toda la condición es falsa. En un `or`, si la izquierda se cumple toda la condición es verdadera: el `T_BI` salta al bloque. Si no se cumple, se evalúa la derecha, que empieza después de ese `T_BI`.

Si el `if` es la última sentencia, el destino es `cant_tercetos`, un índice sin terceto: el fin del programa.

### 13.4 Dónde van las acciones

- **No hay acción intermedia después de `)` en el `if`.** Las dos alternativas de `seleccion` empiezan con `IF ( condiciones ) bloque`. Dos acciones intermedias en el mismo lugar son dos no terminales vacíos distintos, y Bison no puede elegir cuál reducir: conflicto reducción/reducción. Por eso la condición trae `fin`, y la lista `verdadero` se completa al reducir el `if`.
- **La acción del `else` va después de `ELSE`.** Ahí solo queda una alternativa, así que no hay conflicto. El `T_BI` tiene que emitirse antes del bloque del `else`.
- **La acción de `and` y `or` va entre `op_logico` y `condicion`.** El `T_BI` del `or` tiene que quedar antes de la segunda comparación, y los destinos que apuntan a la segunda condición se conocen recién ahí.

Las tres son acciones intermedias: no cambian las producciones de `04-analizador-sintactico` §5.

`loop … until` usa `condiciones` pero todavía no tiene acciones: en `p3` y `p6` el `T_BF` del `until` queda con destino `-` hasta el paso 6.

### 13.5 Criterios de aceptación

- Bison genera el parser sin conflictos ni warnings. Compila sin warnings con `-Wall -Wextra`.
- Los 8 programas de `tests/correctos/` siguen en compilación exitosa.
- Los programas con error informan lo mismo que antes.
- Ningún `T_BF` ni `T_BI` de un `if` queda con destino `-`.
- `p7_condiciones` (`and`, `if` sin `else`):

```
#     OPERADOR      OP1                  OP2                  TIPO       LINEA
0     BI            DIR:1                -                    -          1
1     ASIG          TS:s                 TS:0/1               -          4
2     ASIG          TS:n                 TS:1/1               -          5
3     MAYOR         TS:n                 TS:40/1              ENTERO     6
4     BF            TER:3                DIR:8                -          6
5     DISTINTO      TS:s                 TS:n                 ENTERO     6
6     BF            TER:5                DIR:8                -          6
7     WRITE         TS:s                 -                    -          7
```

- `p4_if_else` (`and` y `or`, los dos con `else`):

```
#     OPERADOR      OP1                  OP2                  TIPO       LINEA
0     BI            DIR:1                -                    -          1
1     ASIG          TS:a                 TS:3/4               -          5
2     ASIG          TS:b                 TS:1/2               -          6
3     MAYOR         TS:a                 TS:b                 ENTERO     7
4     BF            TER:3                DIR:9                -          7
5     DISTINTO      TS:a                 TS:1/1               ENTERO     7
6     BF            TER:5                DIR:9                -          7
7     ASIG          TS:r                 TS:1/1               -          8
8     BI            DIR:10               -                    -          9
9     ASIG          TS:r                 TS:0/1               -          10
10    MENOR         TS:a                 TS:0/1               ENTERO     12
11    BF            TER:10               DIR:13               -          12
12    BI            DIR:15               -                    -          12
13    MENOR_IGUAL   TS:b                 TS:1/2               ENTERO     12
14    BF            TER:13               DIR:17               -          12
15    WRITE         TS:r                 -                    -          13
16    BI            DIR:18               -                    -          14
17    WRITE         TS:0/1               -                    -          15
```