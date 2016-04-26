.include "p30f6014A.inc"

; assembler file for the HSYNC interrupt
.global	__T4Interrupt
__T4Interrupt:
; /!\ interrupt nesting, push.s allowed only once !
		push.s

		bclr 	IFS1,#T4IF		; clear interrupt flag
		clr 	TMR4			; clear timer

		; for fine syncronisation
		nop

		mov __po3030k_buffer, w1
		mov #__po3030k_line_conf, w2
		mov #1, w3				; used for comparaisons

restart_loop:
		cp.b w3,[w2++]	
		bra Z, take_it 	; if w2 == 1		
		bra LT, end_line ; if w2 == 2
		; w2 == 0
		nop
		nop
		nop
		bra restart_loop
take_it:
		mov PORTD, w0	
		lsr w0,#8,w0	
		mov.b w0,[w1++]	
		bra restart_loop

end_line:
		mov w1, __po3030k_buffer
		inc __po3030k_current_row
		mov __po3030k_current_row,w0
		cp __po3030k_row
		bra NZ, go_out
		; tell others we are ready
		mov #1, w0
		mov w0, __po3030k_img_ready		
		; disable ourself
		bclr T4CON,#TON
go_out:
		pop.s
		retfie
