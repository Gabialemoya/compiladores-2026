# Compilador 2026

## Integrantes

| Nombre        | Legajo | Rol |
| ------------- | ------ | --- |
| Gabriela Moya | —      | —   |
| Rocio Pepek   | —      | —   |

## TP asignado

<details>
<summary>Consignas Generales</summary>

**Cómo leer este documento.** Estas consignas rigen para todos los grupos por igual y son de cumplimiento obligatorio. Cada grupo debe cumplir, además, los requisitos de su documento particular (Grupo A, B, C, D, E o F). Ante cualquier duda de interpretación, consultar antes de la primera entrega.

### 1. Herramientas y lenguaje de implementación

1. El compilador se desarrolla con el desarrollo manual del analizador léxico, y integración con herramienta YACC para análisis sintáctico.
2. El lenguaje de las acciones semánticas es a elección: C o Java (recomendados). Permitidos: C++, C#, Pascal, Delphi, Go, Rust, Swift, Python.
3. Cada grupo define su propio lenguaje fuente: alfabeto, tabla de tokens, palabras reservadas y gramática independiente del contexto.

### 2. Estructura del lenguaje fuente

4. El programa principal es también una función, pero identificada con un nombre reservado del lenguaje. No existe un bloque "programa principal" sintácticamente distinto de una función: la unidad de compilación es un conjunto de funciones, una de las cuales tiene el nombre reservado y es el punto de entrada.
5. Todo lo relacionado con las estructuras de control (`if`, `while`, `loop until` o cualquier otra) queda definido en la consigna particular de cada grupo.
6. El lenguaje debe admitir comentarios (de línea y/o de bloque, a elección) y el analizador léxico debe descartarlos.
7. Debe existir al menos una sentencia de salida que permita visualizar el valor de una variable o de una expresión, y una sentencia que permita emitir un literal de texto (para rotular la salida).

### 3. Tabla de símbolos

8. Debe construirse una tabla de símbolos que registre, como mínimo: nombre, tipo, valor (para constantes) y longitud.
9. La tabla de símbolos debe exportarse a un archivo al finalizar la compilación, para su control durante la corrección.

### 4. Generación de código

10. La salida del compilador debe ser un programa en lenguaje Assembler.
11. El Assembler generado debe ensamblarse y ejecutarse efectivamente en alguna plataforma. La plataforma y el ensamblador son a elección del grupo (por ejemplo x86 con MASM o NASM), pero deben informarse al inicio.
12. Debe generarse el archivo intermedio de tercetos (o la representación intermedia que el grupo elija), y presentarse junto con el Assembler.

### 5. Manejo de errores

13. El compilador debe informar errores léxicos, sintácticos y semánticos, indicando en cada caso la línea del programa fuente.
14. Un error no debe abortar la compilación en el primer hallazgo: debe intentarse la recuperación para reportar la mayor cantidad de errores posible en una sola corrida.

### 6. Entregas

El trabajo se divide en tres entregas:

| # | Entrega | Contenido |
| - | ------- | --------- |
| 1 | Documentación y diseño | Alfabeto del lenguaje, tabla de tokens, autómata finito, matriz de transiciones, matriz de nuevos estados, gramática independiente del contexto, programas fuente de ejemplo. |
| 2 | Analizador léxico y sintáctico | Ejecución del compilador sobre programas fuente sintácticamente correctos e incorrectos, mostrando el reporte de errores. |
| 3 | Compilador completo | Assembler generado, ensamblado y ejecutado, sobre la batería de casos de prueba obligatorios de la consigna del grupo. |

### 7. Casos de prueba obligatorios

Cada grupo debe entregar, como mínimo:

- 3 programas fuente correctos que ejerciten todas las estructuras de control del lenguaje.
- 3 programas fuente con errores léxicos y sintácticos deliberados.
- 2 programas fuente con errores semánticos deliberados, según las reglas semánticas propias del grupo.

A esta batería se suman los casos de prueba específicos indicados en el documento de cada grupo.

</details>


<details>
<summary>Grupo F</summary>

Estas consignas se complementan con el documento *Consignas Generales*, de cumplimiento obligatorio.

**Eje del trabajo:** tipo racional con simplificación y manejo de overflow por degradación controlada de la precisión.

### F.1 — Tipos de datos

1. El lenguaje maneja números racionales representados como pares ordenados de dos enteros de 32 o 64 bits (numerador y denominador). El grupo elige el tamaño y lo informa.
2. Los literales del lenguaje deben permitir escribir un racional (por ejemplo `3/4`) y también un entero suelto, que se interpreta como `n/1`.
3. Reglas de forma normal, que toda variable racional debe cumplir en todo momento:
   - Simplificación por factores comunes en numerador y denominador: tras cada operación debe dividirse el par por el máximo común divisor.
   - El denominador es siempre positivo; el signo se lleva en el numerador.
   - El cero se representa como `0/1`.
   - Denominador cero: es un error. El grupo decide si lo detecta en tiempo de compilación (para literales) o de ejecución (para expresiones), pero debe cubrir ambos casos.
4. Deben implementarse las operaciones `+`, `-`, `*`, `/` sobre racionales, con simplificación posterior obligatoria.
5. Las comparaciones deben ser exactas y realizarse sin convertir a punto flotante (comparación cruzada de productos, cuidando el signo). El resultado de toda comparación es el entero `0` (falso) o `-1` (verdadero).
6. La sentencia de salida debe mostrar el valor en formato `numerador/denominador` ya simplificado.

### F.2 — Overflow por pérdida de precisión

7. El control de overflow se realiza mediante pérdida de precisión en el numerador o el denominador: cuando una operación produce un par que excede el rango del entero elegido, el programa no se cancela; en su lugar, el resultado se sustituye por una aproximación racional representable y la ejecución continúa.
8. Algoritmo a implementar: dividir numerador y denominador por una misma potencia de 2 (con redondeo del numerador) hasta que ambos entren en rango, y volver a simplificar por MCD.
9. Recomendación de diseño: simplificar de forma cruzada antes de multiplicar reduce drásticamente la frecuencia con que se llega a la pérdida de precisión.
10. El programa debe advertir (sin cancelar) cuando se produjo una pérdida de precisión, indicando la línea de la operación afectada.

### F.3 — Declaración y alcance

11. Todas las variables deben ser declaradas. No se exige orden entre la declaración y el primer uso.
12. No existen variables locales (ver F.5): todas las variables son globales y de vida útil igual a la del programa.

### F.4 — Estructuras de control

13. El lenguaje debe poseer `if` con `else` y `loop ... until` (iteración con evaluación de la condición al final, que ejecuta el cuerpo al menos una vez).
14. Las estructuras de control deben poder anidarse una cantidad arbitraria de veces.
15. Las condiciones admiten comparaciones ligadas por `and` y por `or`, con uso de paréntesis.

### F.5 — Funciones y gestión de memoria

16. El lenguaje admite funciones o subrutinas SIN variables locales, sobre memoria estática. Toda variable que use una función es global.
17. Las funciones admiten un parámetro racional pasado por valor, alojado en una posición estática fija asignada en tiempo de compilación (no en una pila). Las funciones devuelven un valor racional.
18. Como el parámetro ocupa una dirección fija, la recursión no está permitida. El compilador debe detectar la llamada recursiva directa y reportarla como error semántico.

### F.6 — Casos de prueba específicos

Además de la batería obligatoria indicada en las Consignas Generales, deben entregarse:

- Un programa que sume `1/2 + 1/3` y muestre `5/6` (verifica simplificación y común denominador).
- Un programa que produzca un resultado simplificable (por ejemplo `2/4`) y lo muestre normalizado.
- Una iteración que acumule sumas de racionales hasta forzar el crecimiento del denominador y disparar la pérdida de precisión, mostrando la advertencia.
- Un programa con recursión directa, que debe ser rechazado por el compilador.

</details>

## Lenguaje

* **Lenguaje fuente:** RG
* **Lenguaje de implementación:** C
* **Arquitectura destino:** —
