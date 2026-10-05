# Spec — Analizador sintáctico de RG

**Depende de:** `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md` · `specs/03-analizador-lexico/spec-etapa-2.md`
**Produce:** `src/parser/`

Integra el analizador léxico con Bison y valida que el programa fuente pertenezca al lenguaje RG.

---

## 1. Alcance e interfaz

### 1.1 Objetivo

Reconocer la estructura del programa fuente aplicando las reglas gramaticales de `01-diseno` §9. El parser pide tokens a `yylex` (spec-etapa-2) y reduce hasta el axioma. Si no llega al axioma, el programa no pertenece al lenguaje.

### 1.2 Entrada

- Los tokens que entrega `yylex`, uno por llamada, hasta recibir `0`.
- El valor semántico en `yylval` para `ID`, `CTE` y `CADENA`.

### 1.3 Salida

1. **Veredicto**: el programa pertenece o no al lenguaje.
2. **Errores sintácticos** reportados por `yyerror`, con la línea del fuente.
3. **Punto de enganche** para las acciones semánticas de etapas posteriores.

### 1.4 Fuera de alcance

| Qué | Dónde se especifica |
| --- | --- |
| Catálogo y recuperación de errores | `specs/05-errores/` |
| Generación de tercetos en cada regla | `specs/06-codigo-intermedio/` |
| Controles semánticos R1, R2, R7 | `specs/05-errores/` |
| Reconocimiento de tokens | `specs/03-analizador-lexico/` |

Las acciones semánticas se escriben físicamente dentro de las reglas de esta spec, pero **su contenido** lo define la spec 06. Esta spec define el esqueleto; la 06 lo llena.

### 1.5 Responsabilidades

- Declarar los tokens de forma que Bison genere los códigos que `yylex` retorna.
- Declarar el valor semántico.
- Expresar las 25 producciones de `01-diseno` §9 sin alterarlas.
- Proveer el `main` que invoca `yyparse`.
- Dejar el parser libre de conflictos, o con los conflictos documentados y justificados.

---

## 2. Decisiones propias de esta fase

| # | Decisión | Valor |
| --- | --- | --- |
| S1 | Herramienta | Bison. Consigna general 1 exige integración con YACC |
| S2 | Lenguaje de las acciones | C. Consigna general 2 |
| S3 | Generador de léxico | **Ninguno.** No se usa Flex: `yylex` es manual (spec-etapa-2, Y1) |
| S4 | Códigos de token | Los asigna Bison. `yylex` traduce con `traducir_token` (spec-etapa-2 §3) |
| S5 | Valor semántico | `%union` con un entero: índice en la tabla de símbolos (spec-etapa-2, Y4) |
| S6 | Precedencia y asociatividad | **No se declaran.** Quedan codificadas en la jerarquía `expresion` → `termino` → `factor` |
| S7 | Axioma | `programa` |
| S8 | Mensajes de error | Modo detallado de Bison, para que `yyerror` reciba qué se esperaba |

**Sobre S6.** Al resolver el signo negativo en el estado 7 del autómata, una constante negativa llega como un único `CTE` y no existe la regla `factor: OP_RESTA factor`. Por eso no hace falta declarar precedencia para el menos unario.

---

## 3. Estructura del archivo `parser.y`

| Sección | Contenido en RG |
| --- | --- |
| Declaraciones | `#include` de la tabla de símbolos y del léxico, prototipos de `yylex` y `yyerror`, variables globales |
| Tokens | `%union`, `%token`, `%start` |
| Reglas | Las 25 producciones de `01-diseno` §9 |
| Código auxiliar | `yyerror`, `main`, y las funciones que invoquen las acciones de la spec 06 |

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

Son 29 tokens. **`READ` no se declara:** fue retirado de la gramática, de modo que `read` se reconoce como identificador común. Registrarlo en la bitácora de `01-diseno` para que la tabla de tokens quede coherente.

Declarar un token que ninguna regla usa produce advertencia de Bison, así que la lista de arriba y la gramática de §5 deben coincidir exactamente.

---

## 5. Gramática

Traducción literal de `01-diseno` §9 a la sintaxis de Bison. **Ninguna regla se modifica, reordena ni corrige.**

```
programa            : lista_funciones funcion_main
                    | funcion_main
                    ;

lista_funciones     : lista_funciones definicion_funcion
                    | definicion_funcion
                    ;

definicion_funcion  : ID PAR_ABRE ID PAR_CIERRA cuerpo
                    ;

funcion_main        : MAIN PAR_ABRE PAR_CIERRA cuerpo
                    ;

cuerpo              : LLAVE_ABRE declaraciones lista_sentencias LLAVE_CIERRA
                    ;

declaraciones       : declaraciones declaracion
                    | %empty
                    ;

declaracion         : VAR ID PUNTO_Y_COMA
                    ;

lista_sentencias    : lista_sentencias sentencia
                    | sentencia
                    ;

sentencia           : asignacion
                    | seleccion
                    | iteracion
                    | salida
                    | retorno
                    | invocacion PUNTO_Y_COMA
                    ;

asignacion          : ID OP_ASIG expresion PUNTO_Y_COMA
                    ;

retorno             : RETURN expresion PUNTO_Y_COMA
                    ;

salida              : WRITE PAR_ABRE expresion PAR_CIERRA PUNTO_Y_COMA
                    | WRITE PAR_ABRE CADENA PAR_CIERRA PUNTO_Y_COMA
                    ;

seleccion           : IF PAR_ABRE condiciones PAR_CIERRA bloque
                    | IF PAR_ABRE condiciones PAR_CIERRA bloque ELSE bloque
                    ;

iteracion           : LOOP bloque UNTIL PAR_ABRE condiciones PAR_CIERRA PUNTO_Y_COMA
                    ;

bloque              : LLAVE_ABRE lista_sentencias LLAVE_CIERRA
                    ;

condiciones         : condiciones op_logico condicion
                    | condicion
                    ;

condicion           : expresion op_comparacion expresion
                    | PAR_ABRE condiciones PAR_CIERRA
                    ;

expresion           : expresion OP_SUMA termino
                    | expresion OP_RESTA termino
                    | termino
                    ;

termino             : termino OP_MUL factor
                    | termino OP_DIV factor
                    | factor
                    ;

factor              : ID
                    | CTE
                    | invocacion
                    | PAR_ABRE expresion PAR_CIERRA
                    ;

invocacion          : ID PAR_ABRE expresion PAR_CIERRA
                    ;

op_comparacion      : OP_MENOR
                    | OP_MENOR_IGUAL
                    | OP_MAYOR
                    | OP_MAYOR_IGUAL
                    | OP_IGUAL
                    | OP_DISTINTO
                    ;

op_logico           : AND
                    | OR
                    ;
```

### 5.1 Notas de traducción

| Punto | Nota |
| --- | --- |
| `%empty` | Es la forma en que Bison escribe la producción vacía de `declaraciones`. El comentario `/* vacio */` del documento de diseño no es sintaxis |
| `main` contra funciones | `funcion_main` usa el terminal `MAIN` y `definicion_funcion` usa `ID`, de modo que no se superponen |
| `ID` seguido de `(` | Va a `invocacion`; `ID` seguido de cualquier otra cosa reduce a `factor`. Un símbolo de anticipación alcanza |
| Llaves obligatorias | Todos los cuerpos son `bloque` con llaves, así que no aparece el problema del `else` colgante |
| `CADENA` | Solo aparece en `salida`, nunca como `factor`. Así `"texto" + 1/1` lo rechaza la sintaxis |
| Declaraciones | Solo en `cuerpo`, no en `bloque`: se declaran al inicio de cada función, nunca dentro de un `if` o un `loop` |

---

## 6. Conflictos: qué revisar

Generar siempre el informe y revisarlo antes de dar la etapa por cerrada:

```
bison -d --report=all -Wcounterexamples parser.y
```

El archivo `parser.output` lista los estados con sus reglas punteadas. Un conflicto aparece como una celda con dos acciones posibles, igual que en el ejemplo de construcción manual de la tabla que entregó la cátedra.

### 6.1 Punto a inspeccionar primero

```
condicion : PAR_ABRE condiciones PAR_CIERRA
factor    : PAR_ABRE expresion  PAR_CIERRA
```

Al leer un `(` dentro de una condición, el parser no sabe todavía si lo que sigue es otra condición o una expresión. Analíticamente se resuelve, porque una condición exige un operador de comparación y una expresión no lo admite, pero es el lugar donde el método LALR puede fusionar estados y reportar conflicto.

**Si aparece**, antes de tocar la gramática verificar que el conflicto sea real y no una advertencia por estados fusionados. Toda modificación a las reglas debe reflejarse primero en `01-diseno` §9 y registrarse en la bitácora: las especificaciones son la fuente de la verdad.

### 6.2 Criterio

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

El `main` deja de recorrer el archivo: esa responsabilidad pasó a `yyparse`. La apertura del fuente ocurre acá, antes de la primera llamada a `yylex`.

`yyerror` la provee el léxico (spec-etapa-2 §9.2) e informa **E5** con la línea.

---

## 8. Estructura y compilación

```
src/parser/parser.y        gramática, tokens, acciones y main
src/lexico/lexico.c        yylex, matrices y acciones semánticas del AL
src/lexico/lexico.h        prototipos
```

```
bison -d --report=all src/parser/parser.y
gcc parser.tab.c src/lexico/lexico.c -o compilador
```

`lexico.c` incluye `parser.tab.h` para conocer los códigos de token. La dependencia es en un solo sentido.

**No enlazar Flex.** Generaría su propio `yylex` y pisaría el del grupo (S3).

---

## 9. Casos de prueba de esta fase

### 9.1 Programas correctos

Los seis de `tests/correctos/` deben parsear completos, llegando al axioma y sin errores.

| Caso | Qué ejercita de la gramática |
| --- | --- |
| `p1_suma` | Asignaciones, `write` de expresión y de literal |
| `p2_normalizar` | Constantes y salida |
| `p3_loop_until` | `iteracion` con condición al final |
| `p4_if_else` | `seleccion` con `else`, `condiciones` con `and`, `or` y paréntesis |
| `p5_funcion` | `definicion_funcion`, `invocacion` como sentencia y como factor, `retorno` |
| `p6_overflow` | Iteración con acumulación |

### 9.2 Programas con error sintáctico

`tests/errores-lexicos-sintacticos/es1_sentencia.rg` debe ser rechazado, con la línea correcta informada.

Verificar además que los programas con **error léxico** (`el1_caracter`, `el2_constante`) no interrumpan el parsing: el léxico informa y sigue entregando tokens.

### 9.3 Prueba de cobertura

Toda producción de §5 debe quedar ejercitada por al menos un caso. Si alguna no se alcanza nunca, agregar un programa que la cubra o justificar por qué no es alcanzable.

---

## 10. Criterios de aceptación

- `bison -d` genera el parser sin conflictos, o con los conflictos documentados en la bitácora.
- La gramática del `parser.y` coincide producción por producción con `01-diseno` §9.
- Los 29 tokens declarados coinciden con los que retorna `traducir_token`; ninguno queda sin usar.
- Los seis programas correctos parsean completos.
- Los programas con error sintáctico se rechazan informando la línea.
- Un error léxico no aborta el parsing.
- No se enlaza Flex.

---

## 11. Remisiones

| Tema | Spec |
| --- | --- |
| Qué emite cada regla | `06-codigo-intermedio` |
| Catálogo de errores y recuperación | `05-errores` |
| Formato del terceto y operadores | `06-codigo-intermedio` |
| Contrato de `yylex` y `yylval` | `03-analizador-lexico/spec-etapa-2.md` |
| Gramática original y catálogo de errores | `01-diseno/spec.md` §9 y §12 |