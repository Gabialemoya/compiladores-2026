# Spec — Generación de código intermedio de RG

**Depende de:** `specs/01-diseno/spec.md` · `specs/02-tabla-simbolos/spec.md` · `specs/04-analizador-sintactico/spec.md`  
**Entrada:** el programa reconocido por el parser.  
**Salida:** una lista de tercetos en `out/tercetos.txt`.

## Objetivo

Representar las operaciones y el orden de ejecución del programa en una lista de tercetos. Cada terceto indica una acción que las etapas posteriores traducirán a assembler. En esta etapa no se ejecutan las operaciones ni se genera assembler.

## Cómo se va armando la lista

El parser reconoce el programa usando las reglas de la gramática. Cada vez que reconoce una parte que requiere una acción —por ejemplo, una suma, una asignación o una comparación— ejecuta la acción asociada y agrega el terceto correspondiente al final de la lista.

La lista se arma en el orden en que se reconocen las operaciones. Si una operación usa el resultado de otra, guarda como operando el número del terceto que produjo ese resultado. Así, cada resultado queda disponible para los tercetos que se agreguen después.

Por ejemplo, para `c = a + b * d;`, primero se representa la multiplicación porque tiene mayor precedencia. Después se representa la suma y, por último, la asignación:

| Orden | Terceto | Qué representa |
|---|---|---|
| 1 | `MUL b, d` | Multiplica `b` por `d`; el resultado queda en este terceto |
| 2 | `SUMA a, resultado del terceto 1` | Suma `a` al resultado anterior |
| 3 | `ASIG c, resultado del terceto 2` | Guarda el resultado final en `c` |

Los índices concretos dependen de los tercetos que ya se hayan agregado al programa, incluido el salto inicial.

## Qué acción genera cada parte del programa

### Al comenzar el programa y reconocer funciones

Al comienzo se agrega un salto para que la ejecución entre por `main`, aunque haya definiciones de funciones antes. Cuando el parser reconoce el encabezado de una función, se registra dónde empieza su código y cuál es su parámetro. Al reconocer el encabezado de `main`, se completa el destino del salto inicial.

Si una llamada aparece antes de que el parser haya reconocido la definición de esa función, se guardan sus referencias y se completan cuando se encuentra el encabezado de la función.

### Al reconocer expresiones, variables y asignaciones

Cuando el parser reconoce una operación aritmética completa, agrega un terceto para esa operación. Las operaciones se agregan respetando la precedencia y los paréntesis de la gramática. El resultado de cada operación se identifica por la posición de su terceto.

Al reconocer un identificador o una constante como parte de una expresión, se toma su referencia de la tabla de símbolos; esa referencia por sí sola no genera un terceto. Al reconocer una declaración `var`, se marca la variable como declarada, pero tampoco se agrega un terceto.

Después de reconocer el lado derecho de una asignación, se agrega un terceto que indica que su valor debe copiarse a la variable del lado izquierdo.

### Al reconocer condiciones y saltos

Cuando se completa una comparación, se agrega un terceto que compara sus dos expresiones. El resultado es `-1` si la condición se cumple y `0` si no se cumple. A continuación se agrega un salto condicional, cuyo destino puede quedar pendiente hasta que se conozca el lugar al que debe llevar.

Para las condiciones unidas con `and` y `or`, se agregan saltos en el momento de pasar de la condición izquierda a la derecha. Esto permite continuar por la rama correcta sin evaluar condiciones que ya no hacen falta.

En un `if`, después de reconocer la condición y antes del bloque, se preparan los destinos de sus saltos. Si hay `else`, al terminar el primer bloque se agrega un salto para evitar que la ejecución entre también en el `else`. Los destinos pendientes se completan cuando el parser reconoce el comienzo o el final de cada bloque.

En `loop ... until`, antes del cuerpo se guarda la posición de su primer terceto. Después de reconocer la condición final, se completan los saltos: si la condición todavía no se cumple, la ejecución vuelve al comienzo del cuerpo; si se cumple, continúa después del ciclo.

### Al reconocer salidas y funciones

Al completar `write(expresión)` se agrega un terceto de salida para el valor. Al completar `write("texto")` se agrega otro tipo de terceto para el texto.

Al reconocer una llamada, primero se copia el argumento al espacio del parámetro, luego se agrega el terceto de llamada y, al volver, se copia el valor devuelto a un espacio temporal. Si se reconoce `return expresión`, se copia la expresión al espacio de retorno de la función y se agrega el terceto de retorno.

## Acciones dentro de las reglas

Las acciones se escriben en las reglas de la gramática. Normalmente se ubican al final de la regla, cuando ya se reconocieron todos los elementos necesarios para agregar el terceto. En las estructuras de control también hay acciones entre partes de una regla: se usan cuando es necesario preparar o completar un salto antes de seguir con el bloque o la condición siguiente.

```text
expresion : expresion OP_SUMA termino
            { agregar el terceto de suma }

asignacion : ID OP_ASIG expresion PUNTO_Y_COMA
             { agregar el terceto de asignación }

condicion : expresion op_comparacion expresion
            { agregar la comparación y su salto condicional }

seleccion : IF PAR_ABRE condiciones PAR_CIERRA
            { preparar los saltos de la condición }
            bloque
          | IF PAR_ABRE condiciones PAR_CIERRA
            { preparar los saltos de la condición }
            bloque ELSE
            { dirigir el caso falso al bloque ELSE }
            bloque
            { completar el salto al final }

iteracion : LOOP
            { guardar el comienzo del cuerpo }
            bloque UNTIL PAR_ABRE condiciones PAR_CIERRA
            { completar los saltos para repetir o salir }
```

Los textos entre llaves describen la acción que debe realizarse; no son código C. La gramática completa se mantiene en la spec de diseño.

## Controles semánticos

Mientras agrega los tercetos, esta etapa verifica que las variables utilizadas estén declaradas, que no se declaren dos veces y que una función no se llame directamente a sí misma. Informa los errores semánticos correspondientes y continúa construyendo la lista.

## Resultado y comprobación

Al terminar el análisis del programa, la lista se escribe en `out/tercetos.txt`, incluso si se detectaron errores semánticos. Los destinos de saltos y llamadas deben quedar completos cuando se conozca la parte correspondiente del programa.

- Las operaciones aparecen en el orden definido por la precedencia y los paréntesis.
- Las asignaciones, comparaciones, salidas y llamadas representan las acciones del fuente.
- Los saltos de `if`, `else`, `and`, `or` y `loop ... until` llevan al lugar correcto.
- Los errores de declaración y recursión directa se informan sin detener la construcción de la lista.
- Los casos de prueba de RG producen los tercetos esperados.
