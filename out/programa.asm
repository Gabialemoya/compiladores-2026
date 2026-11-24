section .data
a: dd 0
dd 1
b: dd 0
dd 1
c: dd 0
dd 1
cte_1_2: dd 1
dd 2
cte_1_3: dd 1
dd 3
str5: db "Resultado", 0
ter3: dd 0
dd 1
sa_lo: dd 0
sa_hi: dd 0
sb_lo: dd 0
sb_hi: dd 0
sc_lo: dd 0
sc_hi: dd 0
sd_lo: dd 0
sd_hi: dd 0
g_lo: dd 0
g_hi: dd 0
dg_lo: dd 0
dg_hi: dd 0
bg_lo: dd 0
bg_hi: dd 0
p1_lo: dd 0
p1_hi: dd 0
p2_lo: dd 0
p2_hi: dd 0
den_lo: dd 0
den_hi: dd 0
nr_lo: dd 0
nr_hi: dd 0
nd_lo: dd 0
nd_hi: dd 0
quot_lo: dd 0
quot_hi: dd 0
rem_lo: dd 0
rem_hi: dd 0
dvd_lo: dd 0
dvd_hi: dd 0
dvs_lo: dd 0
dvs_hi: dd 0
dest_ptr: dd 0
linea_op: dd 0
aviso_op: dd 0
signo_op: dd 0
a1_lo: dd 0
a1_hi: dd 0
c1_lo: dd 0
c1_hi: dd 0
b1_lo: dd 0
b1_hi: dd 0
d1_lo: dd 0
d1_hi: dd 0
ma_lo: dd 0
ma_hi: dd 0
mb_lo: dd 0
mb_hi: dd 0
ta_lo: dd 0
ta_hi: dd 0
tb_lo: dd 0
tb_hi: dd 0
pr_lo: dd 0
pr_hi: dd 0
p_lo: dd 0
p_hi: dd 0
q_lo: dd 0
q_hi: dd 0
fmt_rac: db "%d/%d", 10, 0
fmt_str: db "%s", 10, 0
fmt_a1: db "Linea %d: Advertencia: perdida de precision", 10, 0
fmt_e8: db "Linea %d: Error: denominador cero", 10, 0

section .text
global _main
extern _printf
extern _exit

_main:
; 0 BI
t0:
    jmp t1
; 1 ASIG
t1:
    mov eax, [cte_1_2]
    mov [a], eax
    mov eax, [cte_1_2+4]
    mov [a+4], eax
; 2 ASIG
t2:
    mov eax, [cte_1_3]
    mov [b], eax
    mov eax, [cte_1_3+4]
    mov [b+4], eax
; 3 SUMA
t3:
    push 7
    push ter3
    push b
    push a
    call suma_rac
    add esp, 16
; 4 ASIG
t4:
    mov eax, [ter3]
    mov [c], eax
    mov eax, [ter3+4]
    mov [c+4], eax
; 5 WRITE_STR
t5:
    push str5
    push fmt_str
    call _printf
    add esp, 8
; 6 WRITE
t6:
    push dword [c+4]
    push dword [c]
    push fmt_rac
    call _printf
    add esp, 12
t7:
    xor eax, eax
    ret

; Rutinas del racional. Se copian una sola vez al final de programa.asm.
; Enteros de 64 bits en memoria: dword bajo, dword alto. cdecl.

udiv64:
    push ebx
    push esi
    push edi
    mov eax, [esp+16]
    mov [dvd_lo], eax
    mov eax, [esp+20]
    mov [dvd_hi], eax
    mov eax, [esp+24]
    mov [dvs_lo], eax
    mov eax, [esp+28]
    mov [dvs_hi], eax
    xor eax, eax
    mov [quot_lo], eax
    mov [quot_hi], eax
    mov [rem_lo], eax
    mov [rem_hi], eax
    mov ecx, 63
.bit:
    mov eax, [rem_lo]
    mov edx, [rem_hi]
    shld edx, eax, 1
    shl eax, 1
    cmp ecx, 32
    jl .lowb
    mov ebx, ecx
    sub ebx, 32
    mov esi, [dvd_hi]
    bt esi, ebx
    jnc .have
    or eax, 1
    jmp .have
.lowb:
    mov esi, [dvd_lo]
    bt esi, ecx
    jnc .have
    or eax, 1
.have:
    mov [rem_lo], eax
    mov [rem_hi], edx
    mov eax, [rem_hi]
    cmp eax, [dvs_hi]
    ja .sub
    jb .next
    mov eax, [rem_lo]
    cmp eax, [dvs_lo]
    jb .next
.sub:
    mov eax, [rem_lo]
    sub eax, [dvs_lo]
    mov [rem_lo], eax
    mov eax, [rem_hi]
    sbb eax, [dvs_hi]
    mov [rem_hi], eax
    cmp ecx, 32
    jl .setqlo
    mov ebx, ecx
    sub ebx, 32
    mov esi, [quot_hi]
    bts esi, ebx
    mov [quot_hi], esi
    jmp .next
.setqlo:
    mov esi, [quot_lo]
    bts esi, ecx
    mov [quot_lo], esi
.next:
    dec ecx
    jns .bit
    pop edi
    pop esi
    pop ebx
    ret

mcd64:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
.loop:
    mov eax, [ebp+16]
    or eax, [ebp+20]
    jz .done
    push dword [ebp+20]
    push dword [ebp+16]
    push dword [ebp+12]
    push dword [ebp+8]
    call udiv64
    add esp, 16
    mov eax, [ebp+16]
    mov edx, [ebp+20]
    mov [ebp+8], eax
    mov [ebp+12], edx
    mov eax, [rem_lo]
    mov edx, [rem_hi]
    mov [ebp+16], eax
    mov [ebp+20], edx
    jmp .loop
.done:
    mov eax, [ebp+8]
    mov edx, [ebp+12]
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

; Cociente exacto con signo. El valor absoluto de -2^63 queda en bits
; y la negacion posterior lo devuelve si el divisor es 1.
div_exacto:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov dword [signo_op], 0
    mov eax, [ebp+8]
    mov edx, [ebp+12]
    test edx, edx
    jns .abs
    mov dword [signo_op], 1
    not eax
    not edx
    add eax, 1
    adc edx, 0
.abs:
    push dword [ebp+20]
    push dword [ebp+16]
    push edx
    push eax
    call udiv64
    add esp, 16
    mov eax, [quot_lo]
    mov edx, [quot_hi]
    cmp dword [signo_op], 0
    je .fin
    not eax
    not edx
    add eax, 1
    adc edx, 0
.fin:
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

; esi apunta al entero de 64 bits. Lo reemplaza por div2.
div2_ptr:
    mov eax, [esi]
    mov edx, [esi+4]
    cmp edx, 0x80000000
    jne .notmin
    test eax, eax
    jnz .notmin
    mov dword [esi], 0
    mov dword [esi+4], 0xC0000000
    ret
.notmin:
    test edx, edx
    js .neg
    add eax, 1
    adc edx, 0
    shrd eax, edx, 1
    shr edx, 1
    mov [esi], eax
    mov [esi+4], edx
    ret
.neg:
    sub eax, 1
    sbb edx, 0
    not eax
    not edx
    add eax, 1
    adc edx, 0
    shrd eax, edx, 1
    shr edx, 1
    not eax
    not edx
    add eax, 1
    adc edx, 0
    mov [esi], eax
    mov [esi+4], edx
    ret

mitad_den:
    mov eax, [nd_lo]
    mov edx, [nd_hi]
    test edx, edx
    js .neg
    shrd eax, edx, 1
    shr edx, 1
    jmp .chk
.neg:
    cmp edx, 0x80000000
    jne .negok
    test eax, eax
    jnz .negok
    xor eax, eax
    mov edx, 0xC0000000
    jmp .chk
.negok:
    not eax
    not edx
    add eax, 1
    adc edx, 0
    shrd eax, edx, 1
    shr edx, 1
    not eax
    not edx
    add eax, 1
    adc edx, 0
.chk:
    mov ecx, eax
    or ecx, edx
    jnz .store
    mov eax, 1
    xor edx, edx
.store:
    mov [nd_lo], eax
    mov [nd_hi], edx
    ret

reducir_par:
    push ebx
    push esi
    push edi
    mov eax, [nr_lo]
    mov edx, [nr_hi]
    test edx, edx
    jns .abs
    not eax
    not edx
    add eax, 1
    adc edx, 0
.abs:
    push dword [nd_hi]
    push dword [nd_lo]
    push edx
    push eax
    call mcd64
    add esp, 16
    mov [g_lo], eax
    mov [g_hi], edx
    push dword [g_hi]
    push dword [g_lo]
    push dword [nr_hi]
    push dword [nr_lo]
    call div_exacto
    add esp, 16
    mov [nr_lo], eax
    mov [nr_hi], edx
    push dword [g_hi]
    push dword [g_lo]
    push dword [nd_hi]
    push dword [nd_lo]
    call udiv64
    add esp, 16
    mov eax, [quot_lo]
    mov edx, [quot_hi]
    mov [nd_lo], eax
    mov [nd_hi], edx
    pop edi
    pop esi
    pop ebx
    ret

norm_rac:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov eax, [ebp+8]
    mov [nr_lo], eax
    mov eax, [ebp+12]
    mov [nr_hi], eax
    mov eax, [ebp+16]
    mov [nd_lo], eax
    mov eax, [ebp+20]
    mov [nd_hi], eax
    mov eax, [ebp+32]
    mov [aviso_op], eax
    mov eax, [nd_hi]
    test eax, eax
    jns .denpos
    mov edx, [nr_hi]
    cmp edx, 0x80000000
    jne .niega
    cmp dword [nr_lo], 0
    jne .niega
    mov dword [nr_lo], 0
    mov dword [nr_hi], 0xC0000000
    call mitad_den
    jmp .denpos
.niega:
    mov eax, [nr_lo]
    mov edx, [nr_hi]
    not eax
    not edx
    add eax, 1
    adc edx, 0
    mov [nr_lo], eax
    mov [nr_hi], edx
    mov eax, [nd_lo]
    mov edx, [nd_hi]
    not eax
    not edx
    add eax, 1
    adc edx, 0
    mov [nd_lo], eax
    mov [nd_hi], edx
.denpos:
    mov eax, [nr_lo]
    or eax, [nr_hi]
    jnz .sgcd
    mov edi, [ebp+24]
    mov dword [edi], 0
    mov dword [edi+4], 1
    jmp .fin
.sgcd:
    call reducir_par
.dgloop:
    mov eax, [nr_lo]
    cdq
    cmp edx, [nr_hi]
    jne .degr
    cmp dword [nd_hi], 0
    jne .degr
    cmp dword [nd_lo], 2147483647
    ja .degr
    jmp .dgfin
.degr:
    mov esi, nr_lo
    call div2_ptr
    call mitad_den
    mov dword [aviso_op], 1
    jmp .dgloop
.dgfin:
    call reducir_par
    mov eax, [nr_lo]
    or eax, [nr_hi]
    jnz .av
    mov dword [nr_lo], 0
    mov dword [nr_hi], 0
    mov dword [nd_lo], 1
    mov dword [nd_hi], 0
.av:
    cmp dword [aviso_op], 0
    je .store
    push dword [ebp+28]
    push fmt_a1
    call _printf
    add esp, 8
.store:
    mov edi, [ebp+24]
    mov eax, [nr_lo]
    mov [edi], eax
    mov eax, [nd_lo]
    mov [edi+4], eax
.fin:
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

mul_s64:
    push ebx
    push esi
    push edi
    mov dword [signo_op], 0
    mov eax, [ma_lo]
    mov edx, [ma_hi]
    test edx, edx
    jns .aok
    xor dword [signo_op], 1
    not eax
    not edx
    add eax, 1
    adc edx, 0
.aok:
    mov [ta_lo], eax
    mov [ta_hi], edx
    mov eax, [mb_lo]
    mov edx, [mb_hi]
    test edx, edx
    jns .bok
    xor dword [signo_op], 1
    not eax
    not edx
    add eax, 1
    adc edx, 0
.bok:
    mov [tb_lo], eax
    mov [tb_hi], edx
    mov eax, [ta_lo]
    mul dword [tb_lo]
    mov [pr_lo], eax
    mov [pr_hi], edx
    mov eax, [ta_lo]
    mul dword [tb_hi]
    add [pr_hi], eax
    mov eax, [ta_hi]
    mul dword [tb_lo]
    add [pr_hi], eax
    mov eax, [pr_lo]
    mov edx, [pr_hi]
    cmp dword [signo_op], 0
    je .fin
    not eax
    not edx
    add eax, 1
    adc edx, 0
.fin:
    pop edi
    pop esi
    pop ebx
    ret

sumar_cargados:
    push ebx
    push esi
    push edi
    push dword [sd_hi]
    push dword [sd_lo]
    push dword [sb_hi]
    push dword [sb_lo]
    call mcd64
    add esp, 16
    mov [g_lo], eax
    mov [g_hi], edx
    push dword [g_hi]
    push dword [g_lo]
    push dword [sd_hi]
    push dword [sd_lo]
    call udiv64
    add esp, 16
    mov eax, [quot_lo]
    mov [dg_lo], eax
    mov eax, [quot_hi]
    mov [dg_hi], eax
    push dword [g_hi]
    push dword [g_lo]
    push dword [sb_hi]
    push dword [sb_lo]
    call udiv64
    add esp, 16
    mov eax, [quot_lo]
    mov [bg_lo], eax
    mov eax, [quot_hi]
    mov [bg_hi], eax
    mov eax, [sa_lo]
    mov [ma_lo], eax
    mov eax, [sa_hi]
    mov [ma_hi], eax
    mov eax, [dg_lo]
    mov [mb_lo], eax
    mov eax, [dg_hi]
    mov [mb_hi], eax
    call mul_s64
    mov [p1_lo], eax
    mov [p1_hi], edx
    mov eax, [sc_lo]
    mov [ma_lo], eax
    mov eax, [sc_hi]
    mov [ma_hi], eax
    mov eax, [bg_lo]
    mov [mb_lo], eax
    mov eax, [bg_hi]
    mov [mb_hi], eax
    call mul_s64
    mov [p2_lo], eax
    mov [p2_hi], edx
    mov eax, [sb_lo]
    mov [ma_lo], eax
    mov eax, [sb_hi]
    mov [ma_hi], eax
    mov eax, [dg_lo]
    mov [mb_lo], eax
    mov eax, [dg_hi]
    mov [mb_hi], eax
    call mul_s64
    mov [den_lo], eax
    mov [den_hi], edx
    mov dword [aviso_op], 0
.sumloop:
    mov eax, [p1_lo]
    mov edx, [p1_hi]
    add eax, [p2_lo]
    adc edx, [p2_hi]
    jno .sumok
    mov esi, p1_lo
    call div2_ptr
    mov esi, p2_lo
    call div2_ptr
    mov eax, [den_lo]
    mov edx, [den_hi]
    shrd eax, edx, 1
    shr edx, 1
    mov ecx, eax
    or ecx, edx
    jnz .dennz
    mov eax, 1
    xor edx, edx
.dennz:
    mov [den_lo], eax
    mov [den_hi], edx
    mov dword [aviso_op], 1
    jmp .sumloop
.sumok:
    push dword [aviso_op]
    push dword [linea_op]
    push dword [dest_ptr]
    push dword [den_hi]
    push dword [den_lo]
    push edx
    push eax
    call norm_rac
    add esp, 28
    pop edi
    pop esi
    pop ebx
    ret

mul_cargados:
    push ebx
    push esi
    push edi
    mov eax, [sa_lo]
    mov edx, [sa_hi]
    test edx, edx
    jns .abs_a
    not eax
    not edx
    add eax, 1
    adc edx, 0
.abs_a:
    push dword [sd_hi]
    push dword [sd_lo]
    push edx
    push eax
    call mcd64
    add esp, 16
    mov [g_lo], eax
    mov [g_hi], edx
    push dword [g_hi]
    push dword [g_lo]
    push dword [sa_hi]
    push dword [sa_lo]
    call div_exacto
    add esp, 16
    mov [a1_lo], eax
    mov [a1_hi], edx
    push dword [g_hi]
    push dword [g_lo]
    push dword [sd_hi]
    push dword [sd_lo]
    call udiv64
    add esp, 16
    mov eax, [quot_lo]
    mov [d1_lo], eax
    mov eax, [quot_hi]
    mov [d1_hi], eax
    mov eax, [sc_lo]
    mov edx, [sc_hi]
    test edx, edx
    jns .abs_c
    not eax
    not edx
    add eax, 1
    adc edx, 0
.abs_c:
    push dword [sb_hi]
    push dword [sb_lo]
    push edx
    push eax
    call mcd64
    add esp, 16
    mov [g_lo], eax
    mov [g_hi], edx
    push dword [g_hi]
    push dword [g_lo]
    push dword [sc_hi]
    push dword [sc_lo]
    call div_exacto
    add esp, 16
    mov [c1_lo], eax
    mov [c1_hi], edx
    push dword [g_hi]
    push dword [g_lo]
    push dword [sb_hi]
    push dword [sb_lo]
    call udiv64
    add esp, 16
    mov eax, [quot_lo]
    mov [b1_lo], eax
    mov eax, [quot_hi]
    mov [b1_hi], eax
    mov eax, [a1_lo]
    mov [ma_lo], eax
    mov eax, [a1_hi]
    mov [ma_hi], eax
    mov eax, [c1_lo]
    mov [mb_lo], eax
    mov eax, [c1_hi]
    mov [mb_hi], eax
    call mul_s64
    mov [p1_lo], eax
    mov [p1_hi], edx
    mov eax, [b1_lo]
    mov [ma_lo], eax
    mov eax, [b1_hi]
    mov [ma_hi], eax
    mov eax, [d1_lo]
    mov [mb_lo], eax
    mov eax, [d1_hi]
    mov [mb_hi], eax
    call mul_s64
    push dword 0
    push dword [linea_op]
    push dword [dest_ptr]
    push edx
    push eax
    push dword [p1_hi]
    push dword [p1_lo]
    call norm_rac
    add esp, 28
    pop edi
    pop esi
    pop ebx
    ret

suma_rac:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov esi, [ebp+8]
    mov edi, [ebp+12]
    mov eax, [esi]
    cdq
    mov [sa_lo], eax
    mov [sa_hi], edx
    mov eax, [esi+4]
    mov [sb_lo], eax
    mov dword [sb_hi], 0
    mov eax, [edi]
    cdq
    mov [sc_lo], eax
    mov [sc_hi], edx
    mov eax, [edi+4]
    mov [sd_lo], eax
    mov dword [sd_hi], 0
    mov eax, [ebp+16]
    mov [dest_ptr], eax
    mov eax, [ebp+20]
    mov [linea_op], eax
    call sumar_cargados
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

resta_rac:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov esi, [ebp+8]
    mov edi, [ebp+12]
    mov eax, [esi]
    cdq
    mov [sa_lo], eax
    mov [sa_hi], edx
    mov eax, [esi+4]
    mov [sb_lo], eax
    mov dword [sb_hi], 0
    mov eax, [edi]
    cdq
    mov [sc_lo], eax
    mov [sc_hi], edx
    mov eax, [edi+4]
    mov [sd_lo], eax
    mov dword [sd_hi], 0
    mov eax, [sc_lo]
    mov edx, [sc_hi]
    not eax
    not edx
    add eax, 1
    adc edx, 0
    mov [sc_lo], eax
    mov [sc_hi], edx
    mov eax, [ebp+16]
    mov [dest_ptr], eax
    mov eax, [ebp+20]
    mov [linea_op], eax
    call sumar_cargados
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

mul_rac:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov esi, [ebp+8]
    mov edi, [ebp+12]
    mov eax, [esi]
    cdq
    mov [sa_lo], eax
    mov [sa_hi], edx
    mov eax, [esi+4]
    mov [sb_lo], eax
    mov dword [sb_hi], 0
    mov eax, [edi]
    cdq
    mov [sc_lo], eax
    mov [sc_hi], edx
    mov eax, [edi+4]
    mov [sd_lo], eax
    mov dword [sd_hi], 0
    mov eax, [ebp+16]
    mov [dest_ptr], eax
    mov eax, [ebp+20]
    mov [linea_op], eax
    call mul_cargados
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

div_rac:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov esi, [ebp+8]
    mov edi, [ebp+12]
    mov eax, [edi]
    test eax, eax
    jnz .ok
    push dword [ebp+20]
    push fmt_e8
    call _printf
    add esp, 8
    push 1
    call _exit
.ok:
    mov eax, [esi]
    cdq
    mov [sa_lo], eax
    mov [sa_hi], edx
    mov eax, [esi+4]
    mov [sb_lo], eax
    mov dword [sb_hi], 0
    mov eax, [edi+4]
    cdq
    mov [sc_lo], eax
    mov [sc_hi], edx
    mov eax, [edi]
    cdq
    mov [sd_lo], eax
    mov [sd_hi], edx
    test edx, edx
    jns .pos
    mov eax, [sc_lo]
    mov edx, [sc_hi]
    not eax
    not edx
    add eax, 1
    adc edx, 0
    mov [sc_lo], eax
    mov [sc_hi], edx
    mov eax, [sd_lo]
    mov edx, [sd_hi]
    not eax
    not edx
    add eax, 1
    adc edx, 0
    mov [sd_lo], eax
    mov [sd_hi], edx
.pos:
    mov eax, [ebp+16]
    mov [dest_ptr], eax
    mov eax, [ebp+20]
    mov [linea_op], eax
    call mul_cargados
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

comparar:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov esi, [ebp+8]
    mov edi, [ebp+12]
    mov eax, [esi]
    imul dword [edi+4]
    mov [p_lo], eax
    mov [p_hi], edx
    mov eax, [edi]
    imul dword [esi+4]
    mov [q_lo], eax
    mov [q_hi], edx
    mov eax, [p_hi]
    cmp eax, [q_hi]
    jl .lt
    jg .gt
    mov eax, [p_lo]
    cmp eax, [q_lo]
    jb .lt
    ja .gt
    xor eax, eax
    jmp .map
.lt:
    mov eax, -1
    jmp .map
.gt:
    mov eax, 1
.map:
    mov edx, [ebp+20]
    xor ecx, ecx
    cmp edx, 0
    je .eq
    cmp edx, 1
    je .ne
    cmp edx, 2
    je .menor
    cmp edx, 3
    je .menig
    cmp edx, 4
    je .mayor
    cmp eax, 0
    jl .store
    mov ecx, -1
    jmp .store
.eq:
    test eax, eax
    jnz .store
    mov ecx, -1
    jmp .store
.ne:
    test eax, eax
    jz .store
    mov ecx, -1
    jmp .store
.menor:
    cmp eax, -1
    jne .store
    mov ecx, -1
    jmp .store
.menig:
    cmp eax, 1
    je .store
    mov ecx, -1
    jmp .store
.mayor:
    cmp eax, 1
    jne .store
    mov ecx, -1
.store:
    mov edx, [ebp+16]
    mov [edx], ecx
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
