# Specs

Especificaciones y bitácoras de cada etapa del compilador RG (Grupo F).

El desarrollo sigue **Spec-Driven Development**: la lógica del compilador se escribe primero acá; el código en `src/` se deriva de estas specs.

## Quién escribió las specs


| Etapa                                  | Autor(es) | Fecha      |
| -------------------------------------- | --------- | ---------- |
| 01-diseno                              | Grupo     | 31/08/2026 |
| 02-tabla-simbolos                      | Grupo     | 14/09/2026 |
| 03-analizador-lexico (etapa 1 listado) | Grupo     | 20/09/2026 |
| 03-analizador-lexico (etapa 2 lexer)   | Grupo     | 28/09/2026 |
| 04-analizador-sintactico               | Grupo     | 05/10/2026 |

## Modelos usados


| Etapa                | Modelo          | Uso                                                                          |
| -------------------- | --------------- | ---------------------------------------------------------------------------- |
| 01-diseno            | Cursor Grok 4.6 | Redacción a partir de consignas, definición del lenguaje, matrices y teoría. |
| 02-tabla-simbolos    | Cursor Grok 4.6 | Definición del contrato de la tabla de simbolos                                                                                                          
| 03-analizador-lexico | Cursor Grok 4.6 | Etapa 1 listado de tokens                                                    |
| 04-analizador-sintactico | Antigravity | Generar parser.y y compilador                                                |



## Convención

Cada carpeta de etapa contiene:

- `spec.md` — qué se va a construir y bajo qué reglas (contrato de la etapa)
- `bitacora.md` — registro cronológico de decisiones, problemas y cambios

