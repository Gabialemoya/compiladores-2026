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
| 5 | Acciones de `if` y `if … else` | **hecho** |
| 6 | Acciones de `loop … until` | **hecho** |
| 7 | Condiciones compuestas con `and` y `or` | **hecho** |
| 8 | Revisión del archivo completo contra los programas de prueba | **hecho** |

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
| CI7 | Inicio del programa | El terceto 0 es un `T_BI` al primer terceto de `main`. La gramática ya lo define como punto de entrada (D16) |

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

`yylex` no cambia: sigue cargando solo `indice_ts`.

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

Una comparación anota un terceto de tipo `ENTERO`. El operador sale de `op_comparacion`: `T_MENOR`, `T_MENOR_IGUAL`, `T_MAYOR`, `T_MAYOR_IGUAL`, `T_IGUAL` o `T_DISTINTO`. Los paréntesis de una condición propagan la referencia, sin terceto nuevo.

`if` emite `T_BF` antes del cuerpo: si la condición es falsa, salta al terceto que sigue al `if`. El destino se completa al cerrar el cuerpo, con `completar_op2`.

`if … else` hace lo mismo y, entre las dos ramas, emite `T_BI`. Ese salto pasa por encima del `else`. `completar_op2` deja el `T_BF` en el primer terceto del `else`, y `completar_op1` deja el `T_BI` en el terceto que sigue a las dos ramas. Si ese lugar es el fin del programa, el índice es el que tendría el próximo terceto y no hay fila con ese número.

Los `if` anidados usan una pila: cada uno guarda su `T_BF` y, si tiene `else`, su `T_BI`.

`and` y `or` todavía no se combinan. La condición compuesta emite cada comparación, y el `T_BF` usa la de la izquierda. El paso 7 arma el salto corto.

El `until` ya deja anotada su comparación, porque usa la misma regla. El salto del ciclo es el paso 6.

### 13.1 Criterios de aceptación

- Bison genera el parser sin conflictos. Compila sin warnings con `-Wall -Wextra`.
- Los programas de `tests/correctos/` siguen en compilación exitosa.
- Los programas con error informan lo mismo que antes.
- En `p4_if_else`, el primer `if` queda como `MAYOR`, `DISTINTO`, `BF` hacia el `else`, el `ASIG` del `then`, `BI` hacia el terceto siguiente y el `ASIG` del `else`.
- En `p7_condiciones`, el `if` sin `else` tiene un `BF` cuyo destino es el índice que sigue al `WRITE`.
- No aparece el salto del `loop`.

---

## 14. Acciones del paso 6

`loop … until` ejecuta el cuerpo y después la condición. Si la condición es falsa, vuelve al primer terceto del cuerpo. Si es verdadera, sigue con el terceto de después.

Al reconocer `loop` se guarda el índice que va a tener el cuerpo. Al cerrar el `until` se emite `T_BF`: op1 es la condición y op2 es ese índice. El destino ya se conoce, así que no hace falta completarlo después.

Un `loop` anidado guarda su propio inicio en el valor de la regla. No comparte la pila del `if`.

La comparación del `until` sigue siendo la del paso 5. Si la condición tiene `and` u `or`, el `BF` usa la comparación de la izquierda hasta el paso 7.

### 14.1 Criterios de aceptación

- Bison genera el parser sin conflictos. Compila sin warnings con `-Wall -Wextra`.
- Los programas de `tests/correctos/` siguen en compilación exitosa.
- Los programas con error informan lo mismo que antes.
- En `p3_loop_until`, después de `MAYOR n > 4/1` hay un `BF` hacia el primer terceto del cuerpo, el `SUMA` de `acc + n`.
- En `p6_overflow`, el `BF` vuelve al `DIV` de `1/1 / n`.

---

## 15. Acciones del paso 7

No hay operador de `and` ni de `or`. Cada comparación emite su terceto y, enseguida, un `T_BF`. Caer al terceto siguiente significa que esa comparación es verdadera. El `T_BF` se completa cuando ya se sabe dónde queda el caso falso.

`and` deja los `T_BF` de las dos comparaciones apuntando al mismo lugar falso. Si la izquierda es falsa, ese salto no entra en la derecha.

`or` emite un `T_BI` antes de la derecha. Si la izquierda es verdadera, ese salto pasa por encima de la derecha. Si es falsa, sus `T_BF` apuntan al inicio de la derecha.

El `if` y el `until` ya no agregan otro `T_BF`: completan los que armó la condición. Los verdaderos caen en el cuerpo del `if`, o afuera del `loop`. Los falsos van al `else`, a lo que sigue al `if`, o de vuelta al cuerpo del `loop`.

### 15.1 Criterios de aceptación

- Bison genera el parser sin conflictos. Compila sin warnings con `-Wall -Wextra`.
- Los programas de `tests/correctos/` siguen en compilación exitosa.
- Los programas con error informan lo mismo que antes.
- En `p7_condiciones`, `n > 40/1 and s <> n` deja dos `BF` hacia el índice que sigue al `WRITE`. El segundo se alcanza solo si el primero no saltó.
- En `p4_if_else`, el `or` tiene un `BI` que salta la comparación derecha cuando la izquierda es verdadera, y el `BF` de la izquierda apunta a esa comparación derecha.

---

## 16. Inicio en `main`

La gramática ya hace de `main` el punto de entrada: `programa` termina en `funcion_main` (D16). Las otras funciones se traducen antes, en el orden del fuente, y solo se ejecutan con `T_CALL`.

Para que la lista también empiece en `main`, el terceto 0 es un `T_BI`. Se emite vacío al reconocer el programa y se completa con `completar_op1` al llegar al encabezado de `main`. El destino es el primer terceto de su cuerpo.

Si el programa no tiene funciones delante, ese `BI` apunta al terceto 1. Si las tiene, las salta. El `CALL` de cada función apunta a su entrada, que ya no es 0.

---

## 17. Revisión del paso 8

Se recompila con Bison y con `gcc -Wall -Wextra`. El parser no tiene conflictos y la compilación no tiene advertencias. Cada programa de `tests/` se corre y se lee `out/tercetos.txt` contra el fuente.

El terceto 0 es el `BI` de §16, así que los índices citados en los pasos 2 a 7 quedan corridos uno respecto de esas secciones.

| Programa | Contraste |
| --- | --- |
| `p1_suma` | `BI` a 1. `SUMA` de `a + b` antes del `ASIG` de `c`. `WRITE_STR` de `Resultado` y `WRITE` de `c`. |
| `p2_normalizar` | `2/4` queda `TS:1/2`, porque el léxico ya normaliza la constante. |
| `p3_loop_until` | El `BF` del `until` vuelve al `SUMA` de `acc + n`. |
| `p4_if_else` | El `and` deja dos `BF` en el `else`. El `or` tiene un `BI` que salta la comparación derecha y el `BF` de la izquierda entra en ella. |
| `p5_funcion` | El `BI` inicial va al primer terceto de `main`. `CALL` entra a `cuadrado`, que quedó delante. |
| `p6_overflow` | `1/1 / n` es `DIV` antes que la `SUMA`. El `BF` vuelve a ese `DIV`. |
| `p7_condiciones` | Los dos `BF` del `and` apuntan al índice que sigue al `WRITE`. |
| `p8_expresiones` | `*` y `/` quedan antes que `+` y `-`. Los paréntesis no agregan terceto. |

Los programas con error léxico o sintáctico informan lo mismo que antes de esta etapa. `em1_declaracion` y `em2_recursion` siguen compilando: E6, E7 y E9 están fuera de alcance (§1.1).

Ningún `BF`, `BI` ni `CALL` queda con destino vacío. Un destino igual a la cantidad de tercetos es el lugar que sigue al último, y aparece cuando el `if` o el `else` cierran el programa.