# Bitácora — 03-analizador-lexico


| Fecha      | Quién | Qué se hizo / decidió                                 | Notas                       |
| ---------- | ----- | ----------------------------------------------------- | --------------------------- |
| 20/09/2026 | grupo | Alcance de esta entrega: archivo → listado de tokens. |                             |
| 20/09/2026 | grupo | Spec partida en etapa 1 (`spec-etapa-1.md`)           | `spec.md` queda como índice |
| 05/10/2026 | grupo | Renombrar tokens con prefijo `TK_`                    | Evita colisión con las constantes generadas por Bison |
| 05/10/2026 | grupo | Añadir `ungetc` en la rama de error léxico (`-2`)     | Evita consumir el carácter causante del error (ej. el `;` tras una mala CTE) |
| 05/10/2026 | grupo | Normalizar racionales en `fin_cte` antes de TS        | Signo en numerador, simplificación por MCD (Euclides) |
| 05/10/2026 | grupo | Exportación incondicional de la TS                    | Se exporta `out/ts.txt` haya errores o no, según consigna general 9 |
