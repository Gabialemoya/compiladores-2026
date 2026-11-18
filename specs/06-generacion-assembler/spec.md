# Spec — Generación de assembler de RG

**Depende de:** `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md` · `specs/05-codigo-intermedio/spec.md`
**Produce:** `src/assembler/` · la acción de `programa` en `parser.y` · `out/programa.asm`

Traduce la lista de tercetos ya cerrada a un programa NASM de 32 bits. La aritmética racional, la forma normal, la degradación y la advertencia A1 ocurren en la ejecución de ese programa.

Esta etapa fija la plataforma que `01-diseno` dejó abierta en D17: NASM, x86 de 32 bits, objeto `win32`, enlace con gcc.

---

## 1. Alcance

Al reducir el axioma, `genera_cod_asm` recorre la lista desde el primer terceto y escribe `out/programa.asm`. Cada terceto aporta su etiqueta y sus instrucciones. Las operaciones del racional viven en rutinas del mismo archivo, una sola vez.

---

## 2. Invocación

La generación de código intermedio termina al reducir `programa`: los saltos están completos y las llamadas pendientes también. Recién ahí se traduce a assembler.

El axioma es el start symbol. Las dos alternativas llaman a la misma rutina:

```
programa : entrada lista_funciones funcion_main { genera_cod_asm(); }
         | entrada funcion_main                 { genera_cod_asm(); }
```

`genera_cod_asm` es la única traducción a assembler. El resto de las reglas solo arma tercetos: no abre el `.asm`, no emite una instrucción y no llama a otra rutina de traducción. El módulo puede partir el recorrido en funciones internas. Ninguna regla, salvo `programa`, las llama.

`main` no vuelve a generar assembler después de `yyparse`. Si el fuente no reduce `programa`, la rutina no se ejecuta.

Si `hay_errores` está levantado, `genera_cod_asm` vuelve sin escribir. Un salto que haya quedado vacío no entra en un archivo para ensamblar. El `out/programa.asm` de una corrida anterior queda como estaba.

---

## 3. Módulo `src/assembler/`

### 3.1 Interfaz — `assembler.h`

```c
void genera_cod_asm(void);
```

`parser.y` incluye `../assembler/assembler.h`. La única aparición de `genera_cod_asm` en la gramática son las dos acciones de §2.

### 3.2 `genera_cod_asm` — `assembler.c`

Lee `tercetos`, `cant_tercetos` y la tabla de símbolos. Escribe `out/programa.asm` y crea el directorio `out/` si no existe. Si el archivo no se puede crear, informa `Error al crear archivo assembler.` y levanta `hay_errores`.

En el apunte la rutina se llama `generaCodAsm`. En el compilador el nombre es `genera_cod_asm`.

---

## 4. Memoria estática

Un racional ocupa dos `dd` contiguos: numerador y después denominador. Un resultado entero ocupa un `dd`. Todo tiene dirección fija. El programa generado no pide memoria.

| Dato | Etiqueta | Inicial |
| --- | --- | --- |
| Variable, parámetro, `@ret_`, `@tmpN` | §4.1 | `0`, `1` |
| Constante | §4.1 | el par de `valor`, ya normalizado |
| Cadena | `str` y el índice de tabla | los bytes de `valor` y un `0` |
| Terceto `TIPO_RACIONAL` | `ter` y el índice | `0`, `1` |
| Terceto `TIPO_ENTERO` | `ter` y el índice | `0` |
| Terceto `TIPO_NINGUNO` | sin hueco de dato | — |

El orden en `.data` es el de la tabla de símbolos y, detrás, los huecos `ter` en orden de índice. Los formatos de §7 van al final de `.data`.

Una celda racional que se lee durante la ejecución está en forma normal: denominador positivo, simplificada por MCD, y el cero guardado como `0/1`. Las variables arrancan así (R3). Las constantes entran así desde la tabla. Cada operación de §7 vuelve a dejarlas así.

### 4.1 Etiqueta de un símbolo

Los identificadores del fuente son solo letras. El prefijo de las otras etiquetas lleva un dígito o un `_`, así que no pisan un nombre del fuente.

| Símbolo | Etiqueta |
| --- | --- |
| Racional cuyo nombre son solo letras | el nombre |
| `@ret_<nombre>` | `ret_<nombre>` |
| `@tmpN` | `tmpN` |
| Constante `numerador/denominador` | `cte_`, el numerador, `_` y el denominador. El menos del numerador se escribe `m` |
| Cadena | `str` y el índice de tabla |

`1/2` queda `cte_1_2`. `-3/4` queda `cte_m3_4`. El generador parte el texto `valor`. No vuelve a simplificar esa constante: el léxico ya la dejó en forma normal.

---

## 5. Recorrido

El recorrido empieza en el terceto `0` y sigue mientras el índice sea menor que `cant_tercetos`. El `BI` inicial se traduce como cualquier otro terceto. No hay un segundo recorrido que empiece en `main`.

Cada índice `i` emite la etiqueta `t<i>`: y, en la línea anterior, un comentario con el índice y el operador. Un `REF_DIR` con valor `k` es la etiqueta `t<k>`.

Si `k` es igual a `cant_tercetos`, el destino es la instrucción que sigue al último terceto. Esa etiqueta se emite al cerrar el recorrido:

```
t<cant_tercetos>:
    xor eax, eax
    ret
```

Ahí termina el proceso cuando `main` no ejecuta un `T_RET`. El código de retorno es `0`.

Las rutinas de §7 se emiten después de esa etiqueta. El hilo de `main` no cae adentro de ellas.

`_main` es la primera etiqueta de código y coincide con `t0`. El CRT llama a `_main`. El terceto `0` es un `BI` hacia `main`, así que las funciones que quedaron delante solo se alcanzan con `call`. El `ret` de una función vuelve a ese `call`. El `ret` de `main` vuelve al CRT, porque el `BI` no apila un retorno.

---

## 6. Traducción de cada terceto

La dirección de un operando se resuelve así. `REF_TS` usa la etiqueta de §4.1. `REF_TER` usa `ter` y el índice. `REF_DIR` es una etiqueta `t`, no una celda. `REF_VACIO` no se usa.

El código generado solo deja basura en `eax`, `ecx` y `edx`. Entre un terceto y el siguiente no se conserva ningún registro: el resultado quedó escrito en memoria.

### 6.1 Aritmética

`T_SUMA`, `T_RESTA`, `T_MUL` y `T_DIV` llaman a `suma_rac`, `resta_rac`, `mul_rac` o `div_rac`. El destino es el hueco `ter<i>` de ese terceto. La línea es la del terceto.

```
t<i>:
    push <linea>
    push ter<i>
    push <dir op2>
    push <dir op1>
    call <rutina>
    add esp, 16
```

### 6.2 Asignación

`T_ASIG` copia los ocho bytes del origen al destino. El origen es un racional de la tabla o el hueco de un terceto. El destino es una celda de la tabla.

```
t<i>:
    mov eax, [<origen>]
    mov [<destino>], eax
    mov eax, [<origen>+4]
    mov [<destino>+4], eax
```

### 6.3 Comparación

`T_IGUAL`, `T_DISTINTO`, `T_MENOR`, `T_MENOR_IGUAL`, `T_MAYOR` y `T_MAYOR_IGUAL` llaman a `comparar`. El resultado queda en el `dd` de `ter<i>`: `-1` si la relación se cumple y `0` si no. No hay degradación ni aviso.

```
t<i>:
    push <codigo>
    push ter<i>
    push <dir op2>
    push <dir op1>
    call comparar
    add esp, 16
```

| Operador | Código |
| --- | --- |
| `T_IGUAL` | `0` |
| `T_DISTINTO` | `1` |
| `T_MENOR` | `2` |
| `T_MENOR_IGUAL` | `3` |
| `T_MAYOR` | `4` |
| `T_MAYOR_IGUAL` | `5` |

### 6.4 Saltos

`T_BI` salta siempre. El destino está en `op1`.

```
t<i>:
    jmp t<destino>
```

`T_BF` salta si el entero de `op1` es `0`. El destino está en `op2`. Caer a la instrucción siguiente es el caso verdadero (`-1`).

```
t<i>:
    cmp dword [ter<condicion>], 0
    je t<destino>
```

### 6.5 Salida

`T_WRITE` imprime el racional de `op1` con el formato `numerador/denominador` y un salto de línea (R9). `T_WRITE_STR` imprime la cadena de `op1` y un salto de línea (R10).

```
t<i>:
    push dword [<op>+4]
    push dword [<op>]
    push fmt_rac
    call _printf
    add esp, 12
```

```
t<i>:
    push <dir de la cadena>
    push fmt_str
    call _printf
    add esp, 8
```

### 6.6 Llamada y retorno

`T_CALL` entra a la etiqueta de `op1`. El `T_ASIG` anterior ya copió el argumento al parámetro. El `T_ASIG` siguiente copia `@ret_` al `@tmpN`.

```
t<i>:
    call t<destino>
```

`T_RET` no transporta el racional. El valor ya está en `@ret_`.

```
t<i>:
    ret
```

---

## 7. Racionales en ejecución

Las rutinas de esta sección se emiten una vez en `out/programa.asm`. Usan la convención cdecl: argumentos de derecha a izquierda, el llamador limpia la pila, y `ebx`, `esi` y `edi` se restauran. La cuenta se hace con enteros de 64 bits. No se usa la FPU.

`_printf` y `_exit` se declaran `extern`. En el enlace con gcc son `printf` y `exit` de la biblioteca de C. `_exit` vacía la salida antes de terminar.

| Formato | Texto |
| --- | --- |
| `fmt_rac` | `"%d/%d"` y salto de línea |
| `fmt_str` | `"%s"` y salto de línea |
| `fmt_a1` | `Linea %d: Advertencia: perdida de precision` y salto de línea |
| `fmt_e8` | `Linea %d: Error: denominador cero` y salto de línea |

A1 se imprime por la misma salida que `write` y la ejecución sigue. E8 se imprime y el proceso termina con `push 1` y `call _exit`.

### 7.1 MCD, signo y degradación

`mcd(a, b)` es el algoritmo de Euclides con `a` y `b` mayores o iguales que cero. `mcd(0, x)` devuelve `x`.

`div2(n)` divide por dos con redondeo, y la mitad se aleja de cero. La división entera es hacia cero. Si `n` es mayor o igual que cero, el resultado es `(n + 1) / 2`. Si es menor, `(n - 1) / 2`.

`normalizar` recibe el numerador y el denominador en 64 bits, la dirección de destino y la línea. El denominador que le llega es distinto de cero.

1. Si el denominador es negativo, se niegan los dos. Si el numerador es −2⁶³ y no se puede negar, pasa a ser −2⁶² y el denominador se divide por dos. Si ese denominador queda en cero, pasa a ser `1`.
2. Si el numerador es cero, se guarda `0/1` y termina.
3. Se dividen los dos por `mcd` de sus valores absolutos.
4. Mientras el numerador quede fuera de −2.147.483.648…2.147.483.647, o el denominador quede por encima de 2.147.483.647, se reemplaza el numerador por `div2` y el denominador por su mitad hacia cero. Si el denominador queda en cero, pasa a ser `1`. Esa operación queda marcada para A1.
5. Se vuelve a simplificar por MCD. Si el numerador quedó en cero, se guarda `0/1`.
6. Si la operación quedó marcada, se imprime A1 con la línea. El aviso es uno por llamada, aunque el paso 4 haya repetido varias veces.
7. Se guardan los dos enteros de 32 bits en el destino.

### 7.2 Suma, resta, producto y división

Los dos operandos entran en forma normal. El resultado sale por `normalizar`.

| Rutina | Cuenta, con `b > 0` y `d > 0` |
| --- | --- |
| `suma_rac` de `a/b` y `c/d` | `g = mcd(b, d)`. Numerador `a*(d/g) + c*(b/g)`. Denominador `b*(d/g)` |
| `resta_rac` | la suma de `a/b` y `(-c)/d` |
| `mul_rac` | `g1 = mcd(\|a\|, d)` y `g2 = mcd(\|c\|, b)`. Numerador `(a/g1)*(c/g2)`. Denominador `(b/g2)*(d/g1)` |
| `div_rac` de `a/b` por `c/d` | si `c` es cero, E8 y termina. Si no, multiplica `a/b` por `d/c`, pasando antes el signo de `c` al numerador para que el denominador quede positivo |

Cada producto de dos enteros de 32 bits entra en 64 bits. El numerador de la suma es la suma de dos de esos productos. Si esa suma no entra en 64 bits con signo, los dos productos y el denominador se reemplazan por su mitad (`div2` en cada producto, mitad hacia cero en el denominador) y se suman de nuevo, hasta que la suma entre. Recién ahí se llama a `normalizar`. Esa mitad también marca la operación para A1.

### 7.3 Comparación

`comparar` calcula `p = a*d` y `q = c*b` en 64 bits. Los denominadores son positivos, así que el orden de `p` y `q` es el de los racionales. Los dos productos se comparan como enteros de 64 bits con signo, sin restarlos cuando la resta no entra en 64 bits y sin pasar por flotante.

Según el código de §6.3 guarda `-1` o `0` en el `dd` de destino.

---

## 8. Forma de `out/programa.asm`

Un archivo NASM, sintaxis Intel, `section .data` y después `section .text`. La primera línea de código global es:

```
global _main
extern _printf
extern _exit
```

`p1_suma` deja esta forma. Las rutinas de §7 van después de `t7` y acá no se listan.

```
section .data
a:        dd 0
          dd 1
b:        dd 0
          dd 1
c:        dd 0
          dd 1
cte_1_2:  dd 1
          dd 2
cte_1_3:  dd 1
          dd 3
str5:     db "Resultado", 0
ter3:     dd 0
          dd 1

section .text
global _main
extern _printf
extern _exit

_main:
; 0 BI
t0:
    jmp t1
; 1 ASIG
t1:
    mov eax, [cte_1_2]
    mov [a], eax
    mov eax, [cte_1_2+4]
    mov [a+4], eax
; 3 SUMA
t3:
    push 7
    push ter3
    push b
    push a
    call suma_rac
    add esp, 16
; 6 WRITE
t6:
    push dword [c+4]
    push dword [c]
    push fmt_rac
    call _printf
    add esp, 12
t7:
    xor eax, eax
    ret
```

Una comilla doble adentro de una cadena se duplica al emitir el `db`. El índice de `str5` es el de esa corrida: si la tabla cambia de orden, cambia el número, no el criterio.

---

## 9. Compilación y ensamblado

```
win_bison -d --report=all -o src/parser/parser.tab.c src/parser/parser.y
gcc -Wall -Wextra -o compilador.exe src/parser/parser.tab.c src/lexico/lexico.c src/intermedio/tercetos.c src/assembler/assembler.c
```

El primer comando se repite cada vez que cambia `parser.y`.

```
.\compilador.exe tests\correctos\p1_suma.rg
nasm -f win32 -o out\programa.obj out\programa.asm
gcc -m32 -o out\programa.exe out\programa.obj
.\out\programa.exe
```

El compilador sigue pisando `out/tokens.txt`, `out/ts.txt` y `out/tercetos.txt`. `out/programa.asm` se escribe al reducir `programa`, antes del veredicto.

---

## 10. Aceptación

- Bison genera el parser sin conflictos.
- Compila sin warnings con `-Wall -Wextra`.
- En `parser.y`, `genera_cod_asm` aparece en las dos alternativas de `programa` y en ninguna otra regla.
- Los programas de `tests/correctos/` terminan en compilación exitosa y dejan `out/programa.asm`.
- Ese archivo ensambla con NASM y enlaza con `gcc -m32`. El proceso sale con código `0`.
- Los programas que no llegan al axioma no regeneran el assembler.
- `em1_declaracion` y `em2_recursion` reducen `programa` con `hay_errores` levantado. `genera_cod_asm` vuelve sin escribir y el `out/programa.asm` de una corrida anterior queda como estaba.

Salida del ejecutable:

| Programa | Salida |
| --- | --- |
| `p1_suma` | `Resultado` y después `5/6` |
| `p2_normalizar` | `1/2` |
| `p3_loop_until` | `10/1` y después `5/1` |
| `p4_if_else` | `1/1` |
| `p5_funcion` | `9/4` y después `5/9` |
| `p6_overflow` | Al menos una línea `Advertencia: perdida de precision`. La última línea es un racional `numerador/denominador` ya simplificado, con denominador positivo. El proceso no se cancela |
| `p7_condiciones` | Sin líneas. El `and` no entra al `write` |
| `p8_expresiones` | `-2/3` |

`p1_suma` produce el esqueleto de §8: el `jmp` inicial, la copia de `cte_1_2` a `a`, la suma en `ter3` y el `write` de `c`.
