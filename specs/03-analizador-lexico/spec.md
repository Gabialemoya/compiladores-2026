# Specs — Analizador léxico de RG

El léxico se especifica en **dos etapas**.


| Etapa | Spec                                 | Qué entrega                                                     |
| ----- | ------------------------------------ | --------------------------------------------------------------- |
| 1     | `[spec-etapa-1.md](spec-etapa-1.md)` | Archivo fuente → **listado de tokens** (programa independiente) |
| 2     | `[spec-etapa-2.md](spec-etapa-2.md)` | `yylex()` → **un token por llamada** (para el analizador sintáctico) |


**Depende de:** `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md`  
**Produce:** `src/lexico/`