# Spec — Análisis léxico de RG (etapa 2: integración con el parser)

**Depende de:** `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md` · `specs/03-analizador-lexico/spec-etapa-1.md`  
**Entrada:** el archivo fuente abierto por el programa principal del parser.  
**Salida:** los tokens que solicita el analizador sintáctico.

## Objetivo

Integrar el analizador léxico con Bison. En esta etapa el parser controla la lectura del programa y solicita al léxico un token cada vez que necesita continuar.

## Cómo funciona

Cuando Bison solicita un token, el analizador léxico lee el archivo hasta reconocer uno y lo devuelve. Para identificadores, constantes y cadenas, también entrega el índice de la entrada correspondiente en la tabla de símbolos.

### Reconocimiento de tokens con las tablas

El reconocimiento usa el mismo proceso que en la etapa 1. El analizador comienza en el estado inicial y lee un carácter. Identifica el grupo del carácter —por ejemplo, letra, dígito o símbolo— y usa ese grupo como columna de las tablas.

Con el estado actual y esa columna, consulta la tabla de acciones (`proceso`) para saber qué hacer con el carácter, y la tabla de estados (`nuevo_estado`) para saber cómo continuar. Repite la consulta hasta que las tablas indican que encontró un token completo o un error. Si el carácter que hizo terminar el token pertenece al siguiente, se vuelve a leer para no perderlo.

Por ejemplo, al leer `n = 0/1;`, las tablas permiten reconocer `n` como identificador, `=` como asignación y `0/1` como constante. Al terminar de reconocer cada uno, el analizador entrega ese token a Bison. El punto y coma se conserva para reconocerlo cuando el parser vuelva a pedir el próximo token.

La diferencia con la etapa 1 es qué hace el analizador con el token completo: en la etapa 1 lo agrega al listado y sigue por su cuenta; en esta etapa devuelve un token a Bison y espera a que el parser solicite el siguiente.

El reconocimiento mantiene las reglas de la etapa 1: se usa el mismo autómata y se producen los mismos tokens para el mismo programa. Los códigos internos que espera Bison se relacionan con los códigos definidos para el lenguaje mediante la traducción prevista por esta etapa.

Al llegar al final del archivo, el léxico devuelve la señal de fin que espera Bison. Si encuentra un error léxico, lo informa con la línea, recupera el carácter cuando corresponda y sigue buscando un token válido. No devuelve un token de error al parser.

El parser comprueba si los tokens recibidos forman parte de la gramática de RG. El léxico no decide si la estructura del programa es sintácticamente correcta.

## Decisiones de esta etapa

- La función `yylex()` reconoce y devuelve un token por cada llamada del parser; al llegar al fin del archivo devuelve `0`.
- Los tokens `ID`, `CTE` y `CADENA` llevan como valor el índice de su entrada en la tabla de símbolos, no el texto del lexema.
- Los códigos numéricos de Bison se traducen desde los códigos del lenguaje en un único punto del programa.
- Ante un error léxico, el analizador informa y continúa buscando; no devuelve al parser un código de error.
- Las constantes racionales se registran normalizadas en la tabla de símbolos.
- El listado de tokens se conserva para compararlo con el de la etapa 1.
- La tabla de símbolos se exporta al terminar el análisis sintáctico.

## Resultados

El parser puede solicitar tokens hasta consumir el programa completo. El listado se guarda en `out/tokens.txt` y la tabla de símbolos en `out/ts.txt` al terminar el análisis sintáctico.

## Comprobación

- Para los mismos programas, las etapas 1 y 2 reconocen la misma secuencia de tokens.
- El parser recibe los tokens en el orden en que aparecen en el programa.
- El fin del archivo se comunica correctamente y no se interpreta como un token del lenguaje.
- Los errores léxicos informan la línea y no detienen la lectura.
- Los valores de identificadores, constantes y cadenas permiten al parser consultar la tabla de símbolos.
