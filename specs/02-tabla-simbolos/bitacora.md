# Bitácora — 02-tabla-simbolos


| **Fecha**  | **Quién** | **Qué se hizo / decidió**                                                                                                                      | **Notas**                                                                     |
| ---------- | --------- | ---------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------- |
| 14/09/2026 | Grupo     | Contrato de la tabla: nombre, tipo, valor (constantes) y longitud; exportación a `out/` al terminar la compilación                             | Consignas generales 8 y 9                                                     |
| 14/09/2026 | Grupo     | Tipo racional (par 32 bits) para `ID`/`CTE`; cadena para `CADENA` (máx. 20). Una sola tabla global                                             | Alineado a D1, D11 y D18 de diseño                                            |
| 14/09/2026 | Grupo     | Altas desde el léxico (`fin_id`, `fin_cte`, `fin_cadena`). Un `ID` se registra la primera vez que aparece; el `var` duplicado lo detecta R2/E7 | Aceptación: auditar constantes de `tests/correctos/` con el archivo exportado |
| 29/09/2026 | Grupo     | Renumeración del catálogo de errores: el error de variable redeclarada pasa de E6 a E7. La entrada del 14/09 queda con la numeración anterior | Ver bitácora de 01-diseño                                                     |


