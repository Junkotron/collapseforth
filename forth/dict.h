

; Supersmall forth stack and user dict for now, TODO: How to allocate
; a lot of memory ?
dictstart:
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

; System variables, user dictionary starts empty, stack starts at that point
; and is relocated as data is added to the dictionary
stackstart: 
dictend: 
	 .dw dictstart

; Here are some hidden routines

evalForth:
	; TODO: This will be jumped to from the jp (HL) in all non-prim
	; functions, we shall start to call each function contained
	; in this functions body and then further down etc.
	inc IY
	; IY now points to the
	; function we are to evaluate

nextInstr:

	; a primitive function returns to
	; this address
	ld IX, primfunc

	ld H,(IY+1)
	ld L,(IY)

	inc IY
	inc IY
	push IY
	jp (HL)

primfunc:
	; OK take next IY
	pop IY
	jr nextInstr

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

; This will be pointed to at all non-primitive definitions end
; and will return to previous routine or back to prompt
forthretcode:

	jp (IX)

; Here is the chained dictionary for all primitive words for now

; This is where vlist and word lookup starts
firstword:

dup:
	.dw dupend  ; next
	.db "DUP", 0
dupcode:
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
	; TODO: Do some manipulations on the forth stack (also todo)
	ld HL,dropdbg
	call printstr
	jp (IX)
dropdbg:
	.db "Now we run DROP", 0x0d, 0x0a, 0
dropend:

; ...

; This is a forth non-primitive defintion, does two DUP calls

testfn:
	.dw testfnend
	.db "TESTFN", 0
testfncode:
	jp (HL)
	.dw dupcode
	.dw dupcode
	.dw forthretcode
testfnend:


; When searching the dictionary, this is the last element
lastword:
	.dw 0
	.db 0


found:
	.db " Found match ", 0
