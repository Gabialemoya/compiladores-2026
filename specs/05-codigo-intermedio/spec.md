# Spec — Código intermedio de RG (tercetos)

**Depende de:** `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md` · `specs/04-analizador-sintactico/spec.md`
**Produce:** `src/intermedio/` · las acciones semánticas de `parser.y` · `out/tercetos.txt`

Traduce el programa fuente a una lista de tercetos mientras Bison reduce. Cada reducción anota la operación. No calcula el racional. La lista es el insumo de la generación de assembler.

---

## 1. Alcance

Durante `yyparse`, las reducciones de §6 apilan un operando, agregan un terceto, completan un salto, o dejan pasar el valor que ya está en la pila. Al terminar `yyparse`, haya errores o no, `main` escribe `out/tercetos.txt`.

Queda fuera de esta etapa la traducción de cada terceto a assembler y los controles semánticos E6, E7 y E9.

---

## 2. El terceto

| Campo | Contenido |
| --- | --- |
| `operador` | §2.2 |
| `op1` | Referencia. En un binario, el operando izquierdo |
| `op2` | Referencia. En un binario, el operando derecho. Vacía cuando el operador usa un solo operando |
| `tipo` | `TIPO_RACIONAL`, `TIPO_ENTERO` o `TIPO_NINGUNO` |
| `linea` | Línea del programa fuente |

El índice no es un campo: es la posición en la lista. Ese índice es el temporario del resultado y, si otro terceto salta hacia aquí, el destino del salto. El temporario tiene dirección estática fija. No hay etiquetas.

### 2.1 Referencias

| Clase | Apunta a |
| --- | --- |
| `REF_VACIO` | Ningún operando |
| `REF_TS` | Entrada de la tabla de símbolos: variable, constante, cadena, `@ret_` o `@tmpN` |
| `REF_TER` | Resultado de otro terceto |
| `REF_DIR` | Índice de terceto. Destino de un salto o entrada de un `CALL` |

Una operación aritmética tiene tipo `TIPO_RACIONAL`. Una comparación tiene tipo `TIPO_ENTERO`: su resultado es el entero `0` o `−1`. Asignación, salto, salida, llamada y retorno tienen tipo `TIPO_NINGUNO`.

### 2.2 Operadores

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

El destino de `T_BI` y de `T_CALL` ocupa `op1`. El destino de `T_BF` ocupa `op2`. Hasta que se conoce, ese operando queda vacío.

---

## 3. Módulo `src/intermedio/`

### 3.1 Estructuras — `tercetos.h`

```c
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
```

`Cond.falso` guarda índices de `T_BF` con `op2` pendiente. `Cond.verdadero` guarda índices de `T_BI` con `op1` pendiente.

### 3.2 Operaciones — `tercetos.c`

`colocar` y `crear_terceto` agregan un terceto al final de la lista y devuelven su índice. La línea guardada es `linea_anterior`. Si todavía no hay token anterior, usan `linea_actual`.

| Firma | Efecto |
| --- | --- |
| `Referencia apilar(int ptr)` | Referencia `REF_TS` al índice `ptr` de la tabla de símbolos |
| `Referencia apilar_ti(int ti)` | Referencia `REF_TER` al terceto `ti` |
| `Referencia sin_operando(void)` | Referencia `REF_VACIO` |
| `int colocar(OperadorTerceto op, Referencia op1, Referencia op2, TipoResultado tipo)` | Agrega un terceto de operación, asignación, salida o retorno |
| `int crear_terceto(OperadorTerceto op, Referencia op1, Referencia op2, TipoResultado tipo)` | Agrega un terceto de salto o de llamada |
| `int nro_terceto(void)` | Índice que tendrá el próximo terceto. No lo agrega |
| `void completar(int t, int destino)` | Escribe `REF_DIR` con `destino`. En un `T_BF` lo escribe en `op2`. En un `T_BI` o un `T_CALL`, en `op1` |
| `void apilar_nro(int nro)` | Apila un número de terceto |
| `int desapilar(void)` | Saca el último número apilado. Con la pila vacía devuelve `-1` |
| `void imprimir_tercetos(const char *ruta)` | Escribe la lista en `ruta` y crea el directorio `out/` si no existe |

`completar` no modifica un índice fuera de la lista. `apilar_nro` admite 64 números. Si se llena, informa `Linea N: Error: pila de saltos llena` y levanta `hay_errores`.

Si `colocar` o `crear_terceto` supera `MAX_TERCETOS`, informa `Linea N: Error: lista de tercetos llena`, levanta `hay_errores` y devuelve `-1`.

### 3.3 Estado en `parser.y`

| Estado | Para qué |
| --- | --- |
| `funcion_actual` | Nombre de la función cuyo cuerpo se está traduciendo. Arranca en `"main"` |
| `funciones[]` | Hasta 64 funciones: índice del nombre, índice del parámetro e índice del primer terceto del cuerpo |
| `pendientes[]` | Hasta 256 llamadas cuya función todavía no apareció: nombre y los tres índices de terceto a completar |
| `cant_tmp` | Próximo número de `@tmpN` |
| `salto_entrada` | Índice del `BI` inicial |
| `pila_cond[]` | Hasta 64 `if` anidados. Cada nivel guarda el `Cond` con los `BF` abiertos |

La pila de `apilar_nro` guarda, para cada `if`, el índice de su `BI` o `-1` si ese `if` todavía no tiene `else`. También guarda el índice de inicio de cada `loop`.

Si se supera un tope, la acción informa la línea, levanta `hay_errores` y no agrega el elemento que no entra: `demasiadas funciones`, `demasiadas llamadas pendientes`, `condicion demasiado compleja`, `anidamiento de if demasiado profundo` o `pila de saltos llena`.

---

## 4. Valor semántico

```
%code requires {
    #include "../intermedio/tercetos.h"
}

%union {
    int        indice_ts;   /* lo carga yylex para ID, CTE, CADENA */
    Referencia ref;         /* operando apilado                    */
    int        operador;    /* op_comparacion y op_logico          */
    Cond       cond;        /* saltos incompletos de una condición */
}

%type <ref> expresion termino factor invocacion
%type <cond> condiciones condicion
%type <operador> op_comparacion op_logico
```

`%code requires` copia `Referencia` y `Cond` a `parser.tab.h`. `yylex` sigue cargando solo `indice_ts`.

La pila de operandos es el valor semántico de Bison. Apilar un resultado es devolverlo en `$$`.

Si la alternativa no tiene acción y el símbolo de la izquierda usa el mismo campo que el primero de la derecha, Bison copia con `$$ = $1`. Eso vale para `expresion : termino`, `termino : factor` y `condiciones : condicion`.

| Regla | Acción |
| --- | --- |
| `factor : ( expresion )` | `$$ = $2` |
| `factor : invocacion` | `$$ = $1` |
| `condicion : ( condiciones )` | `$$ = $2` |

---

## 5. Reglas que no agregan un terceto

Listas, `cuerpo`, `bloque`, `declaraciones` y `declaracion` no apilan un operando nuevo ni agregan un terceto. `var` no agrega la inicialización en `0/1` de R3: ese valor queda para la memoria estática.

`op_comparacion` apila el operador y no agrega terceto:

| Token | Operador |
| --- | --- |
| `<` | `T_MENOR` |
| `<=` | `T_MENOR_IGUAL` |
| `>` | `T_MAYOR` |
| `>=` | `T_MAYOR_IGUAL` |
| `==` | `T_IGUAL` |
| `<>` | `T_DISTINTO` |

`op_logico` apila `1` para `and` y `0` para `or`.

---

## 6. Qué agrega cada reducción

### 6.1 Programa, funciones y `main`

| Regla | Generación |
| --- | --- |
| `entrada` vacía | `salto_entrada = crear_terceto(T_BI, sin_operando(), sin_operando(), TIPO_NINGUNO)`. Es el terceto 0 |
| Encabezado `ID ( ID )`, antes del cuerpo | `funcion_actual` pasa a ser ese nombre. Crea `@ret_<nombre>` en la tabla, de tipo racional. La entrada de la función es `nro_terceto()`. La registra junto con su parámetro y completa las llamadas pendientes de ese nombre |
| Encabezado `MAIN ( )`, antes del cuerpo | `funcion_actual = "main"`. `completar(salto_entrada, nro_terceto())` |

`agregar_ts` devuelve la misma entrada si `@ret_<nombre>` ya estaba. El encabezado de una función y su `return` comparten esa celda. En `main` la celda se crea en el `return`. Si `main` no tiene `return`, la celda no se crea.

Completar una llamada pendiente deja así los tres tercetos:

| Terceto | Qué se escribe |
| --- | --- |
| `ASIG` del argumento | `op1` = `apilar(parámetro)` |
| `CALL` | `completar(call, entrada)` |
| `ASIG` del retorno | `op2` = `apilar` de `@ret_<nombre>` |

Las funciones quedan delante de `main`, en el orden del fuente. Solo se entra a ellas con `CALL`. El `BI` inicial las salta. `main` es la entrada del programa (D16).

### 6.2 Expresiones

| Regla | Agrega | Apila |
| --- | --- | --- |
| `expresion : expresion + termino` | `colocar(T_SUMA, $1, $3, TIPO_RACIONAL)` | `apilar_ti` de ese índice |
| `expresion : expresion - termino` | `colocar(T_RESTA, $1, $3, TIPO_RACIONAL)` | `apilar_ti` |
| `expresion : termino` | nada | la referencia del término |
| `termino : termino * factor` | `colocar(T_MUL, $1, $3, TIPO_RACIONAL)` | `apilar_ti` |
| `termino : termino / factor` | `colocar(T_DIV, $1, $3, TIPO_RACIONAL)` | `apilar_ti` |
| `termino : factor` | nada | la referencia del factor |
| `factor : ID` | nada | `apilar($1)` |
| `factor : CTE` | nada | `apilar($1)` |
| `factor : ( expresion )` | nada | `$$ = $2` |
| `factor : invocacion` | nada | `$$ = $1` |

`*` y `/` se agregan al reducir `termino`, antes de que `expresion` agregue `+` o `-`. Los paréntesis dejan en la pila el operando de adentro.

### 6.3 Asignación, salida y retorno

| Regla | Generación |
| --- | --- |
| `ID = expresion ;` | `colocar(T_ASIG, apilar($1), $3, TIPO_NINGUNO)` |
| `write ( expresion ) ;` | `colocar(T_WRITE, $3, sin_operando(), TIPO_NINGUNO)` |
| `write ( cadena ) ;` | `colocar(T_WRITE_STR, apilar($3), sin_operando(), TIPO_NINGUNO)` |
| `return expresion ;` | `colocar(T_ASIG, apilar(@ret_<funcion_actual>), $2, TIPO_NINGUNO)` y después `colocar(T_RET, sin_operando(), sin_operando(), TIPO_NINGUNO)` |

`T_RET` no transporta el racional. El valor de retorno de R11 queda en el `ASIG` previo.

`sentencia : invocacion ;` descarta la referencia. Los tercetos de la llamada ya se agregaron al reducir `invocacion`.

### 6.4 Invocación

`invocacion : ID ( expresion )` crea `@tmpN` en la tabla, de tipo racional, y apila `apilar` de esa celda. `@ret_<funcion>` es una sola celda: el temporal conserva el retorno antes de otra llamada a la misma función. El argumento se traduce al reducir `expresion`, antes de estos tres tercetos.

| Orden | Función ya registrada | Función todavía no vista |
| --- | --- | --- |
| 1 | `colocar(T_ASIG, apilar(parámetro), argumento, TIPO_NINGUNO)` | `colocar(T_ASIG, sin_operando(), argumento, TIPO_NINGUNO)` |
| 2 | `crear_terceto(T_CALL, sin_operando(), sin_operando(), TIPO_NINGUNO)` y `completar(call, entrada)` | `crear_terceto(T_CALL, sin_operando(), sin_operando(), TIPO_NINGUNO)` |
| 3 | `colocar(T_ASIG, apilar(@tmpN), apilar(@ret_), TIPO_NINGUNO)` | `colocar(T_ASIG, apilar(@tmpN), sin_operando(), TIPO_NINGUNO)` |

El segundo caso anota los tres índices en `pendientes`. El encabezado de §6.1 los completa. Una llamada que está dentro del cuerpo ya tiene la entrada, porque el encabezado se registró antes.

### 6.5 Comparación

`condicion : expresion op_comparacion expresion` agrega dos tercetos:

1. `cmp = colocar(op, izquierda, derecha, TIPO_ENTERO)`.
2. `crear_terceto(T_BF, apilar_ti(cmp), sin_operando(), TIPO_NINGUNO)`.

Apila un `Cond` con ese `BF` en `falso`. `verdadero` queda vacío: si la comparación es verdadera, la ejecución sigue en el terceto siguiente.

`condicion : ( condiciones )` apila `$$ = $2`.

### 6.6 `and` y `or`

`and` y `or` no tienen operador propio. La regla es `condiciones : condiciones op_logico condicion`, con una acción intermedia entre el operador y la condición derecha.

| Operador | Antes de la derecha |
| --- | --- |
| `and` | `completar` de cada `BI` verdadero de la izquierda con `nro_terceto()`, y vacía esa lista. Los `BF` falsos siguen abiertos |
| `or` | `crear_terceto(T_BI, sin_operando(), sin_operando(), TIPO_NINGUNO)`, lo agrega a los verdaderos, `completar` de los `BF` falsos de la izquierda con `nro_terceto()` y vacía esa lista |

Al juntar:

| Operador | `Cond` resultante |
| --- | --- |
| `and` | Falsos de la izquierda más falsos de la derecha. Verdaderos: los de la derecha |
| `or` | Falsos de la derecha. Verdaderos de la derecha más el `BI` de la izquierda |

En un `and`, si la izquierda es falsa el `BF` no entra en la derecha. En un `or`, si la izquierda es verdadera el `BI` salta la derecha; si es falsa, el `BF` entra en ella.

`if` y `until` completan los saltos que armó la condición. No agregan otro `BF`.

### 6.7 `if` y `else`

| Momento | Generación |
| --- | --- |
| Después de `if ( condiciones )`, antes del bloque | `completar` de los verdaderos con `nro_terceto()`. `apilar_nro(-1)`. Guarda el `Cond` con los falsos abiertos |
| Al ver `else`, antes de su bloque | `desapilar` de la marca `-1`. `crear_terceto(T_BI, sin_operando(), sin_operando(), TIPO_NINGUNO)`. `completar` de los falsos con `nro_terceto()`. `apilar_nro` del índice de ese `BI` |
| Al cerrar el `if` con `else` | `desapilar` del `BI` y `completar(bi, nro_terceto())` |
| Al cerrar el `if` sin `else` | `desapilar` devuelve `-1`. `completar` de los falsos con `nro_terceto()` |

Cada `if` anidado guarda su propio `Cond` y su propio número en la pila. Si el destino es el fin del programa, el índice es el que tendría el próximo terceto. No hay una fila con ese número.

### 6.8 `loop … until`

El cuerpo se ejecuta y después se evalúa la condición. Si es falsa, la ejecución vuelve al primer terceto del cuerpo. Si es verdadera, sigue en el terceto que está después del ciclo.

| Momento | Generación |
| --- | --- |
| Al ver `LOOP`, antes del bloque | `apilar_nro(nro_terceto())`. Ese número es el primer terceto del cuerpo |
| Al cerrar `until ( condiciones )` | `inicio = desapilar()`. `completar` de los verdaderos con `nro_terceto()`. `completar` de los falsos con `inicio` |

El `BF` lo agrega la comparación. `until` le escribe el inicio del cuerpo. Un `loop` anidado apila su propio inicio. Esa pila es la misma de §3.2: un `if` interior desapila su marca antes de que el `until` exterior desapile el inicio.

---

## 7. Formato de `out/tercetos.txt`

Una línea por terceto. `main` lo escribe al terminar `yyparse`, después del veredicto y de `out/ts.txt`.

```
#     OPERADOR      OP1                  OP2                  TIPO       LINEA
0     BI            DIR:1                -                    -          1
1     ASIG          TS:a                 TS:1/2               -          5
2     ASIG          TS:b                 TS:1/3               -          6
3     SUMA          TS:a                 TS:b                 RACIONAL   7
4     ASIG          TS:c                 TER:3                -          7
5     WRITE_STR     TS:Resultado         -                    -          8
6     WRITE         TS:c                 -                    -          9
```

| Clase o tipo | Texto |
| --- | --- |
| `REF_TS` | `TS:` y el nombre del símbolo |
| `REF_TER` | `TER:` y el índice |
| `REF_DIR` | `DIR:` y el índice |
| `REF_VACIO` | `-` |
| `TIPO_RACIONAL` | `RACIONAL` |
| `TIPO_ENTERO` | `ENTERO` |
| `TIPO_NINGUNO` | `-` |

Un índice de tabla fuera de rango se imprime `TS:?`. Las columnas quedan alineadas a la izquierda: índice en 6, operador en 14, cada operando en 21, tipo en 11 y la línea al final. Si el archivo no se puede crear, se informa `Error al crear archivo de tercetos.`

---

## 8. Compilación

```
bison -d --report=all -o src/parser/parser.tab.c src/parser/parser.y
gcc -Wall -Wextra -o compilador src/parser/parser.tab.c src/lexico/lexico.c src/intermedio/tercetos.c
```

---

## 9. Aceptación

- Bison genera el parser sin conflictos.
- Compila sin warnings con `-Wall -Wextra`.
- Los programas de `tests/correctos/` terminan en compilación exitosa.
- Los programas con error léxico o sintáctico terminan en compilación con errores. `em1_declaracion` y `em2_recursion` compilan: E6, E7 y E9 están fuera de alcance.
- Ningún `BF`, `BI` ni `CALL` queda con destino vacío. Un destino igual a la cantidad de tercetos es el lugar que sigue al último.

`tests/correctos/p1_suma.rg` produce el listado de §7. `var` no aparece. El `BI` apunta a 1 porque no hay funciones delante de `main`. `c = a + b` agrega primero la `SUMA` y después el `ASIG` de `c` con `TER` de esa suma.

| Programa | Resultado |
| --- | --- |
| `p2_normalizar` | `2/4` queda `TS:1/2`, porque el léxico ya normalizó la constante |
| `p3_loop_until` | El `BF` del `until` vuelve al `SUMA` de `acc + n` |
| `p4_if_else` | El `and` deja dos `BF` hacia el `else`. El `or` tiene un `BI` que salta la comparación derecha, y el `BF` de la izquierda entra en ella |
| `p5_funcion` | El `BI` inicial va al primer terceto de `main`. `CALL` entra a `cuadrado`, que quedó delante. `return x * x` es `MUL`, `ASIG` a `@ret_cuadrado` y `RET`. Cada llamada copia el retorno a un `@tmpN` distinto |
| `p6_overflow` | `1/1 / n` es `DIV` antes que la `SUMA`. El `BF` vuelve a ese `DIV` |
| `p7_condiciones` | Los dos `BF` del `and` apuntan al índice que sigue al `WRITE` |
| `p8_expresiones` | `*` y `/` quedan antes que `+` y `-`. Los paréntesis no agregan terceto |
