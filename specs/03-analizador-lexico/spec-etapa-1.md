# Spec — Análisis léxico de RG (etapa 1: listado de tokens)

**Depende de:** `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md`  
**Entrada:** un archivo con un programa escrito en RG.  
**Salida:** listado de tokens y, cuando no hay errores léxicos, la tabla de símbolos.

## Objetivo

Construir un analizador léxico independiente que lea un programa RG y muestre los tokens que reconoce. En esta etapa no se integra con el parser ni entrega tokens de a uno a otro programa.

## Cómo funciona

El analizador abre el archivo y lo recorre de izquierda a derecha, carácter por carácter. Reconoce cada token con el autómata definido en la spec de diseño, implementado mediante las matrices de estados y acciones.

### Reconocimiento de tokens con las tablas

Para reconocer un token, el analizador comienza en el estado inicial y lee un carácter. Primero identifica a qué grupo pertenece el carácter —por ejemplo, letra, dígito o símbolo—. Ese grupo indica qué columna consultar en las tablas.

Con el estado actual y la columna del carácter, consulta dos tablas:

1. La tabla de acciones (`proceso`) indica qué hacer con el carácter: por ejemplo, empezar a formar un identificador o agregar un carácter al token actual.
2. La tabla de estados (`nuevo_estado`) indica a qué estado pasar para seguir leyendo.

El analizador repite estos pasos hasta que las tablas indican que el token terminó o que hubo un error. Si el carácter que marca el final no pertenece al token, se vuelve a leer al comenzar el siguiente. Así no se pierde el primer carácter del próximo token.

Por ejemplo, al leer `n = 0/1;`, la letra `n` inicia un identificador. Cuando aparece el espacio, las tablas indican que el identificador terminó; se agrega `ID` al listado y el espacio se descarta. Luego se reconoce `=`, y después `0/1` como una constante. El `;` que marca el final de la constante se vuelve a leer para reconocerlo como el próximo token.

En esta etapa, el analizador repite el reconocimiento hasta terminar el archivo y va agregando cada token al listado.

Cada identificador, constante racional o cadena reconocida se registra en la tabla de símbolos. Las palabras reservadas se distinguen al reconocer los identificadores. Los espacios, tabuladores, saltos de línea y comentarios de línea se descartan y no aparecen como tokens.

Si encuentra un carácter o una secuencia incorrecta, informa el error léxico y la línea. Luego recupera el análisis y continúa con el resto del archivo. No agrega un token inventado por el error.

## Decisiones de esta etapa

- El analizador funciona como un programa independiente y controla su propio recorrido del archivo.
- Produce el listado completo de tokens; no implementa todavía la función que solicitará tokens desde el parser.
- Usa el autómata por matrices definido en la etapa de diseño.
- Los códigos de tokens son los definidos para el lenguaje en la etapa de diseño.
- Los errores léxicos E1–E4 se informan sin detener el análisis.
- El fin del archivo no se agrega al listado como token.

## Resultados

El listado se muestra en pantalla y se guarda en `out/tokens.txt`. Si no hubo errores léxicos, la tabla de símbolos se muestra y se guarda en `out/ts.txt`.

## Comprobación

- Los tokens aparecen en el mismo orden en que se encuentran en el programa.
- Los espacios, saltos de línea y comentarios no aparecen en el listado.
- Los errores E1–E4 informan la línea y el análisis continúa.
- Los programas sin errores producen un listado correcto y la tabla de símbolos.

