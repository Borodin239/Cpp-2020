section         .text

                global          _start

_start:

                sub             rsp, 5 * 128 * 8 		; rsp - stack-pointer, указывает на место, где справа всё полезное и писать нельзя, а слева пусто 
                mov             rcx, 128 			; размер вводимых данных
		mov		r11, rcx			; rcx постоянно меняться будет при умножении и сложении, а этот регистр нужен в copy	
		mov		r15, 128 * 2			; размер результата умножения 	
                lea             rdi, [rsp + 128 * 8] 		; вычисляет эффективный адресс источника - обязательно память и помещает в приёмник (обязатиельно регистр)
                call            read_long 			; вызов функции чтения первого лонга
                mov             rdi, rsp		
                call            read_long 			; вызов функции чтения второго лонга
		lea             rsi, [rsp + 128 * 8] 		; адресс первого множителя (второй - rdi)
		lea		r13, [rsp + 128 * 8 * 2]	; это начало ответа
		lea		r8, [rsp + 128 * 8 * 4]		; для промежуточных вычислений

		call		mul_long_long		

		mov		rdi, r13
                call            write_long 			; вызов функции записи ответа

		
                mov             al, 0x0a
                call            write_char

                jmp             exit 				; выход из программы
		

; Multiplies two long number
;    rsi -- address of argument #1 (long number)
;    rdi -- address of argument #2 (long number)
;    rcx -- length of long numbers in qwords
; result:
;    result is written to r13

mul_long_long:
                push            r13
                push            rsi
                push            r11
		push		rbx
		
		
.loop:
      		mov		rbx, [rsi]			; помещаем в rbx то что будем умножать   
		push		rsi				; ты зануляешь rsi при умножении
		call		mul_long_short     		
		pop		rsi

		call		add_long_long			; прибавили к r13 r8 со сдвигом (сдвиг ниже)

		lea		rsi, [rsi + 8]		
		lea		r13, [r13 + 8]
                dec             r11				; decrement 		
                jnz             .loop

		pop		rbx
                pop             r11
                pop             rsi
                pop             r13
                ret	


; adds long number to result
;    r13 -- address of summand #1 (it will be the result)
;    r8 -- address of summand #2 (long number)
;    rcx -- length of long number #2 in qwords
; result:
;    sum is written to r13

add_long_long:
                push            r13
                push            r8
                push            rcx

                clc
.loop:
                mov             rax, [r8]
                lea             r8, [r8 + 8]
                adc             [r13], rax
                lea             r13, [r13 + 8]
                dec             rcx
                jnz             .loop

                pop             rcx
                pop             r8
                pop             r13
                ret


; adds 64-bit number to long number
;    rdi -- address of summand #1 (long number)
;    rax -- summand #2 (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    sum is written to rdi
add_long_short:
                push            rdi
                push            rcx
                push            rdx

                xor             rdx,rdx
.loop:
                add             [rdi], rax
                adc             rdx, 0				
                mov             rax, rdx
                xor             rdx, rdx
                add             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rdx
                pop             rcx
                pop             rdi
                ret

; multiplies long number by a short
;    rdi -- address of multiplier #1 (long number)
;    rbx -- multiplier #2 (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    product is written to r8
mul_long_short:		
                push            rax
                push            rdi
                push            rcx
		push		r8

                xor             rsi, rsi
.loop:
                mov             rax, [rdi]
                mul             rbx
                add             rax, rsi
                adc             rdx, 0
                mov             [r8], rax
                add             rdi, 8
		add		r8, 8
                mov             rsi, rdx
                dec             rcx
                jnz             .loop

		pop		r8
                pop             rcx
                pop             rdi
                pop             rax
                ret

; divides long number by a short
;    rdi -- address of dividend (long number)
;    rbx -- divisor (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    quotient is written to rdi
;    rdx -- remainder
div_long_short:
                push            rdi
                push            rax
                push            rcx

                lea             rdi, [rdi + 8 * rcx - 8]
                xor             rdx, rdx

.loop:
                mov             rax, [rdi]
                div             rbx
                mov             [rdi], rax
                sub             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rcx
                pop             rax
                pop             rdi
                ret

; assigns a zero to long number
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
set_zero:
                push            rax
                push            rdi
                push            rcx

                xor             rax, rax
                rep stosq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; checks if a long number is a zero
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
; result:
;    ZF=1 if zero
is_zero:
                push            rax
                push            rdi
                push            rcx

                xor             rax, rax
                rep scasq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; read long number from stdin
;    rdi -- location for output (long number)
;    rcx -- length of long number in qwords
read_long:
                push            rcx
                push            rdi

                call            set_zero
.loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              .done
                cmp             rax, '0'
                jb              .invalid_char
                cmp             rax, '9'
                ja              .invalid_char

                sub             rax, '0'
                mov             rbx, 10
		mov		r8, rdi
                call            mul_long_short
                call            add_long_short
                jmp             .loop

.done:
                pop             rdi
                pop             rcx
                ret

.invalid_char:								; обработка некоректного символа на вводе
                mov             rsi, invalid_char_msg
                mov             rdx, invalid_char_msg_size
                call            print_string
                call            write_char
                mov             al, 0x0a
                call            write_char

.skip_loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              exit
                jmp             .skip_loop

; write long number to stdout
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
;    r15 -- length of answer in qwords
write_long:
                push            rax
                push            rcx
			
                mov             rax, 20
                mul             rcx
                mov             rbp, rsp
                sub             rsp, rax

                mov             rsi, rbp

.loop:
                mov             rbx, 10
                call            div_long_short
                add             rdx, '0'
                dec             rsi
                mov             [rsi], dl
                call            is_zero
                jnz             .loop

                mov             rdx, rbp
                sub             rdx, rsi
                call            print_string

                mov             rsp, rbp
                pop             rcx
                pop             rax
                ret

; read one char from stdin
; result:
;    rax == -1 if error occurs
;    rax \in [0; 255] if OK
read_char:
                push            rcx
                push            rdi

                sub             rsp, 1
                xor             rax, rax
                xor             rdi, rdi
                mov             rsi, rsp
                mov             rdx, 1
                syscall

                cmp             rax, 1
                jne             .error
                xor             rax, rax
                mov             al, [rsp]
                add             rsp, 1

                pop             rdi
                pop             rcx
                ret
.error:
                mov             rax, -1
                add             rsp, 1
                pop             rdi
                pop             rcx
                ret

; write one char to stdout, errors are ignored
;    al -- char
write_char:
                sub             rsp, 1
                mov             [rsp], al

                mov             rax, 1
                mov             rdi, 1
                mov             rsi, rsp
                mov             rdx, 1
                syscall
                add             rsp, 1
                ret

exit:
                mov             rax, 60
                xor             rdi, rdi
                syscall

; print string to stdout
;    rsi -- string
;    rdx -- size
print_string:
                push            rax

                mov             rax, 1
                mov             rdi, 1
                syscall

                pop             rax
                ret


section         .rodata

invalid_char_msg:      db              "Invalid character: "
invalid_char_msg_size: equ             $ - invalid_char_msg

