# Spec — 02-tabla-simbolos


**Depende de:** `specs/01-diseno/spec.md`

## Objetivo

Registrar cada símbolo que el compilador necesita consultar después del reconocimiento léxico y en semántica / generación de código.

## Alcance

La tabla registra **como mínimo** nombre, tipo, valor (para constantes) y longitud, y se **exporta a un archivo** al finalizar la compilación.

En RG el tipo de dato de variables y constantes numéricas es el racional (par de enteros 32 bits). 

Las cadenas tienen una longitud máxima de 20 caracteres.

## Entradas / Salidas

- Entrada: desde `fin_id`, `fin_cte`, `fin_cadena` (`specs/03-analizador-lexico/spec.md`) y, más adelante, declaraciones (`var`) en el sintáctico (`specs/04-analizador-sintactico/spec.md`).
- Salida: estructura en memoria + archivo en `out/`.

## Reglas y decisiones de diseño

| Campo exigido | Uso en RG |
|---|---|
| nombre | lexema del `ID` o nombre de constante |
| tipo | racional o cadena, según el token |
| valor | par (numerador, denominador) en `CTE`; texto en `CADENA` |
| longitud | del lexema / de la cadena |

Alcance: una sola tabla global (No existen las variables locales).
