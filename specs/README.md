# Specs

Especificaciones y bitácoras de cada etapa del compilador RG (Grupo F).

El desarrollo sigue **Spec-Driven Development**: la lógica del compilador se escribe primero acá; el código en `src/` se deriva de estas specs.

## Quién escribió las specs

| Etapa | Autor(es) | Fecha |
|-------|-----------|-------|
| 01-diseno | Grupo | 31/08/2026 |
| 02-tabla-simbolos | Grupo | 14/09/2026 |
| 03-analizador-lexico | Grupo| -|
| 04-analizador-sintactico | | |
| 05-errores | | |
| 06-codigo-intermedio | | |
| 07-codigo-assembler | | |

## Modelos usados

| Etapa | Modelo | Uso |
|-------|--------|-----|
| 01-diseno | Cursor Grok 4.6 | Redacción a partir de consignas, definición del lenguaje, matrices y teoría. |
| 02-tabla-simbolos | Cursor Grok 4.6 | Definición del contrato de la tabla de simbolos |
|  |

## Convención

Cada carpeta de etapa contiene:

- `spec.md` — qué se va a construir y bajo qué reglas (contrato de la etapa)
- `bitacora.md` — registro cronológico de decisiones, problemas y cambios