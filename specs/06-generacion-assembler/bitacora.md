# Bitácora — 06-generacion-assembler

| Fecha      | Quién | Qué se hizo / decidió | Notas |
| ---------- | ----- | --------------------- | ----- |
| 16/11/2026 | Grupo | Spec inicial. `genera_cod_asm` se llama al reducir `programa`, cuando la lista de tercetos ya está cerrada. El recorrido sale del terceto 0. | Ninguna otra regla traduce a assembler. D17 queda en NASM x86 de 32 bits, objeto win32. |
| 16/11/2026 | Grupo | Implementación de `src/assembler/` y la acción de `programa`. Las rutinas del racional quedan en `runtime.asm` y se copian al final de `out/programa.asm`. | |
| 18/11/2026 | Grupo | E6, E7 y E9 ya no están fuera de alcance: los informa la etapa de tercetos. | Con `hay_errores` levantado, `em1_declaracion` y `em2_recursion` no regeneran el assembler. |
