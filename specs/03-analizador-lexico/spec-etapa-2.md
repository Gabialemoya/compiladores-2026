# Spec — Analizador léxico de RG (etapa 2: `yylex`)

**Depende de:** `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md` · `[spec-etapa-1.md](spec-etapa-1.md)`
**Produce:** `src/lexico/`

Convierte el AL de la etapa 1 en la función `int yylex(void)` que invoca el analizador sintáctico.

---

## 1. Alcance e interfaz

### 1.1 Objetivo

En la etapa 1 el AL controlaba el recorrido: abría el fuente y lo consumía entero en un bucle propio. En esta etapa **deja de controlar el recorrido**: el analizador sintáctico llama a `yylex()` y recibe **un token por llamada**.

El autómata no cambia. Cambia quién lo invoca.

| | Etapa 1 | Etapa 2 |
| --- | --- | --- |
| Controla el bucle | El AL | `yyparse` |
| Entrada del programa | `main` del AL | `yyparse`, desde el `main` del parser |
| Salida | Listado de tokens | Un código de token por llamada |
| Fin de archivo | Termina el listado | Retorna `0` |

### 1.2 Entrada

- El fuente ya abierto por el `main` del parser, antes de la primera llamada.
- Fallo de apertura: error de I/O, no error léxico; no se llama a `yyparse`.

### 1.3 Salida

1. **Valor de retorno**: el código de token que espera el parser (§3).
2. **`yylval`** cargado para `ID`, `CTE` y `CADENA` (§4).
3. **Informe de errores léxicos** E1–E4, sin abortar (§9).
4. **Listado de tokens** en `out/`, conservado para contrastar contra la etapa 1.
5. **Tabla de símbolos** exportada al terminar `yyparse` (contrato de spec 02).

### 1.4 Fuera de alcance

- Reglas gramaticales, acciones del parser y resolución de conflictos.
- Semántica (R1, R2, R7).
- Todo lo ya especificado en `[spec-etapa-1.md](spec-etapa-1.md)`: eventos, estados, matrices y acciones semánticas se reutilizan sin cambios.

### 1.5 Responsabilidades del reconocedor

- Reconocer **un** token por llamada y retornarlo.
- Retornar `0` al llegar a EOF.
- Ante error léxico: informar, recuperar y **seguir buscando** un token válido. Nunca retornar un código que no sea un token del lenguaje.
- Proveer `yyerror` al parser.
- Mantener accesible el contador de línea.

---

## 2. Decisiones propias de esta fase

| # | Decisión | Valor |
| --- | --- | --- |
| Y1 | Códigos de token | Los declara el generador. `yylex` retorna las constantes de `parser.tab.h` |
| Y2 | Códigos de `01-diseno` §4 | Se conservan como numeración del lenguaje: documentación, listado y campo `TIPO` de la TS |
| Y3 | Puente entre ambas | Una única función `traducir_token` (§3) |
| Y4 | Valor semántico | Índice en la tabla de símbolos, no el lexema |
| Y5 | Error léxico | `continue`, nunca `return`. El parser no recibe códigos de error |
| Y6 | Unread ante error | Se agrega: el carácter que provoca el error vuelve al flujo (§8) |
| Y7 | Normalización de racionales | En `fin_cte`, antes del alta en TS (§6) |
| Y8 | Cadenas | Sin cambios: solo letras, según L4 y §10 de la etapa 1 |
| Y9 | Errores derivados | Se informan ambos: el léxico y el sintáctico que provoca (§13, a consultar con la cátedra) |

---

## 3. Códigos de token

Los valores `1`–`255` están reservados para caracteres literales y el `0` para el fin de entrada, así que los códigos de `01-diseno` §4 (10 a 300) no sirven como valor de retorno.

Declarar los tokens en la gramática sin número y retornar las constantes generadas:

```
#include "parser.tab.h"
...
return traducir_token(token_actual);
```

`traducir_token` es el **único** lugar del programa donde conviven las dos numeraciones:

```
int traducir_token(int codigo_interno):
    10  -> ID
    20  -> CTE
    30  -> OP_ASIG
    ...
    300 -> CADENA
```

Fin de entrada: `yylex` retorna `0`. **No** existe un token `EOF` en el lenguaje.

---

## 4. Valor semántico (`yylval`)

El código dice **qué** se reconoció; `yylval` dice **cuál**.

| Token | `yylval` |
| --- | --- |
| `ID` | Índice en TS |
| `CTE` | Índice en TS |
| `CADENA` | Índice en TS |
| Resto | No se carga |

Declaración en la gramática:

```
%union { int indice_ts; }
%token <indice_ts> ID CTE CADENA
```

Cargar `yylval` dentro de `fin_id`, `fin_cte` y `fin_cadena`, antes de retornar.

**Cambio requerido en spec 02:** las altas en TS son `void`. Devolver el índice de la entrada, tanto si se inserta como si ya existía.

---

## 5. Cambios sobre la etapa 1

| Elemento | Qué pasa |
| --- | --- |
| `nuevo_estado`, `proceso`, matriz de tokens | Sin cambios |
| `get_evento` | Sin cambios |
| Acciones de acumulación | Sin cambios |
| `fin_id`, `fin_cte`, `fin_cadena` | Cargan `yylval` |
| `fin_cte` | Suma la normalización de §6 |
| `reconocer_token()` | Pasa a ser el cuerpo de `yylex()` |
| `return 1` al cerrar token | Pasa a `return traducir_token(...)` |
| `return 0` en EOF | Se mantiene: es lo que espera el parser |
| `main` del AL | Se elimina. El `main` pasa al parser |
| Apertura del fuente | Se traslada al `main` del parser |
| — | Se agrega `yyerror` (§9) |

---

## 6. Normalización de racionales

`fin_cte` registra la constante tal como fue escrita, de modo que `2/4` y `1/2` generan dos entradas de TS para el mismo valor. D9 y el caso F.6 exigen mostrar los resultados simplificados.

Aplicar en `fin_cte`, después de validar el denominador y antes del alta:

1. **Signo.** Si el denominador es negativo, invertir el signo de ambos.
2. **Simplificación.** Dividir ambos por su máximo común divisor (Euclides).

| Entrada | En TS |
| --- | --- |
| `2/4` | `1/2` |
| `0/5` | `0/1` |
| `3/-6` | `-1/2` |

La TS guarda la forma normalizada, no el lexema original.

---

## 7. Pseudocódigo

```
yylex():
    estado = 0
    mientras verdadero
        leer(c)
        si EOF
            si estado == 0 o estado == 10: retornar 0
            columna = SPACE            /* fuerza el cierre del último token */
        si no
            columna = get_evento(c)
            si columna inválida
                informar E1 con línea
                estado = 0; continuar   /* NO retornar */
        (*proceso[estado][columna])()
        nuevo = nuevo_estado[estado][columna]
        si nuevo == -2
            informar el error según el estado
            unget(c)                    /* §8 */
            estado = 0; continuar       /* NO retornar */
        si nuevo == -1
            si corresponde unread: unget(c)
            si el token se canceló: estado = 0; continuar
            cargar yylval si el token lo requiere
            agregar {token} al listado
            retornar traducir_token(token)
        estado = nuevo
```

Conductor:

```
main:
    si fopen del fuente falla: error de apertura; salir
    yyparse()
    fclose
    si no hubo errores
        printf(" - Compilacion EXITOSA - ")
    si no
        printf(" - Compilacion completa con ERRORES - ")
    mostrarTS()
```

Los tres `continuar` son la diferencia estructural con la etapa 1: ante un error, `yylex` **no retorna**.

---

## 8. Unreads

Se mantiene el criterio de `01-diseno` §10, con las dos excepciones conocidas:

| Caso | Unread |
| --- | --- |
| Cierre por columna "otro" | Sí |
| Comilla de cierre (estado 21, col. 14) | No: pertenece al lexema |
| EOL del comentario (estado 10) | No: se consume como cierre |
| EOF | No |

**Corrección respecto de la etapa 1.** La rama `-2` no hacía unread: el carácter que provocaba el error quedaba consumido. Con `n = 0;` el listado salía `ID OP_ASIG WRITE`, sin el `PUNTO_Y_COMA`.

En la etapa 1 eso solo ensuciaba el listado. Con el parser en línea, el sintáctico recibe una sentencia sin terminar y emite un E5 que el fuente no contiene: **una causa, dos errores**. Agregar el unread en la rama `-2`, decrementando el contador de línea si el carácter es EOL.

No genera bucle: el carácter se relee desde el estado 0, y la fila 0 no tiene celdas `-2`.

---

## 9. Errores

### 9.1 Léxicos

Los mismos de la etapa 1, sin agregados:

| Código | Condición |
| --- | --- |
| E1 | `get_evento` sin columna, o carácter inválido en estado 0 |
| E2 | Estado 1 sin `/`, o estado 2 sin dígito |
| E3 | `fin_cte` con denominador 0 |
| E4 | Estado 21 y no-letra (salvo `"`) |

Un error no entrega token al parser ni agrega token ficticio al listado.

### 9.2 Sintácticos

`yyerror(const char* msg)` la provee esta fase; la invoca el parser. Informa **E5** (`01-diseno` §12) con la línea, y levanta la bandera de error. El contador de línea ya es global, así que no requiere trabajo adicional.

Ninguno de los dos aborta la compilación (consigna general 14).

---

## 10. Estructura y compilación

```
src/lexico/lexico.h     prototipos de yylex, yyerror y altas en TS
src/lexico/lexico.c     matrices, acciones semánticas, yylex
src/parser/parser.y     gramática, %token, %union y main
```

```
bison -d parser.y          genera parser.tab.c y parser.tab.h
gcc parser.tab.c lexico.c -o compilador
```

`lexico.c` incluye `parser.tab.h`. La dependencia es en un solo sentido: el léxico conoce los códigos del parser; el parser no conoce el interior del léxico.

Rutas de salida relativas (`out/`). El código de la etapa 1 tiene rutas absolutas que impiden compilarlo en otro equipo.

---

## 11. Casos de prueba de esta fase

| Caso | Qué verifica |
| --- | --- |
| `yylex` en bucle sobre P1–P6 | Listado **idéntico** al de la etapa 1 |
| Último token sin blanco final | No se pierde: EOF fuerza el cierre |
| `n = 0;` | E2 y `PUNTO_Y_COMA` presente en el listado (§8) |
| `@` en medio de una expresión | E1 y `yylex` sigue entregando tokens |
| `2/4` | En TS figura `1/2` |
| `0/5` | En TS figura `0/1` |
| Un `ID` usado dos veces | Mismo índice en `yylval` |
| Integración con `yyparse` | El fuente se consume entero: `yylex` llega a retornar `0` |

La primera es la más importante: verifica que la adaptación no alteró el reconocimiento.

---

## 12. Criterios de aceptación

- `yylex` retorna un token por llamada y `0` en EOF; nunca un código de error.
- El listado de tokens de los seis programas correctos coincide con el de la etapa 1.
- `yylval` llega cargado en las reglas que usan `ID`, `CTE` y `CADENA`.
- Las constantes figuran normalizadas en la TS.
- Un error léxico no interrumpe `yyparse` ni hace perder caracteres; el error sintáctico derivado que igual se produzca se informa (§13).
- Las dos numeraciones conviven solo dentro de `traducir_token`.

---

## 13. Errores derivados — decisión a consultar con la cátedra

> **A consultar con el profesor antes de la entrega.** La decisión está tomada y es la que implementa esta spec, pero conviene validar el criterio.

**Decisión:** informar **ambos** errores. No se suprime el error sintáctico derivado de un error léxico.

Un único error en el fuente puede producir dos mensajes. Con `n = 0;`, el léxico informa E2 y entrega al parser la secuencia `ID OP_ASIG PUNTO_Y_COMA`, que es una sentencia sin operando derecho, de modo que el parser informa además E5:

```
Linea 3: Error lexico E2: constante mal formada
Linea 3: Error sintactico E5: sentencia mal formada
```

El E5 no está en el programa fuente: aparece porque el léxico no pudo entregar el `CTE`. El unread de §8 acota el daño a esa sentencia, pero no lo elimina, porque el operando sigue faltando.

| | Se informan ambos (elegido) | Se suprime el derivado |
| --- | --- | --- |
| Qué ve el usuario | Dos mensajes por un error | Un mensaje |
| Fidelidad | Refleja lo que ocurrió en cada fase | Oculta la reacción del parser |
| Riesgo | El usuario debe inferir que el segundo es consecuencia | Se pierde un error sintáctico real e independiente en esa línea |
| Implementación | Ninguna | Registrar la última línea con error léxico y compararla en `yyerror` |

**Motivo de la elección:** suprimir por línea descartaría también errores sintácticos genuinos que compartan línea con un error léxico, y la consigna general 14 pide registrar los errores y continuar, no filtrarlos.

**Qué consultar:** si la cátedra espera que el informe muestre un error por causa o un error por fase. Si la respuesta es la primera, aplicar el filtro de la columna derecha, que es un cambio acotado a `yyerror`.

Registrar la respuesta en `bitacora.md` de esta carpeta.