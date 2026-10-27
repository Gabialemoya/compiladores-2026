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
| 5 | Acciones de `if` y `if … else` | pendiente |
| 6 | Acciones de `loop … until` | pendiente |
| 7 | Condiciones compuestas con `and` y `or` | pendiente |
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