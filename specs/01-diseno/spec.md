# Spec — Diseño del lenguaje RG

**Grupo:** F · **Lenguaje de implementación:** C
**Estado:** borrador (Entrega 1)

---

## 1. Decisiones globales

| #   | Decisión                              | Valor                                                                                                                                                    |
| --- | -------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| D1  | Tipo de datos                          | Un solo tipo: racional, representado como par ordenado (numerador, denominador) de enteros con signo de 32 bits cada uno                                    |
| D2  | Rango de numerador y denominador       | −2.147.483.648 a 2.147.483.647                                            |
| D3  | Literales racionales                   | `numerador/denominador` (ej. `3/4`). Un entero suelto `n` se interpreta como `n/1`                                                                          |
| D4  | Forma normal (invariante permanente)   | Simplificado por MCD tras cada operación; denominador siempre positivo (el signo se lleva en el numerador); el cero se representa como `0/1`               |
| D5  | Denominador cero en un literal         | Error de compilación (detectado en el análisis léxico/semántico sobre la constante)                                                                        |
| D6  | Denominador cero producido en runtime  | Error de ejecución, con mensaje y línea de la operación                                                                                                     |
| D7  | Operaciones aritméticas                | `+ - * /` sobre racionales, con simplificación posterior obligatoria (D4)                                                                                   |
| D8  | Comparaciones                          | Exactas, sin conversión a punto flotante (producto cruzado, atención al signo). Resultado: `-1` verdadero, `0` falso. No es un valor racional asignable: solo se usa para resolver una `<condicion>` |
| D9  | Overflow                               | Degradación controlada de precisión: se divide numerador y denominador por la misma potencia de 2 (con redondeo del numerador) hasta entrar en rango, y se vuelve a simplificar por MCD. No aborta el programa; emite una advertencia con la línea afectada |
| D10 | Declaración de variables               | Obligatoria; no se exige orden respecto del primer uso                                                                                                      |
| D11 | Alcance                                | Único y global: no hay variables locales                                                                                                                    |
| D12 | Funciones                              | Sin variables locales, sobre memoria estática global. Admiten exactamente un parámetro racional, pasado por valor, en dirección fija asignada en compilación. Devuelven un valor racional |
| D13 | Recursión                              | Prohibida (directa). Se detecta y reporta como error semántico                                                                                              |
| D14 | Sensibilidad a mayúsculas              | Sí. `Total` y `total` son variables distintas                                                                                                               |
| D15 | Longitud máxima de identificador       | 20 caracteres; más largo se trunca con advertencia                                                                                                          |
| D16 | Comentarios                            | De línea, `//` hasta fin de línea                                                                                                                           |
| D17 | Plataforma destino                     | -                                                                                                                   |

---

## 2. Alfabeto

| Clase  | Caracteres                             |
| ------ | --------------------------------------- |
| `L`    | `a`–`z`, `A`–`Z`                        |
| `D`    | `0`–`9`                                 |
| `SIM`  | `+ - * / ( ) { } ; = < >`               |
| `BL`   | espacio, tabulador, salto de línea       |
| `OTRO` | cualquier otro carácter → error léxico  |

---

## 3. Palabras reservadas

`if` · `else` · `loop` · `until` · `or` · `and` · `read` · `write` · `main`· `var`

Se reconocen como identificadores y se resuelven por búsqueda en tabla, no con
estados propios del autómata.

---

## 4. Tabla de tokens

| Código | Token             | Lexema                                                          |
| ------ | ----------------- | ---------------------------------------------------------------- |
| 10    | `ID`               | identificador                                                    |
| 20    | `CTE`              | constante racional (ej. `3/4`; un entero suelto `n` vale `n/1`) |
| 30    | `OP_ASIG`          | `=`                                                               |
| 40    | `OP_IGUAL`         | `==`                                                              |
| 50    | `OP_SUMA`          | `+`                                                               |
| 60    | `OP_RESTA`         | `-`                                                               |
| 70    | `OP_MUL`           | `*`                                                               |
| 80    | `OP_DIV`           | `/`                                                               |
| 90   | `OP_MENOR`         | `<`                                                               |
| 100   | `OP_MENOR_IGUAL`   | `<=`                                                              |
| 110    | `OP_MAYOR`         | `>`                                                               |
| 120    | `OP_MAYOR_IGUAL`   | `>=`                                                              |
| 130    | `OP_DISTINTO`      | `<>`                                                              |
| 140    | `PUNTO_Y_COMA`     | `;`                                                               |
| 150    | `PAR_ABRE`         | `(`                                                               |
| 160    | `PAR_CIERRA`       | `)`                                                               |
| 170    | `LLAVE_ABRE`       | `{`                                                               |
| 180    | `LLAVE_CIERRA`     | `}`                                                               |
| 190    | `IF`               | `if`                                                              |
| 200    | `ELSE`             | `else`                                                            |
| 210    | `VAR`             | `var`                                                            |
| 220    | `LOOP`             | `loop`                                                            |
| 230    | `UNTIL`            | `until`                                                           |
| 240    | `OR`               | `or`                                                              |
| 250    | `AND`              | `and`                                                             |
| 260    | `READ`             | `read`                                                            |
| 270    | `WRITE`            | `write`                                                           |
| 280    | `MAIN`             | `main`                                                            |

---

## 5. Estructura del programa

Un programa RG consiste en una función principal obligatoria, de nombre
reservado `main`, sin parámetros y sin valor de retorno, más una cantidad
arbitraria de funciones adicionales definidas por el usuario. Todas las
funciones (incluida `main`) comparten el mismo espacio de memoria estática
global: no existen variables locales. Cada función adicional admite
exactamente un parámetro racional, pasado por valor y alojado en una
dirección fija asignada en tiempo de compilación.

---

## 6. Gramática

```
<programa>       ::= <funciones> MAIN '(' ')' <bloque>

<funciones>      ::= <funciones> <funcion>
                   | /* vacío */

<funcion>        ::= ID '(' ID ')' <bloque>

<bloque>         ::= '{' <sentencias> '}'

<sentencias>     ::= <sentencias> <sentencia>
                   | <sentencia>

<sentencia>      ::= <declaracion>
                   | <asignacion>
                   | <seleccion>
                   | <iteracion>
                   | <entrada>
                   | <salida>
                   | <llamada>

<declaracion>    ::= ID ';'

<asignacion>     ::= ID OP_ASIG <expresion> ';'

<seleccion>      ::= IF '(' <condicion> ')' <bloque>
                   | IF '(' <condicion> ')' <bloque> ELSE <bloque>

<iteracion>      ::= LOOP <bloque> UNTIL '(' <condicion> ')' ';'

<entrada>        ::= READ '(' ID ')' ';'

<salida>         ::= WRITE '(' <expresion> ')' ';'

<llamada>        ::= ID '(' <expresion> ')' ';'

<condicion>      ::= <condicion> OR <termino_cond>
                   | <condicion> AND <termino_cond>
                   | <termino_cond>

<termino_cond>   ::= '(' <condicion> ')'
                   | <expresion> <comparador> <expresion>

<comparador>     ::= OP_IGUAL | OP_DISTINTO | OP_MENOR | OP_MENOR_IGUAL
                   | OP_MAYOR | OP_MAYOR_IGUAL

<expresion>      ::= <expresion> OP_SUMA <termino>
                   | <expresion> OP_RESTA <termino>
                   | <termino>

<termino>        ::= <termino> OP_MUL <factor>
                   | <termino> OP_DIV <factor>
                   | <factor>

<factor>         ::= ID
                   | CTE
                   | ID '(' <expresion> ')'
                   | '(' <expresion> ')'
```

**Notas sobre la gramática**

- Recursión a izquierda en `<expresion>`, `<termino>`, `<condicion>`,
  `<sentencias>` y `<funciones>`: es la forma que prefiere una herramienta
  YACC/Bison.
- La precedencia aritmética queda resuelta por la estructura en tres niveles
  (`expresion` → `termino` → `factor`), no por declaraciones de precedencia.
- El `else` colgante no existe como problema: `<bloque>` siempre lleva
  llaves.
- Esta gramática no fija una precedencia entre `and`/`or` cuando aparecen
  combinados sin paréntesis explícitos. El grupo debe decidir: (a) exigir
  paréntesis siempre que se combinen ambos operadores, o (b) fijar `and` con
  mayor precedencia que `or` (convención usual en la mayoría de los
  lenguajes) si prefieren permitirlo sin paréntesis.
- **Punto abierto de diseño:** la consigna exige que las funciones devuelvan
  un valor racional (F.5.17), pero la lista de tokens provista no incluye una
  palabra reservada para `return`. La gramática ya permite *llamar* una
  función dentro de una expresión (regla `<factor> ::= ID '(' <expresion> ')'`),
  pero falta definir *cómo* la función comunica su valor de salida al
  llamador. Opciones típicas a resolver antes de la Entrega 2: (a) agregar un
  token `RETURN`, (b) que el valor devuelto sea el de la última expresión
  evaluada en el bloque, (c) que se devuelva el valor de una variable global
  con el mismo nombre que la función.
- La recursión directa es sintácticamente válida (una función puede
  invocarse a sí misma dentro de su propio cuerpo), pero debe rechazarse en
  la etapa semántica (ver R7 y E8).
- **Punto abierto de diseño:** las Consignas Generales (punto 7) exigen, además
  de `write` (que muestra el valor de una variable o expresión), una sentencia
  que emita un **literal de texto** para rotular la salida (ej. `write("Resultado:")`).
  La lista de tokens provista no incluye un token de cadena/string, así que
  `<salida>` tal como está definida arriba no cubre este requisito. Falta
  decidir: (a) agregar un token `CADENA` (delimitado por `"` u otro carácter,
  lo que implica sumar ese símbolo al alfabeto §2), o (b) sobrecargar `write`
  para que acepte también una cadena literal, con una regla adicional
  `<salida> ::= WRITE '(' CADENA ')' ';'`. Cualquiera de las dos opciones es
  válida; hay que resolverla antes de cerrar esta spec porque es un requisito
  obligatorio de la cátedra, no solo de este grupo.

---

## 7. Semántica

| Regla | Definición                                                                                  |
| ----- | -------------------------------------------------------------------------------------------- |
| R1    | Usar un `ID` no declarado es error semántico                                                 |
| R2    | Declarar dos veces el mismo `ID` es error semántico                                          |
| R3    | Toda variable se inicializa en `0/1` antes de la primera sentencia (forma normal, D4)          |
| R4    | Las constantes se registran en la tabla de símbolos junto con su par (numerador, denominador) |
| R5    | Toda operación (`+ - * /`) simplifica el resultado por MCD antes de continuar (D4)              |
| R6    | Toda operación que exceda el rango D2 se degrada por potencias de 2 y se advierte (D9)          |
| R7    | Una llamada recursiva directa (una función que se invoca a sí misma) es error semántico (D13)   |
| R8    | El resultado de una comparación (`0`/`-1`) solo es válido dentro de una `<condicion>`; no es asignable a una variable racional |
| R9    | `write` imprime el racional ya simplificado, en formato `numerador/denominador`, seguido de salto de línea |

---

## 8. Responsabilidad de cada error

| Código | Descripción                                          | Fase que lo detecta                  |
| ------ | ----------------------------------------------------- | ------------------------------------- |
| E1     | Carácter fuera del alfabeto                           | Léxico                                |
| E2     | Constante racional mal formada (ej. `3/`, `/4`)        | Léxico                                |
| E3     | Denominador cero en una constante literal (ej. `5/0`)  | Léxico                                |
| E4     | Sentencia mal formada                                  | Sintáctico                            |
| E5     | Variable no declarada (R1)                             | Sintáctico (sobre tabla de símbolos)  |
| E6     | Variable redeclarada (R2)                              | Sintáctico (sobre tabla de símbolos)  |
| E7     | Denominador cero producido por una expresión (D6)      | Ejecución                             |
| E8     | Recursión directa (R7 / D13)                           | Semántico (sobre tabla de símbolos)   |
| A1     | Pérdida de precisión por overflow (D9) — advertencia, no aborta | Ejecución (código emitido por GCA) |

Ninguno de E1 a E6 ni E8 aborta la compilación: se registran y se sigue
leyendo. E7 y A1 se detectan en tiempo de ejecución.

---

## 9. Programas de ejemplo

Los programas de esta sección siguen la gramática de §6. Hasta resolver el
punto abierto de `return` (notas de §6), las funciones comunican el valor de
salida asignándolo a una variable global con el mismo nombre que la función
(opción c). No se usan cadenas en `write` ni el unario `-` al inicio de una
expresión: ambos quedan fuera de la gramática actual. Un racional negativo se
escribe como resta binaria (`0 - 1/2`).

### 9.1 Programas correctos

#### P1 — Suma `1/2 + 1/3` (caso F.6: común denominador y simplificación)

```
main() {
    a;
    b;
    c;
    a = 1/2;
    b = 1/3;
    c = a + b;
    write(c);
}
```

Salida esperada: `5/6`

#### P2 — Normalización de un literal simplificable (caso F.6)

`2/4` debe almacenarse y mostrarse ya en forma normal (D4).

```
main() {
    x;
    x = 2/4;
    write(x);
}
```

Salida esperada: `1/2`

#### P3 — Signo en el numerador, cero como `0/1` y operaciones

```
main() {
    a;
    b;
    c;
    z;
    a = 0 - 3/6;      // forma normal: -1/2
    b = 4/2;          // forma normal: 2/1
    c = a * b;        // -1/1
    z = a + 1/2;      // 0/1
    write(a);
    write(b);
    write(c);
    write(z);
}
```

Salida esperada:

```
-1/2
2/1
-1/1
0/1
```

#### P4 — `if` / `else`, comparaciones y `and` / `or` con paréntesis

Ejercita F.4.13 y F.4.15. Las comparaciones no se asignan a variables (R8).

```
main() {
    a;
    b;
    r;
    a = 3/4;
    b = 1/2;
    if ((a > b) and (a <> 1)) {
        r = 1;
    } else {
        r = 0;
    }
    if ((a < 0) or (b <= 1/2)) {
        write(r);
    } else {
        write(0);
    }
}
```

Salida esperada: `1/1`

#### P5 — `loop` … `until`, anidamiento y `read` / `write`

El cuerpo se ejecuta al menos una vez (condición al final). Incluye `if`
anidado dentro del ciclo (F.4.14).

```
main() {
    n;
    acc;
    acc = 0;
    read(n);
    loop {
        acc = acc + n;
        if (n > 0) {
            n = n - 1;
        } else {
            n = 0;
        }
    } until ((n <= 0) or (acc >= 10));
    write(acc);
}
```

Con entrada `4/1`, el ciclo suma `4 + 3 + 2 + 1` y termina al quedar `n = 0`.
Salida esperada: `10/1`

#### P6 — Función de un parámetro, usada como expresión y como sentencia

El parámetro se pasa por valor a una dirección estática fija (D12). `cuadrado`
se usa dentro de una expresión; `mostrar` se invoca como sentencia (`<llamada>`).

```
cuadrado(x) {
    cuadrado;
    cuadrado = x * x;
}

mostrar(v) {
    write(v);
}

main() {
    r;
    r = cuadrado(3/2);
    mostrar(r);
    write(cuadrado(2/3) + 1/9);
}
```

Salida esperada:

```
9/4
5/9
```

#### P7 — Acumulación hasta pérdida de precisión (caso F.6 / D9 / A1)

Suma exacta de `1 + 1/2 + … + 1/n`. El denominador tiende al MCM de
`1..n`, que supera el rango de 32 bits alrededor de `n = 23`. El programa no
aborta: degrada por potencias de 2, simplifica y advierte (A1).

```
main() {
    s;
    n;
    s = 0;
    n = 1;
    loop {
        s = s + 1 / n;
        n = n + 1;
    } until (n > 40);
    write(s);
}
```

Salida esperada: una o más advertencias A1 (con la línea de `s = s + 1 / n`)
y un racional aproximado en forma `numerador/denominador`, ya simplificado.
El valor exacto de la suma armónica no cabe en el par de 32 bits; lo
verificable es que la ejecución continúa y que se emitió A1.

### 9.2 Programas con errores léxicos y sintácticos

Ninguno de estos aborta la compilación en el primer hallazgo (punto 13–14 de
Consignas Generales): se registran y se sigue leyendo.

#### E-L1 — Carácter fuera del alfabeto (E1)

`@` no pertenece al alfabeto (§2).

```
main() {
    a;
    a = 1/2;
    write(a); @
}
```

Error esperado: E1 en la línea del `@`.

#### E-L2 — Constante mal formada y denominador cero literal (E2, E3)

```
main() {
    a;
    b;
    a = 3/;
    b = 5/0;
    write(a);
}
```

Errores esperados: E2 por `3/`; E3 por `5/0` (D5).

#### E-S1 — Sentencia mal formada (E4)

Falta `;` en la asignación y el `if` no abre bloque con `{` `}` (la gramática
exige `<bloque>`).

```
main() {
    a;
    a = 1/2
    if (a > 0)
        write(a);
}
```

Errores esperados: E4 (asignación incompleta y selección sin llaves).

#### E-S2 — Estructura `loop` / `until` incompleta (E4)

Falta la condición entre paréntesis y el `;` final de `<iteracion>`.

```
main() {
    i;
    i = 0;
    loop {
        i = i + 1;
    } until
    write(i);
}
```

Error esperado: E4.

### 9.3 Programas con errores semánticos

#### E-M1 — Variable no declarada y redeclaración (E5, E6 / R1, R2)

```
main() {
    a;
    a;
    a = 1/2;
    write(b);
}
```

Errores esperados: E6 por la segunda `a;`; E5 por el uso de `b`.

#### E-M2 — Recursión directa (E8 / R7 / D13; caso F.6)

Sintácticamente válido; debe rechazarse en semántica.

```
f(x) {
    f(x);
}

main() {
    f(1);
}
```

Error esperado: E8 en la llamada `f(x)` dentro de `f`.

---

## 10. Fuera de alcance

Se deja constancia de lo que RG **no** tiene, para que ninguna fase lo asuma:

- Punto flotante nativo, caracteres, cadenas, booleano como tipo aparte, arreglos
- Variables locales y anidamiento de alcances
- Recursión, directa o indirecta
- Funciones con más de un parámetro
- Entrada/salida más allá de `read`/`write` sobre racionales