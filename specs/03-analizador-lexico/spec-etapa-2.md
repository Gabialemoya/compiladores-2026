# Especificación Etapa 2 — Función `yylex()`

**Lenguaje RG — Grupo F — Desarrollo de Compiladores (UNO)**

> Alcance de este documento: únicamente la adaptación del analizador léxico de la etapa 1 para que pueda ser invocado por el analizador sintáctico. Las reglas gramaticales, las acciones y el manejo de conflictos se especifican en un documento aparte.

---

## 1. Objetivo

En la etapa 1 el analizador léxico era un programa autónomo: recorría el archivo completo en un bucle propio y emitía el listado de tokens. En la etapa 2 deja de ser un programa y pasa a ser un **servicio** que el analizador sintáctico consume.

La inversión del control es el cambio conceptual central:

| | Etapa 1 | Etapa 2 |
| --- | --- | --- |
| Quién controla el bucle | El léxico | El sintáctico (`yyparse`) |
| Cómo se recorre el fuente | Un bucle `while` propio | Una llamada por token |
| Qué produce | Un listado impreso | Un valor de retorno por llamada |
| Función de entrada | `main()` | `yylex()` |

El analizador léxico no decide cuándo leer: responde cuando el parser pide el token siguiente.

---

## 2. Contrato con el analizador sintáctico

`yylex()` debe cumplir exactamente esta firma y este comportamiento:

```c
int yylex(void);
```

| Aspecto | Requisito |
| --- | --- |
| Valor de retorno | Código del token reconocido |
| Fin de archivo | Retornar `0`. Es el valor que `yyparse` interpreta como fin de entrada |
| Error léxico | No retornar el código de error al parser. Reportar, recuperar y seguir buscando un token válido |
| Valor semántico | Cargar `yylval` **antes** de retornar |
| Reentrada | Cada llamada continúa la lectura donde terminó la anterior. El estado del archivo es global |

Además, el analizador sintáctico requiere que el léxico provea:

- `yyerror(const char* msg)`: función que el parser invoca al detectar un error sintáctico. Debe imprimir el mensaje junto con el número de línea.
- Número de línea accesible desde el parser, para ubicar los errores.

---

## 3. Codificación de los tokens

**Este es el punto que más impacta y conviene resolver primero.**

El generador de parsers produce sus propias constantes de token en el archivo de cabecera (`parser.tab.h`), a partir de las declaraciones `%token` de la gramática. El analizador léxico debe retornar **esos** valores, no los códigos 10, 20, 30 definidos en la etapa 1.

El motivo: los valores de 1 a 255 están reservados para representar caracteres literales, y el 0 está reservado para el fin de entrada. Los códigos actuales de RG van de 10 a 300, así que quedan dentro del rango reservado, salvo `CADENA`.

**Decisión tomada: opción A.** Declarar los tokens en la gramática sin número y dejar que el generador asigne los valores. El léxico incluye `parser.tab.h` y retorna las constantes simbólicas:

```c
#include "parser.tab.h"
...
if (strcmp(lexema, "if") == 0) return IF;
```

Los códigos didácticos 10 a 300 se conservan solo para el listado y la tabla de símbolos, mediante una función de traducción.

Los códigos didácticos dejan de usarse como valor de retorno, pero no desaparecen: siguen siendo la numeración oficial del lenguaje en la documentación, en el listado de tokens y en la columna `TIPO` de la tabla de símbolos. La traducción entre ambas numeraciones se concentra en una única función:

```c
/* Traduce el codigo interno del lexico (10, 20, 30...) al codigo que espera
   el parser. Es el unico lugar del programa donde conviven las dos
   numeraciones. */
int traducir_token(int codigo_interno) {
    switch (codigo_interno) {
        case 10:  return ID;
        case 20:  return CTE;
        case 30:  return OP_ASIG;
        /* ... una entrada por token ... */
        case 300: return CADENA;
        default:  return 0;
    }
}
```

**Alternativa descartada.** Asignar explícitamente los números en la gramática, arriba de 257, y renumerar todo el lenguaje. Mantiene una única numeración, pero obliga a rehacer la tabla de tokens, la matriz de tokens y la documentación de la etapa 1.

---

## 4. Valor semántico (`yylval`)

El código del token dice **qué** se reconoció; el valor semántico dice **cuál**. Sin él, el parser sabe que llegó un `ID` pero no cuál identificador.

Tres tokens necesitan valor semántico:

| Token | Valor a transmitir |
| --- | --- |
| `ID` | Índice del identificador en la tabla de símbolos |
| `CTE` | Índice de la constante en la tabla de símbolos |
| `CADENA` | Índice de la cadena en la tabla de símbolos |

El resto de los tokens no lleva valor: el código ya los identifica por completo.

Se propone transmitir el **índice de la tabla de símbolos** y no el lexema, para evitar copias de cadenas y porque la etapa siguiente va a necesitar ese índice de todos modos.

Declaración en la gramática:

```
%union {
    int indice_ts;
}
%token <indice_ts> ID CTE CADENA
```

Carga en el léxico, dentro de la acción de cierre correspondiente:

```c
yylval.indice_ts = insertar_ts_id(lexema);
```

**Cambio requerido en el código actual.** Las funciones `insertar_ts_id`, `insertar_ts_cte` e `insertar_ts_cadena` son `void`. Modificar las tres para que devuelvan el índice de la entrada, tanto si la insertan como si ya existía.

---

## 5. Cambios sobre el código de la etapa 1

### 5.1. Qué se conserva sin modificar

- Las tres matrices: `nuevo_estado`, `proceso` y `token_por_estado`.
- Las acciones semánticas de acumulación: `inicio_id`, `continuar_id`, `inicio_cte`, `continuar_cte`, `inicio_cadena`, `continuar_cadena`, `nada`.
- La función de clasificación `get_evento`.
- La estructura y las funciones de la tabla de símbolos, salvo el valor de retorno indicado en la sección 4.
- El mecanismo de retroceso con `ungetc`.

El autómata no cambia: cambia quién lo invoca.

### 5.2. Qué se transforma

| Elemento actual | Transformación |
| --- | --- |
| `int reconocer_token(void)` | Pasa a ser el cuerpo de `int yylex(void)` |
| `return 1` al cerrar un token | Pasa a `return <código del token>` |
| `return 0` al llegar a EOF | Se mantiene: es el valor que espera el parser |
| `agregar_token_listado(...)` | Se mantiene, pero como registro opcional, no como salida principal |
| `main()` del léxico | Se elimina. El `main` pasa al programa del parser |
| Apertura del archivo fuente | Se traslada al `main` del parser, antes de llamar a `yyparse()` |

### 5.3. Qué se agrega

- `yyerror(const char* msg)`.
- El archivo de cabecera `lexico.h` con los prototipos que el parser necesita.
- La función `traducir_token`, según la decisión de la sección 3.
- La normalización de las constantes racionales, detallada en la sección 5.4.

### 5.4. Normalización de las constantes racionales

La función `fin_cte` registra la constante tal como fue escrita, de modo que `2/4` y `1/2` generan dos entradas distintas en la tabla de símbolos pese a representar el mismo valor.

Normalizar en el analizador léxico, por dos motivos:

- El caso de prueba F.6 exige que un resultado simplificable se muestre normalizado.
- Sin normalizar, la tabla de símbolos duplica entradas y la comparación de constantes en las etapas siguientes deja de ser directa.

Aplicar dos operaciones sobre el par numerador y denominador, después de validar que el denominador no sea cero y antes de insertar en la tabla:

1. **Signo.** Si el denominador es negativo, invertir el signo de ambos, de modo que el signo quede siempre en el numerador.
2. **Simplificación.** Dividir numerador y denominador por su máximo común divisor.

```c
/* Maximo comun divisor por el algoritmo de Euclides. */
long long mcd(long long a, long long b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        long long t = b;
        b = a % b;
        a = t;
    }
    return a;
}
```

Dentro de `fin_cte`, entre la validación del denominador y la inserción:

```c
if (den < 0) { num = -num; den = -den; }   /* Signo siempre en el numerador */
long long d = mcd(num, den);
if (d > 1) { num /= d; den /= d; }
```

Casos que resuelve: `2/4` queda `1/2`, `0/5` queda `0/1`, `3/-6` queda `-1/2`.

El lexema original se pierde en la tabla de símbolos, que pasa a guardar la forma normalizada. Es el comportamiento buscado, dado que la tabla representa el valor y no el texto.

---

## 6. Estructura de archivos

```
lexico.h        prototipos de yylex, yyerror y la tabla de simbolos
lexico.c        matrices, acciones semanticas y yylex()
tabla_simbolos.c  (opcional) la tabla de simbolos separada
parser.y        gramatica, declaraciones %token y main()
```

Secuencia de compilación:

```
bison -d parser.y          genera parser.tab.c y parser.tab.h
gcc parser.tab.c lexico.c -o compilador
```

`lexico.c` incluye `parser.tab.h` para conocer las constantes de token. La dependencia es en un solo sentido: el léxico conoce los códigos del parser, el parser no conoce el interior del léxico.

---

## 7. Pseudocódigo de `yylex()`

```
funcion yylex() -> entero

    estado = 0
    token_actual = -1
    cancelar_token = 0

    repetir

        leer caracter
        si es fin de linea, incrementar contador de linea

        si es fin de archivo:
            si estado = 0 o estado = 10:
                retornar 0            // fin de entrada para el parser
            sino:
                columna = SPACE       // fuerza el cierre del ultimo token

        sino:
            columna = clasificar(caracter)
            si columna = -1:
                reportar error E1
                estado = 0
                continuar             // NO retornar: seguir buscando token valido

        ejecutar accion semantica [estado][columna]
        destino = nuevo_estado[estado][columna]

        si destino = -2:
            reportar el error segun el estado
            devolver el caracter al flujo
            estado = 0
            continuar                 // NO retornar

        si destino = -1:
            devolver el caracter al flujo, salvo las excepciones de la seccion 9
            si cancelar_token:
                cancelar_token = 0
                estado = 0
                continuar
            si token_actual = -1:
                token_actual = token_por_estado[estado]
            cargar yylval si el token lo requiere
            registrar en el listado
            retornar traducir(token_actual)

        estado = destino

    fin repetir
```

La diferencia estructural respecto de la etapa 1 está en los `continuar`: ante un error, la función **no retorna**. Sigue leyendo hasta encontrar un token válido o el fin del archivo. El parser nunca debe recibir un código que no corresponda a un token del lenguaje.

---

## 8. Manejo de errores

### 8.1. Errores léxicos

Se reportan por pantalla igual que en la etapa 1, con código y número de línea, y el análisis continúa. El token erróneo no se entrega al parser.

**Consecuencia a documentar:** ante un error léxico, el parser recibe una secuencia incompleta y probablemente reporte además un error sintáctico derivado. Definir si en el informe final se muestran ambos o solo el primero.

**Corrección pendiente de la etapa 1: el carácter que provoca el error se pierde.**

En la rama de cierre de token (`destino = -1`) el código devuelve al flujo el carácter que cerró el token, mediante `ungetc`. En la rama de error (`destino = -2`) no lo hace: ejecuta `estado = 0; continue;` y el carácter queda consumido.

Ejemplo con `n = 0;`, que en RG es un error porque toda constante debe escribirse como racional. La secuencia de tokens emitida es:

```
ID  OP_ASIG  WRITE  ...
```

El punto y coma desapareció junto con el error. Al leerlo estando en el estado 1, el autómata fue a error y descartó el carácter, de modo que nunca se reconoció como `PUNTO_Y_COMA`.

En la etapa 1 esto solo ensuciaba el listado. Con el parser en línea, el analizador sintáctico recibe `n = write` y reporta un error de sintaxis que no existe en el programa fuente: el usuario ve dos errores por una sola causa.

Corregir agregando el retroceso en la rama de error, con las mismas precauciones que en la rama de cierre:

```c
if (c_actual != EOF) {
    ungetc(c_actual, archivo);
    if (c_actual == '\n') linea_actual--;
}
```

Con la corrección aplicada, la secuencia pasa a ser:

```
ID  OP_ASIG  PUNTO_Y_COMA  WRITE  ...
```

El error léxico se sigue reportando, pero el parser recibe la sentencia completa y no encadena un error propio.

No genera bucle infinito: el carácter devuelto se vuelve a leer desde el estado 0, y la fila 0 de la matriz de transiciones no tiene ninguna celda de error.

### 8.2. Errores sintácticos

`yyerror` es invocada por el parser. Implementación mínima:

```c
void yyerror(const char* msg) {
    printf("Error sintactico en linea %d: %s\n", linea_actual, msg);
    hubo_error = 1;
}
```

El contador de línea ya existe y es global, así que no requiere trabajo adicional.

### 8.3. Unificación de códigos

La numeración de errores del código actual no coincide con la de la especificación de la etapa 1: el código usa E2 para constante mal formada, E3 para denominador cero y E4 para cadena, mientras que el documento define E2 como falta de dígito, E3 como constante sin barra y E4 como denominador cero. Unificar antes de continuar, en cualquiera de los dos sentidos.

---

## 9. Retroceso de caracteres

Se mantiene el criterio de la etapa 1: al cerrar un token se devuelve al flujo el carácter que lo cerró, con dos excepciones.

| Excepción | Motivo |
| --- | --- |
| Comilla de cierre (estado 21, columna `"`) | Pertenece al lexema de la cadena |
| Fin de línea del comentario (estado 10) | Se consume como cierre del comentario |

Al devolver un fin de línea debe decrementarse el contador de líneas, porque se vuelve a leer en la llamada siguiente y se contaría dos veces.

**Decisión registrada:** el retroceso se resuelve dentro del bucle y no con una cuarta matriz, dado que las excepciones son solo dos.

---

## 10. Salidas

El listado de tokens deja de ser la salida principal, pero se conserva para verificar que el léxico sigue comportándose igual que en la etapa 1.

| Archivo | Contenido | Cuándo se escribe |
| --- | --- | --- |
| `out/listado.txt` | Un token por línea | En cada retorno de `yylex` |
| `out/tabla_simbolos.txt` | Nombre, tipo, valor y longitud | Al finalizar `yyparse` |

Emplear rutas relativas. El código de la etapa 1 tiene rutas absolutas que impiden compilarlo en otro equipo.

---

## 11. Pruebas de la etapa

### 11.1. Prueba de equivalencia

Compilar `lexico.c` con un `main` de prueba que invoque `yylex()` en un bucle hasta recibir `0`, imprimiendo cada token. La salida debe ser idéntica a la del listado de la etapa 1 para los cuatro programas P1, P4, P5 y P6.

Es la prueba más importante: verifica que la adaptación no alteró el reconocimiento.

### 11.2. Prueba de integración

Con el parser ya generado, verificar para cada programa de prueba que:

- Se consume el archivo completo, o sea que `yylex` llega a retornar `0`.
- `yylval` llega cargado en las reglas que usan `ID`, `CTE` y `CADENA`.
- Un error léxico no interrumpe el análisis sintáctico.

### 11.3. Prueba de normalización

Verificar en la tabla de símbolos que `2/4` se registra como `1/2`, que `0/5` se registra como `0/1` y que dos constantes equivalentes escritas de distinta forma comparten una única entrada.

### 11.4. Prueba de valor semántico

Sobre P6, verificar que los índices recibidos para `cuadrado`, `mostrar`, `x`, `v` y `r` corresponden a las entradas correctas de la tabla de símbolos, y que una misma variable usada dos veces devuelve el mismo índice.

---

## 12. Decisiones

### 12.1. Decisiones tomadas

| Punto | Decisión | Sección |
| --- | --- | --- |
| Codificación de tokens | Delegar la numeración en el generador. Los códigos 10 a 300 se conservan solo para documentación y salidas | 3 |
| Contenido de las cadenas | Se mantiene el criterio actual: una cadena admite únicamente letras | 12.2 |
| Normalización de racionales | Simplificar en `fin_cte`, antes de insertar en la tabla de símbolos | 5.4 |
| Retroceso de caracteres | Resuelto dentro del bucle, sin cuarta matriz | 9 |
| Retroceso ante error léxico | Agregar el `ungetc` en la rama de error | 8.1 |

### 12.2. Consecuencia de mantener el alfabeto de las cadenas

Una cadena admite letras y nada más: ni espacios, ni dígitos, ni signos de puntuación. Los programas de prueba deben respetarlo. En particular, el caso P1 debe escribirse como `write("Resultado")` y no `write("Resultado:")`, que produce dos errores léxicos.

Registrar esta restricción en la definición del lenguaje, porque no es evidente para quien lea únicamente la gramática.

### 12.3. Decisiones abiertas

1. Unificación de los códigos de error entre el documento y el código. Sección 8.3.
2. Comportamiento del informe ante un error léxico seguido de su error sintáctico derivado. Sección 8.1.