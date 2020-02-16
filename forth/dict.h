
;enough to print signed 16 bit binary number and last null (18 bytes)
numbuf: .dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

; Supersmall forth stack and user dict for now, TODO: How to allocate
; a lot of memory ?
dictstart:
	.dw 0x9abc ; TODO temp for debuging DOT
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

; System variables, user dictionary starts empty, stack starts at that point
; and is relocated as data is added to the dictionary
; This is the data stack r-stack is cpu stacks
stackstart: 
dictend: 
	.dw dictstart ; Grows upward moving stack upwards
stackcurr:
	; TODO this is a pre set value
	; only to debug the DOT function
	.dw dictstart+2 ; Grows upward in mem
	 
dictLookup:
	; This routine will search for string pointed to by DE and length in C
	ld      HL,firstword
	inc     C      	    ; Compare also the null at end
dictLoop:

	ld	B,C	    ; Use B as temp counter
	push    DE
	push    HL
	inc     HL
	inc     HL
wordLoop:

	ld	A,(DE)
	cp      (HL)
	jp      nz,nextword

	inc     hl
	inc     de

	dec     B
	jp	nz,wordLoop

	; Also need to check that word in dict is not longer than
	; the word from command line buffer
	ld      A,(HL)
	cp      0
	jr      nz, nextword

	pop     DE   ; eat hl value not needed
	pop     DE

	; Getting here means we found a match
	; Return the pointer to the defintions code, HL should now stand on code
	ret

nextword:
	pop     HL
	pop	DE
	push    BC
	ld	C,(HL)
	inc	HL
	ld	B,(HL)
	ld	H,B
	ld	L,C
	pop     BC

	; Check if we have reached last in list
	ld      A,L
	or	H
	jp      z,wordNotFound
	jp	dictLoop

wordNotFound:
	ret


; Here is the chained dictionary for all primitive words for now

; This is where vlist and word lookup starts
firstword:

dup:
	.dw dupend  ; next
	.db "DUP", 0
dupcode:
	.db 0
	; TODO: Do some manipulations on the forth stack (also todo)
	ld HL,dupdbg
	call printstr
	jp (IX)
dupdbg:
	.db "Now we run DUP", 0x0d, 0x0a, 0
dupend:

drop:
	.dw dropend  ; next
	.db "DROP", 0
dropcode:
	.db 0		       
	; TODO: Do some manipulations on the forth stack (also todo)
	ld HL,dropdbg
	call printstr
	jp (IX)
dropdbg:
	.db "Now we run DROP", 0x0d, 0x0a, 0
dropend:

dot:
	.dw dotend
	.db "DOT", 0
dotcode:
	.db 0
	; TODO make this use BASE variable, now is hex, also handle negative
	ld IY,(stackcurr)
	ld hl,numbuf
	ld A,(IY-1)
	call byte2asc
	ld A,(IY-2)
	call byte2asc
	ld (hl),0
	ld hl,numbuf
	call printstr
	jp (IX)
	    
byte2asc:
	ld B,A
	srl a
	srl a
	srl a
	srl a
	call nibble2asc
	ld (hl),a
	inc hl
	ld A,B
	and 0xf
	call nibble2asc
	ld (hl),a
	inc hl
	ret

nibble2asc:
	cp 10
	jp m,skiphex
	add a,7 ; make values 10 or greater into A..F
skiphex:
	add a,'0'
	ret

dotend:	

	    ; ...

; This is a forth non-primitive defintion, does two DUP calls

testfn:
	.dw testfnend
	.db "TESTFN", 0
testfncode:
	.db 1
	.dw dupcode
	.dw dupcode
	.dw forthretcode
testfnend:

testfn2:
	.dw testfn2end
	.db "TESTFN2", 0
testfn2code:
	.db 1
	.dw dupcode
	.dw dupcode
	.dw testfncode
	.dw forthretcode
testfn2end:


; When searching the dictionary, this is the last element
lastword:
	.dw 0
	.db 0


found:
	.db " Found match ", 0
