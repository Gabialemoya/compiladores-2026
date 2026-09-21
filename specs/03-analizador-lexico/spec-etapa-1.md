# Spec — Analizador léxico de RG (etapa 1: listado)

**Depende de:** `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md`  
**Siguiente:** `[spec-etapa-2.md](spec-etapa-2.md)` (`yylex` para el analizador sintáctico)  
**Produce:** `src/lexico/`

Implementa el analizador léxico **manual** (autómata por matrices).

---

## 1. Alcance e interfaz

### 1.1 Objetivo

El AL es un programa independiente: **abre un archivo fuente RG, lo recorre carácter a carácter y produce un listado de tokens** (más los errores léxicos y las altas en tabla de símbolos).

No parsea, no invoca Bison y no entrega tokens de a uno a un consumidor externo.

### 1.2 Entrada

- Ruta a un archivo de texto (programa RG).
- Fallo de apertura: error de I/O, no error léxico; no hay listado.



### 1.3 Salida

1. **Listado de tokens** reconocidos, en orden, hasta EOF. Blancos y comentarios `//` no figuran.
2. **Informe de errores léxicos** (código, línea, mensaje) sin abortar la ejecución en el primero.
3. **Tabla de símbolos** actualizada para `ID`, `CTE` y `CADENA` (contrato de spec 02). Se debe exportar a un archivo.

Al terminar el archivo el listado termina. **No** se emite un token `EOF`.

### 1.4 Fuera de alcance

- Gramática, árbol, Bison/Yacc.
- Semántica (doble `var`, `ID` no declarado, etc.).
- La firma `int yylex(void)`: eso es `[spec-etapa-2.md](spec-etapa-2.md)`.



### 1.5 Responsabilidades del reconocedor

- Leer el fuente carácter a carácter.
- Agrupar en tokens según el autómata de `01-diseno` §7–8.
- Descartar SPACE, TAB, EOL y comentarios `//`.
- Informar errores léxicos con línea; recuperar (estado 0) y seguir.
- Alta en TS de `ID`, `CTE`, `CADENA` vía `fin_id` / `fin_cte` / `fin_cadena`.

Un `main` de prueba abre el fuente, recorre hasta EOF e **imprime o guarda el listado** (y la TS).

---



## 2. Decisiones propias de esta fase


| #   | Decisión                    | Valor                                                                   |
| --- | --------------------------- | ----------------------------------------------------------------------- |
| L1  | Palabras reservadas         | Como identificador + tabla en `fin_id`. Sin estados extra               |
| L2  | Implementación del autómata | Tablas `nuevo_estado` y `proceso`, no cascada de `if` por tipo de token |
| L3  | Convención `-1` / `-2`      | `-1` = cierre de token; `-2` = error léxico                             |
| L4  | Constante                   | Solo `n/d`. Estado 1 sin `/` es E2                                      |
| L5  | `unget`                     | Según §10 de `01-diseno`                                                |
| L6  | Contador de línea           | Se incrementa al consumir `EOL` (estado 0 o comentario estado 10)       |
| L7  | Interfaz                    | Archivo fuente → listado de tokens                                      |


---



## 3. Eventos (columnas)

`get_evento(c)` mapea el carácter a la columna de las matrices de `01-diseno` §7–8.


| Col | Evento | Caracteres                                                                                    |
| --- | ------ | --------------------------------------------------------------------------------------------- |
| 0   | LETRA  | `A`–`Z`, `a`–`z`                                                                              |
| 1   | DIGITO | `0`–`9`                                                                                       |
| 2   | `=`    | `=`                                                                                           |
| 3   | `+`    | `+`                                                                                           |
| 4   | `-`    | `-`                                                                                           |
| 5   | `*`    | `*`                                                                                           |
| 6   | `/`    | `/`                                                                                           |
| 7   | `<`    | `<`                                                                                           |
| 8   | `>`    | `>`                                                                                           |
| 9   | `;`    | `;`                                                                                           |
| 10  | `(`    | `(`                                                                                           |
| 11  | `)`    | `)`                                                                                           |
| 12  | `{`    | `{`                                                                                           |
| 13  | `}`    | `}`                                                                                           |
| 14  | `"`    | `"`                                                                                           |
| 15  | SPACE  | espacio                                                                                       |
| 16  | TAB    | tabulador (`0x09`)                                                                            |
| 17  | EOL    | `LF` (`0x0A`); si aparece `CR`, tratarlo como blanco (SPACE o EOL) y documentarlo en bitácora |


Cualquier otro valor no es columna válida: E1 con la línea y se continúa.

---



## 4. Estados

Estados `0`…`22` de `01-diseno`. `-1` cierra token. `-2` es error léxico.

---



## 5. Acciones semánticas

Las de `01-diseno` §8: `inicio_id`, `continuar_id`, `fin_id`, `inicio_cte`, `continuar_cte`, `fin_cte`, `inicio_cadena`, `continuar_cadena`, `fin_cadena`, `nada`, `ERROR`.

`fin_cte`: alta en TS. Denominador `0` → E3.

`fin_id`: primero palabras reservadas; si no, `ID` y alta si aún no está. La duplicidad de **declaración** es semántica, no léxica.

Al cerrar un token (`-1` sin error) se **agrega una entrada al listado** con el valor del token.

---



## 6. Matriz de nuevos estados

Copiar en código el arreglo de `01-diseno` §7.

`int nuevo_estado[23][18]`

---



## 7. Matriz de transiciones (`proceso`)

Copiar en código el arreglo de `01-diseno` §8.

En C, tabla de punteros a función, indexada `[estado][columna]`:

```
int (*proceso[23][18])(void)
```

---



## 8. Unreads

Según `01-diseno` §10. Si `nuevo_estado` es `-1` y el carácter no pertenece al lexema: `unget(c)`.

---



## 9. Pseudocódigo

Núcleo: reconocer **un** token.

```
reconocer_token():
    estado = 0
    mientras verdadero
        si EOF y estado == 0: no hay más tokens; terminar el listado
        columna = get_evento(c)
        (*proceso[estado][columna])()
        nuevo = nuevo_estado[estado][columna]
        si nuevo == -2
            informar error léxico con línea
            volcar a estado 0 y seguir   /* no abortar */
        si nuevo == -1
            si corresponde unread: unget(c)
            agregar {token} al listado
            retornar
        estado = nuevo
        leer(c)
```

Conductor:

```
main:
    si fopen del fuente falla: error de apertura; salir
    mientras no se haya terminado el fuente
        reconocer_token()
    fclose
    mostrar listado de tokens
    si no hubo errores léxicos
        printf(" - Compilacion AL EXITOSA - ")
        mostrarTS()
    si no
        printf(" - Analisis Lexico completo con ERRORES - ")
```

`error == 1` es bandera de “hubo al menos un error”, no de abortar (consigna general 14).

---



## 10. Errores que emite esta fase


| Código | Condición                                                     | Mensaje (con línea)               |
| ------ | ------------------------------------------------------------- | --------------------------------- |
| E1     | `get_evento` sin columna, o carácter inválido en estado 0     | carácter fuera del alfabeto       |
| E2     | estado 1 sin `/` (entero suelto) o estado 2 sin dígito (`3/`) | constante mal formada             |
| E3     | `fin_cte` con denominador 0                                   | denominador cero                  |
| E4     | estado 21 y no-letra (salvo `"`)                              | cadena mal formada (error léxico) |


Un error **no** agrega un token ficticio al listado. Tras informar, estado 0 y el carácter siguiente.

---



## 11. Traza de verificación

Entrada `n = 0/1;`, estado inicial 0.

Listado esperado: `ID` `OP_ASIG` `CTE` `PUNTO_Y_COMA`


| Estado | Lee         | Evento | Acción        | Nuevo estado | Unread | Al listado     |
| ------ | ----------- | ------ | ------------- | ------------ | ------ | -------------- |
| 0      | `n`         | LETRA  | inicio_id     | 3            | no     | —              |
| 3      | espacio     | SPACE  | fin_id        | -1           | sí     | `ID`           |
| 0      | espacio     | SPACE  | nada          | 0            | no     | —              |
| 0      | `=`         | `=`    | nada          | 4            | no     | —              |
| 4      | espacio     | SPACE  | nada          | -1           | sí     | `OP_ASIG`      |
| 0      | espacio     | SPACE  | nada          | 0            | no     | —              |
| 0      | `0`         | DIGITO | inicio_cte    | 1            | no     | —              |
| 1      | `/`         | `/`    | continuar_cte | 2            | no     | —              |
| 2      | `1`         | DIGITO | continuar_cte | 22           | no     | —              |
| 22     | `;`         | `;`    | fin_cte       | -1           | sí     | `CTE`          |
| 0      | `;`         | `;`    | nada          | 16           | no     | —              |
| 16     | (siguiente) | …      | nada          | -1           | sí     | `PUNTO_Y_COMA` |


Otras entradas:


| Entrada            | Listado / efecto |
| ------------------ | ---------------- |
| `3/4`              | `CTE`            |
| `4;`               | E2; no hay `CTE` |
| `//x` otra palabra | listado vacío    |


---



## 12. Casos de prueba de esta fase

Cada caso: un archivo (o fragmento) → listado + errores.


| Entrada               | Listado / errores                      | Qué verifica                 |
| --------------------- | -------------------------------------- | ---------------------------- |
| `n = 0/1;`            | `ID`, `OP_ASIG`, `CTE`, `PUNTO_Y_COMA` | racional + listado           |
| `4`                   | E2, listado vacío                      | entero suelto                |
| `1/2`                 | `CTE`                                  | racional                     |
| `3/`                  | E2                                     | constante mal formada        |
| `5/0`                 | E3                                     | denominador cero             |
| `if`                  | `IF`                                   | `fin_id`                     |
| `"abc"`               | `CADENA`                               | estado 21                    |
| `"a b"`               | error léxico                           | espacio en cadena            |
| `acc`                 | `ID`                                   | letras                       |
| `//hola` + EOL + `+`  | `OP_SUMA`                              | comentario no tokeniza       |
| `@`                   | E1                                     | fuera de alfabeto            |
| `<=`                  | `OP_MENOR_IGUAL`                       | operadores de dos caracteres |
| `-3/4` desde estado 0 | `CTE`                                  | estado 7 → 1                 |


Programas de integración: `tests/correctos` y `tests/errores-lexicos-sintacticos` de spec 01. En esta etapa solo se exige el **listado léxico** y los errores E1–E4; no se valida sintaxis.

---



## 13. Criterios de aceptación

- Un fuente válido produce el listado de tokens en orden, con códigos de `01-diseno` §4, sin blancos ni comentarios.
- El reconocedor usa las dos matrices; no es un reconocedor ad hoc por `if` de token.
- Errores léxicos informan línea; la corrida continúa; el listado no incluye tokens inventados por el error.
- `mostrarTS()` coherente con consigna general 8 y spec 02.