# Bitácora — 04-analizador-sintactico

| Fecha      | Quién       | Qué se hizo / decidió                                 | Notas                       |
| ---------- | ----------- | ----------------------------------------------------- | --------------------------- |
| 05/10/2026 | Antigravity | Gramática inicial generada y procesada por Bison      | 0 conflictos reportados     |
| 05/10/2026 | Antigravity | Versión de GNU Bison fijada a 2.4.1                   | Compatibilidad del entorno  |
| 05/10/2026 | Antigravity | Uso de `%error-verbose` en vez de `%define parse.error verbose` | Se requiere por GNU Bison 2.4.1 |
| 05/10/2026 | Antigravity | Uso de `/* vacio */` en vez de `%empty`               | Se requiere por GNU Bison 2.4.1 |
| 05/10/2026 | Antigravity | Resolución de los `#include` mediante la CLI (`-I. -Isrc`) | Evita hardcodear rutas relativas en los `#include` (`lexico.h` y `parser.tab.h`) |
