# Spec — Diseño del lenguaje RG

**Grupo:** F · **Lenguaje de implementación:** C
**Estado:** Entrega 1.

---

## 1. Decisiones globales

| # | Decisión | Valor |
|---|---|---|
| D1 | Tipo de datos | Un solo tipo: racional, par ordenado (numerador, denominador) de enteros con signo de 32 bits cada uno |
| D2 | Rango de numerador y denominador | −2.147.483.648 a 2.147.483.647 |
| D3 | Literales | Solo forma `numerador/denominador` (ej. `3/4`, `0/1`, `4/1`). Un entero suelto es error léxico E2 |
| D4 | Forma normal | Tras cada operación: simplificar por MCD; denominador siempre positivo (signo en el numerador); cero como `0/1` |
| D5 | Denominador cero en un literal | Error de compilación |
| D6 | Denominador cero producido en runtime | Error de ejecución, con mensaje y línea de la operación |
| D7 | Operaciones aritméticas | `+ - * /` sobre racionales, con simplificación posterior obligatoria (D4) |
| D8 | Comparaciones | Exactas, sin punto flotante (producto cruzado, cuidando el signo). Resultado: `-1` verdadero, `0` falso. Solo resuelve una `<condicion>` |
| D9 | Overflow | No se cancela. Dividir numerador y denominador por la misma potencia de 2 (redondeo del numerador) hasta entrar en D2, volver a simplificar por MCD. Advertencia con la línea afectada |
| D10 | Declaración | Obligatoria con `var`, en el `cuerpo` de la función, antes de las sentencias |
| D11 | Alcance | Único y global: no hay variables locales |
| D12 | Funciones | Sin variables locales, memoria estática. Un parámetro racional por valor, dirección fija en compilación. Devuelven racional con `return` |
| D13 | Recursión | Prohibida la llamada recursiva directa. Error semántico |
| D14 | Identificadores | Letras `a`–`z` y `A`–`Z`. El lexema se guarda tal cual |
| D15 | Comentarios | De línea, `//` hasta fin de línea; el léxico los descarta |
| D16 | Programa principal | Función de nombre reservado `main`, sin parámetros |
| D17 | Plataforma destino | — |
| D18 | Salida | `write` de una expresión racional (formato `numerador/denominador` ya simplificado) o de un literal de texto |

---

## 2. Alfabeto

| Clase | Caracteres |
|---|---|
| `LETRA` | `a`–`z`, `A`–`Z` |
| `DIGITO` | `0`–`9` |
| símbolos | `= + - * / < > ; ( ) { } "` |
| `SPACE` | espacio |
| `TAB` | tabulador |
| `EOL` | salto de línea |
| `OTRO` | cualquier otro carácter → error léxico E1 |

Columnas de las matrices: `LETRA`, `DIGITO`, `=`, `+`, `-`, `*`, `/`, `<`, `>`, `;`, `(`, `)`, `{`, `}`, `"`, `SPACE`, `TAB`, `EOL`.

---

## 3. Palabras reservadas

`if` · `else` · `loop` · `until` · `var` · `or` · `and` · `write` · `main` · `return`

Se reconocen como identificadores y se resuelven por búsqueda en tabla (`fin_id`), no con estados propios del autómata.

---

## 4. Tabla de tokens

| Código | Token | Lexema |
|---|---|---|
| 10 | `ID` | identificador |
| 20 | `CTE` | constante racional `numerador/denominador` (ej. `3/4`, `4/1`) |
| 30 | `OP_ASIG` | `=` |
| 40 | `OP_IGUAL` | `==` |
| 50 | `OP_SUMA` | `+` |
| 60 | `OP_RESTA` | `-` |
| 70 | `OP_MUL` | `*` |
| 80 | `OP_DIV` | `/` |
| 90 | `OP_MENOR` | `<` |
| 100 | `OP_MENOR_IGUAL` | `<=` |
| 110 | `OP_MAYOR` | `>` |
| 120 | `OP_MAYOR_IGUAL` | `>=` |
| 130 | `OP_DISTINTO` | `<>` |
| 140 | `PUNTO_Y_COMA` | `;` |
| 150 | `PAR_ABRE` | `(` |
| 160 | `PAR_CIERRA` | `)` |
| 170 | `LLAVE_ABRE` | `{` |
| 180 | `LLAVE_CIERRA` | `}` |
| 190 | `IF` | `if` |
| 200 | `ELSE` | `else` |
| 210 | `VAR` | `var` |
| 220 | `LOOP` | `loop` |
| 230 | `UNTIL` | `until` |
| 240 | `OR` | `or` |
| 250 | `AND` | `and` |
| 270 | `WRITE` | `write` |
| 280 | `MAIN` | `main` |
| 290 | `RETURN` | `return` |
| 300 | `CADENA` | `"…"` |

---

## 5. Estructura del programa

Un programa RG es un conjunto de funciones. Una de ellas tiene el nombre reservado `main`, sin parámetros, y es el punto de entrada.

Las funciones de usuario tienen exactamente un parámetro racional por valor, en dirección estática fija, y devuelven un racional con `return`. Todas comparten el único espacio global: no hay locales.

Estructuras de control: `if` con `else`; `loop … until` (condición al final, el cuerpo corre al menos una vez); anidamiento arbitrario; condiciones con comparaciones ligadas por `and` y `or`, con paréntesis.

---

## 6. Gramática

```
programa            : lista_funciones funcion_main
                    | funcion_main

lista_funciones     : lista_funciones definicion_funcion
                    | definicion_funcion

definicion_funcion  : ID PAR_ABRE ID PAR_CIERRA cuerpo

funcion_main        : MAIN PAR_ABRE PAR_CIERRA cuerpo

cuerpo              : LLAVE_ABRE declaraciones lista_sentencias LLAVE_CIERRA

declaraciones       : declaraciones declaracion
                    | /* vacio */

declaracion         : VAR ID PUNTO_Y_COMA

lista_sentencias    : lista_sentencias sentencia
                    | sentencia

sentencia           : asignacion
                    | seleccion
                    | iteracion
                    | salida
                    | retorno
                    | invocacion PUNTO_Y_COMA

asignacion          : ID OP_ASIG expresion PUNTO_Y_COMA

retorno             : RETURN expresion PUNTO_Y_COMA

salida              : WRITE PAR_ABRE expresion PAR_CIERRA PUNTO_Y_COMA
                    | WRITE PAR_ABRE CADENA PAR_CIERRA PUNTO_Y_COMA

seleccion           : IF PAR_ABRE condiciones PAR_CIERRA bloque
                    | IF PAR_ABRE condiciones PAR_CIERRA bloque ELSE bloque

iteracion           : LOOP bloque UNTIL PAR_ABRE condiciones PAR_CIERRA PUNTO_Y_COMA

bloque              : LLAVE_ABRE lista_sentencias LLAVE_CIERRA

condiciones         : condiciones op_logico condicion
                    | condicion

condicion           : expresion op_comparacion expresion
                    | PAR_ABRE condiciones PAR_CIERRA

expresion           : expresion OP_SUMA termino
                    | expresion OP_RESTA termino
                    | termino

termino             : termino OP_MUL factor
                    | termino OP_DIV factor
                    | factor

factor              : ID
                    | CTE
                    | invocacion
                    | PAR_ABRE expresion PAR_CIERRA

invocacion          : ID PAR_ABRE expresion PAR_CIERRA

op_comparacion      : OP_MENOR
                    | OP_MENOR_IGUAL
                    | OP_MAYOR
                    | OP_MAYOR_IGUAL
                    | OP_IGUAL
                    | OP_DISTINTO

op_logico           : AND
                    | OR
```

---

## 7. Matriz de nuevos estados

Convención: `-1` = cierre de token. `-2` = error léxico.


| Est | LETRA | DIGITO | = | + | - | * | / | < | > | ; | ( | ) | { | } | " | SPACE | TAB | EOL |
| --- | ----- | ------ | - | - | - | - | - | - | - | - | - | - | - | - | - | ----- | --- | --- |
| 0 | 3 | 1 | 4 | 6 | 7 | 8 | 9 | 11 | 14 | 16 | 17 | 18 | 19 | 20 | 21 | 0 | 0 | 0 |
| 1 | -2 | 1 | -2 | -2 | -2 | -2 | 2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 |
| 2 | -2 | 22 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 |
| 3 | 3 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 4 | -1 | -1 | 5 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 5 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 6 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 7 | -1 | 1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 8 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 9 | -1 | -1 | -1 | -1 | -1 | -1 | 10 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 0 |
| 11 | -1 | -1 | 12 | -1 | -1 | -1 | -1 | -1 | 13 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 12 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 13 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 14 | -1 | -1 | 15 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 15 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 16 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 17 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 18 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 19 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 20 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 21 | 21 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -2 | -1 | -2 | -2 | -2 |
| 22 | -1 | 22 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |

---

## 8. Matriz de punteros a función

| Est | LETRA | DIGITO | = | + | - | * | / | < | > | ; | ( | ) | { | } | " | SPACE | TAB | EOL |
| --- | ----- | ------ | - | - | - | - | - | - | - | - | - | - | - | - | - | ----- | --- | --- |
| 0 | inicio_id | inicio_cte | nada | nada | inicio_cte | nada | nada | nada | nada | nada | nada | nada | nada | nada | inicio_cadena | nada | nada | nada |
| 1 | ERROR | continuar_cte | ERROR | ERROR | ERROR | ERROR | continuar_cte | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR |
| 2 | ERROR | continuar_cte | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR |
| 3 | continuar_id | fin_id | fin_id | fin_id | fin_id | fin_id | fin_id | fin_id | fin_id | fin_id | fin_id | fin_id | fin_id | fin_id | fin_id | fin_id | fin_id | fin_id |
| 4 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 5 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 6 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 7 | nada | continuar_cte | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 8 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 9 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 10 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 11 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 12 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 13 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 14 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 15 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 16 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 17 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 18 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 19 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 20 | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada | nada |
| 21 | continuar_cadena | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | ERROR | fin_cadena | ERROR | ERROR | ERROR |
| 22 | fin_cte | continuar_cte | fin_cte | fin_cte | fin_cte | fin_cte | fin_cte | fin_cte | fin_cte | fin_cte | fin_cte | fin_cte | fin_cte | fin_cte | fin_cte | fin_cte | fin_cte | fin_cte |

`fin_id` consulta la tabla de palabras reservadas y puede devolver `IF`, `ELSE`, `VAR`, `LOOP`, `UNTIL`, `OR`, `AND`, `WRITE`, `MAIN`, `RETURN` o `ID`.

| Acción | Qué hace |
|---|---|
| `nada` | no arma lexema |
| `inicio_id` | empieza el lexema con la letra |
| `continuar_id` | agrega letra |
| `fin_id` | cierra lexema; si es palabra reservada, ese token; si no, `ID` y alta en tabla de símbolos |
| `inicio_cte` | empieza constante |
| `continuar_cte` | agrega dígito o `/` |
| `fin_cte` | cierra `CTE` `n/d`; registra en tabla de símbolos; denominador 0 → E3 |
| `inicio_cadena` / `continuar_cadena` / `fin_cadena` | literal entre `"` |
| `ERROR` | error léxico; se informa la línea y se continúa |

---

## 9. Matriz de tokens

| Estado | Nombre | LETRA | DIGITO | = | + | - | * | / | < | > | ; | ( | ) | { | } | " | SPACE | TAB | EOL |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 0 | Comienzo | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 1 | Posible variable racional | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 2 | Posible variable racional | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 3 | ID | -1 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 | 10 |
| 4 | Asignacion o posible igual | 30 | 30 | -1 | 30 | 30 | 30 | 30 | 30 | 30 | 30 | 30 | 30 | 30 | 30 | 30 | 30 | 30 | 30 |
| 5 | Igual | 40 | 40 | 40 | 40 | 40 | 40 | 40 | 40 | 40 | 40 | 40 | 40 | 40 | 40 | 40 | 40 | 40 | 40 |
| 6 | Suma | 50 | 50 | 50 | 50 | 50 | 50 | 50 | 50 | 50 | 50 | 50 | 50 | 50 | 50 | 50 | 50 | 50 | 50 |
| 7 | Resta | 60 | -1 | 60 | 60 | 60 | 60 | 60 | 60 | 60 | 60 | 60 | 60 | 60 | 60 | 60 | 60 | 60 | 60 |
| 8 | Multiplicacion | 70 | 70 | 70 | 70 | 70 | 70 | 70 | 70 | 70 | 70 | 70 | 70 | 70 | 70 | 70 | 70 | 70 | 70 |
| 9 | Division o posible comentario | 80 | 80 | 80 | 80 | 80 | 80 | -1 | 80 | 80 | 80 | 80 | 80 | 80 | 80 | 80 | 80 | 80 | 80 |
| 10 | Comentario | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| 11 | Menor | 90 | 90 | -1 | 90 | 90 | 90 | 90 | 90 | -1 | 90 | 90 | 90 | 90 | 90 | 90 | 90 | 90 | 90 |
| 12 | Menor igual | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 100 |
| 13 | Distinto | 130 | 130 | 130 | 130 | 130 | 130 | 130 | 130 | 130 | 130 | 130 | 130 | 130 | 130 | 130 | 130 | 130 | 130 |
| 14 | Mayor | 110 | 110 | -1 | 110 | 110 | 110 | 110 | 110 | 110 | 110 | 110 | 110 | 110 | 110 | 110 | 110 | 110 | 110 |
| 15 | Mayor igual | 120 | 120 | 120 | 120 | 120 | 120 | 120 | 120 | 120 | 120 | 120 | 120 | 120 | 120 | 120 | 120 | 120 | 120 |
| 16 | Punto y coma | 140 | 140 | 140 | 140 | 140 | 140 | 140 | 140 | 140 | 140 | 140 | 140 | 140 | 140 | 140 | 140 | 140 | 140 |
| 17 | Parentesis abre | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 | 150 |
| 18 | Parentesis cierra | 160 | 160 | 160 | 160 | 160 | 160 | 160 | 160 | 160 | 160 | 160 | 160 | 160 | 160 | 160 | 160 | 160 | 160 |
| 19 | Llave abre | 170 | 170 | 170 | 170 | 170 | 170 | 170 | 170 | 170 | 170 | 170 | 170 | 170 | 170 | 170 | 170 | 170 | 170 |
| 20 | Llave cierra | 180 | 180 | 180 | 180 | 180 | 180 | 180 | 180 | 180 | 180 | 180 | 180 | 180 | 180 | 180 | 180 | 180 | 180 |
| 21 | Cadena | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | 300 | -1 | -1 | -1 |
| 22 | Variable Racional | 20 | -1 | 20 | 20 | 20 | 20 | 20 | 20 | 20 | 20 | 20 | 20 | 20 | 20 | 20 | 20 | 20 | 20 |


---

## 10. Unreads

| Criterio | Unread |
|---|---|
| Transición `-1` y el carácter no forma parte del lexema | Sí |
| Cierre de cadena en `"` | No |
| `EOL` en estado 10 (fin de comentario) | No |
| Blancos en estado 0 | No |

---

## 11. Semántica

| Regla | Definición |
|---|---|
| R1 | Usar un `ID` no declarado es error semántico |
| R2 | Declarar dos veces el mismo `ID` es error semántico |
| R3 | Toda variable se inicializa en `0/1` antes de la primera sentencia |
| R4 | Las constantes se registran en la tabla de símbolos con el par (numerador, denominador) |
| R5 | Toda operación `+ - * /` simplifica por MCD antes de continuar (D4) |
| R6 | Toda operación fuera de D2 se degrada (D9) y se advierte |
| R7 | Llamada recursiva directa: error semántico |
| R8 | El `0` / `-1` de una comparación solo vale dentro de una `<condicion>` |
| R9 | `write(<expresion>)` imprime el racional simplificado como `numerador/denominador` y salto de línea |
| R10 | `write("…")` imprime el literal y salto de línea |
| R11 | `return <expresion>;` fija el valor de retorno de la función actual |

Tabla de símbolos: nombre, tipo, valor (constantes) y longitud. Se exporta a archivo al finalizar la compilación.

---

## 12. Responsabilidad de cada error

| Código | Descripción | Fase |
|---|---|---|
| E1 | Carácter fuera del alfabeto | Léxico |
| E2 | Constante mal formada (`3/`, entero suelto `4`) | Léxico |
| E3 | Denominador cero en literal (`5/0`) | Léxico |
| E4 | Sentencia mal formada | Sintáctico |
| E5 | Variable no declarada (R1) | Semántico |
| E6 | Variable redeclarada (R2) | Semántico |
| E7 | Denominador cero producido por una expresión (D6) | Ejecución |
| E8 | Recursión directa (R7 / D13) | Semántico |
| A1 | Pérdida de precisión por overflow (D9) — advertencia | Ejecución |

Ninguno de E1 a E6 ni E8 aborta la compilación: se registran y se sigue leyendo.

---

## 13. Programas de ejemplo

### 13.1 Correctos

#### P1 — Suma `1/2 + 1/3`

```
main() {
    var a;
    var b;
    var c;
    a = 1/2;
    b = 1/3;
    c = a + b;
    write("Resultado");
    write(c);
}
```

Salida esperada:

```
Resultado
5/6
```

#### P2 — Normalización `2/4`

```
main() {
    var x;
    x = 2/4;
    write(x);
}
```

Salida esperada: `1/2`

#### P3 — `loop` … `until`

```
main() {
    var n;
    var acc;
    n = 0/1;
    acc = 0/1;
    loop {
        acc = acc + n;
        n = n + 1/1;
    } until (n > 4/1);
    write(acc);
    write(n);
}
```

Salida esperada:

```
10/1
5/1
```

#### P4 — `if` / `else`, `and` / `or`

```
main() {
    var a;
    var b;
    var r;
    a = 3/4;
    b = 1/2;
    if ((a > b) and (a <> 1/1)) {
        r = 1/1;
    } else {
        r = 0/1;
    }
    if ((a < 0/1) or (b <= 1/2)) {
        write(r);
    } else {
        write(0/1);
    }
}
```

Salida esperada: `1/1`

#### P5 — Función con `return`

```
cuadrado(x) {
    return x * x;
}

main() {
    var r;
    r = cuadrado(3/2);
    write(r);
    write(cuadrado(2/3) + 1/9);
}
```

Salida esperada:

```
9/4
5/9
```

#### P6 — Pérdida de precisión

```
main() {
    var s;
    var n;
    s = 0/1;
    n = 1/1;
    loop {
        s = s + 1/1 / n;
        n = n + 1/1;
    } until (n > 40/1);
    write(s);
}
```

Salida esperada: una o más advertencias A1 y un racional aproximado ya simplificado. La ejecución continúa.

### 13.2 Errores léxicos y sintácticos

#### E-L1 — Carácter fuera del alfabeto (E1)

```
main() {
    var a;
    a = 1/2;
    write(a); @
}
```

#### E-L2 — Constante mal formada y denominador cero (E2, E3)

```
main() {
    var a;
    var b;
    var c;
    a = 3/;
    b = 4;
    c = 5/0;
    write(a);
}
```

#### E-S1 — Sentencia mal formada (E4)

```
main() {
    var a;
    a = 1/2
    if (a > 0/1)
        write(a);
}
```

### 13.3 Errores semánticos

#### E-M1 — Redeclaración y variable no declarada (E5, E6)

```
main() {
    var a;
    var a;
    a = 1/2;
    write(b);
}
```

#### E-M2 — Recursión directa (E8)

```
f(x) {
    return f(x);
}

main() {
    f(1/1);
}
```
