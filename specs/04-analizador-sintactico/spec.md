# Spec — Análisis sintáctico de RG

**Depende de:** `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md` · `specs/03-analizador-lexico/spec-etapa-2.md`  
**Entrada:** los tokens que entrega el analizador léxico.  
**Produce:** la validación sintáctica del programa y el punto de integración para las etapas siguientes.

## Objetivo

Comprobar que los tokens de un programa estén organizados de acuerdo con la gramática de RG. El análisis sintáctico se implementa con Bison en C.

## Cómo funciona

El programa principal abre el archivo fuente e inicia el parser. El parser solicita tokens al analizador léxico, uno por vez. Para cada token recibido, comprueba si puede continuar de acuerdo con las reglas gramaticales de RG.

El parser acepta el programa cuando llega al final del archivo y la secuencia completa respeta la gramática. Si encuentra una secuencia que no cumple las reglas, informa un error sintáctico E5 e indica la línea correspondiente. Luego intenta continuar el análisis para poder informar otros errores, según lo requiere la consigna general.

Las reglas que reconoce son las definidas en la spec de diseño. Esta etapa valida la estructura del programa; los controles semánticos y la generación de código intermedio corresponden a etapas posteriores.

## Decisiones de esta etapa

- Se usa Bison con acciones escritas en C.
- El archivo de entrada de Bison es `src/parser/parser.y`.
- El parser usa la función del analizador léxico para obtener tokens; no vuelve a reconocer caracteres del archivo.
- Las producciones siguen la gramática de la spec de diseño.
- Los tokens `ID`, `CTE` y `CADENA` llevan el índice de su entrada en la tabla de símbolos.
- Los errores sintácticos se informan como E5 con la línea del programa fuente.
- La tabla de símbolos se exporta al terminar el análisis.
- La gramática debe generar un parser sin conflictos no justificados.

## Resultado

Al finalizar, el programa informa si la compilación terminó sin errores o si se encontraron errores. El parser no cambia el texto fuente ni genera assembler. Deja establecido el lugar donde las etapas posteriores incorporan sus acciones.

## Comprobación

- Los programas válidos de prueba llegan al final y son aceptados por la gramática.
- Los programas con errores sintácticos informan E5 con la línea correspondiente.
- Ante un error, el análisis intenta continuar para detectar otros errores, de acuerdo con la consigna general.
- Los tokens con información asociada permiten consultar la tabla de símbolos.
- Bison genera el parser sin conflictos pendientes de justificar.