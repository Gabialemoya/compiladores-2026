# Bitácora — 05-codigo-intermedio

| Fecha      | Quién | Qué se hizo / decidió                                                                 | Notas                                                                                                      |
| ---------- | ----- | ------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------- |
| 26/10/2026 | Grupo | Paso 1: módulo `src/intermedio/` y `%union` de `parser.y` con `ref` y `operador`.     | Sin `%type` ni acciones. `out/tercetos.txt` se escribe al terminar `yyparse`, con encabezado y cero filas. |
| 26/10/2026 | Grupo | Si `emitir` supera `MAX_TERCETOS`, informa, levanta `hay_errores` y devuelve `-1`.    | Mismo criterio que la tabla de símbolos llena.                                                            |
| 26/10/2026 | Grupo | `imprimir_tercetos` muestra el nombre del símbolo en `REF_TS`, no el índice.          | Formato de §7. `REF_VACIO` y `TIPO_NINGUNO` se imprimen como `-`.                                          |
| 26/10/2026 | Grupo | Paso 2: acciones de `expresion`, `termino`, `factor` y `asignacion`.                  | `%type <ref>` en esos tres. `p1_suma` queda en cuatro tercetos: dos `ASIG`, `SUMA` y el `ASIG` del resultado. |
| 26/10/2026 | Grupo | `emitir` guarda `linea_anterior`, no `linea_actual`.                                  | Al reducir, el lookahead ya avanzó la línea. `yylex` sigue cargando solo `indice_ts`.                     |
| 26/10/2026 | Grupo | `factor : invocacion` devuelve `ref_vacia` hasta el paso 4.                           | El argumento igual emite sus operaciones. `r = cuadrado(3/2)` se imprime con origen `-`.                   |
| 26/10/2026 | Grupo | R3 no genera un terceto de inicialización en `0/1`.                                   | §7 no lo muestra. El `0/1` de una variable sin asignación queda para la memoria estática.                  |
| 26/10/2026 | Grupo | Paso 3: `write` de expresión emite `T_WRITE`; `write` de cadena emite `T_WRITE_STR`. | En `p1_suma` quedan en las líneas 8 y 9, después de la asignación de `c`.                                  |
| 26/10/2026 | Grupo | `return` asigna la expresión a `@ret_<funcion>` y después emite `T_RET` vacío.       | §5 deja `T_RET` sin operandos. La celda es la dirección fija del racional de retorno (R11, D12).          |
| 26/10/2026 | Grupo | El nombre de la función se guarda al reconocer el encabezado, antes del cuerpo.      | El `return` se reduce adentro. Sin eso, `@ret_` no sabría de qué función es.                              |
| 26/10/2026 | Grupo | Paso 4: una llamada asigna el argumento al parámetro, emite `CALL` a la entrada y copia `@ret_<funcion>` a `@tmpN`. | La invocación devuelve ese temporal. En `p5`, `cuadrado` empieza en `DIR:0`.                              |
| 26/10/2026 | Grupo | Si la función todavía no se vio, los tres tercetos quedan pendientes.                | Se completan al registrar el encabezado. Cubre la recursión y la llamada adelantada.                      |
