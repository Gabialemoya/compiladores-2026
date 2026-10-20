# Bitácora — 04-analizador-sintactico

| Fecha      | Quién | Qué se hizo / decidió                                                                                        | Notas                                                                                                            |
| ---------- | ----- | ------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------- |
| 05/10/2026 | Grupo | La gramática de `parser.y` es la de `01-diseno` §6, sin cambios.                                             |                                                                                                                  |
| 05/10/2026 | Grupo | Las acciones semánticas quedan vacías. La recuperación ante error sintáctico queda para una etapa posterior. | Por eso `el2_constante` informa el primer E2 y el E5 derivado, y el parser no sigue leyendo el resto del fuente. |
| 13/10/2026 | Grupo | Se documentó `lexico.h` en la spec §8. | Cabecera escrita a mano: el parser y el léxico se compilan por separado. Publica `yylex`, `yyerror`, `fuente`, `hay_errores` y `mostrarTS`. |
| 13/10/2026 | Grupo | Se sumó `p7_condiciones` a los programas correctos (§9.1). Pasan de seis a siete. | Cubre `seleccion` sin `else`. `p4_if_else` ya cubre el `if` con `else`. |
| 13/10/2026 | Grupo | Cada alternativa de §5 lleva el número de regla de Bison, del 1 al 49. | Es el número de `parser.output` y el de la traza (`Reducing stack by rule N`). La 0 es `$accept: programa $end` y no está en la gramática. |
| 13/10/2026 | Grupo | Cada alternativa de `parser.y` tiene un comentario con la construcción que reconoce. | Las producciones no cambian: siguen siendo las de `01-diseno` §6. |
| 13/10/2026 | Grupo | Se activó la traza de Bison: `%define parse.trace` y `yydebug = 1` antes de `yyparse`. | Al correr `compilador.exe` se ven los shift y las reducciones por la salida de error. |


