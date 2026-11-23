# Spec — Generación de assembler

**Depende de:** `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md` · `specs/05-codigo-intermedio/spec.md`
**Produce:** un archivo assembler ejecutable a partir de los tercetos.

## Objetivo

Traducir el código intermedio del programa a assembler. El programa generado debe realizar las operaciones indicadas por los tercetos y respetar el flujo del programa fuente.

El grupo eligió NASM, x86 de 32 bits y enlace con GCC. El archivo generado debe poder ensamblarse y ejecutarse en esa plataforma.

## Cuándo se genera

La generación se realiza una sola vez, cuando el análisis sintáctico terminó de armar la lista completa de tercetos. Se inicia desde la regla principal del programa. Las demás reglas construyen los tercetos y no generan assembler.

Si hay errores que impiden generar un programa válido, no se escribe un assembler nuevo.

## Cómo se genera

El generador recorre el código intermedio y traduce sus elementos a assembler según la operación que representa.

- **Operaciones aritméticas:** leen los dos valores, calculan el resultado y lo guardan en un espacio temporal. Los racionales se mantienen simplificados. Si el valor supera el rango permitido, se reduce su precisión y se informa una advertencia. Una división por cero informa un error durante la ejecución.
- **Asignaciones:** copian el valor de origen a la variable de destino.
- **Comparaciones:** comparan dos racionales sin convertirlos a punto flotante y producen un valor verdadero o falso.
- **Saltos:** cambian la ejecución al terceto indicado. Los saltos condicionales lo hacen según el resultado de una comparación.
- **Salida:** muestran el valor racional o el texto que indica el terceto.
- **Funciones:** copian el argumento al parámetro, transfieren la ejecución a la función y, al retornar, permiten continuar desde la llamada.
