.inc "user.h"

; At hex 4200 normally
.org USER_CODE

        jp start

.dbg:
	; This is just instead of a list file in the assembly which zasm
	; does not have apperently :-(
	; .dw myLabel
	.dw dotcode+1

; TODO: this is better in a separate .asm file prolly
.inc "dict.h"

blaha:
	ret

	
start:

	ld      hl,sBanner
	call    printstr
forever:
	ld      hl,sOK
	call    printstr

	call    stdioReadLine

next:
	; Here we now go back to main loop if an eof (0) is encountered
	ld      a,(hl)
	inc     hl
	cp      0
	jr      z,forever

	; Step unto the next non whitespace char
	cp      32
	jr      z,next
	; Can add more WS here if needed

	dec     HL

	ld      D,H
	ld      E,L

	; DE now stands on the first of the non-ws, HL is incremented
	; till the next space or eof (0)
step:
	inc     HL
	ld      a,(hl)
	cp      32
	jr      z,done
	cp      0
	jr      z,done
	; Can add more WS check if needed..

	jr step

done:
	; Store the pointer to the rest of the parsed command line
	push hl

	; ... Getting here means we have DE pointing to a non-ws
	; and HL is at least one larger than DE
	scf   	       ; sub the position and then one for the last WS
	sbc     HL,DE

	; Standing here we are on a word with DE and its length
	; held by L
	ld      C,L
	call    dictLookup

	; HL is now pointing to end of string char i.e. "NOP"
	; Now the call should be available in HL, just check so HL is not zero
	ld      A,L
 	or	H
	jr      z,notFoundError
	jr	doeval

notFoundError:
	ld HL, sNotFound
	call printstr
	jr forever

doeval:
	inc     HL	; HL now contains address to the function definition
			; This is the pointer to the tag field
	call	evalForth
	pop	HL

	; Done with this word, now we skip it and take on the next
skipword:

	jr      next

	xor	a		; success
	ret

evalForth:
	ld	D,0
	ld 	A,(HL)
	add	A,A      ; Double up
	ld 	E,A
	ld	IY, evalJmpTable
	add	IY,DE
	ld	B,(IY+1)
	ld	C,(IY)
	push	BC
	pop	IY
	inc	HL	; Make HL point to actual code or word links
	jp (IY)


evalPrimitive:
	ld	IX, ixrethere
	jp	(HL)
ixrethere:
	ret

evalSofist:
	; This is a simple loop over a set of pointers in the word we
	; are evaluating
	ld    C,(HL)
	inc   HL
	ld    B,(HL)
	inc   HL
	push hl
	push bc
	pop hl
	call evalForth
	pop hl
	jr evalSofist

; Cheat us into a forth function with special code to end a set of pointers
forthretcode:
	.db 0
forthretcodedbg:
	; Just eat our own call and return
	pop hl
	pop hl
	ret

evalJmpTable:
	.dw evalPrimitive
	.dw evalSofist ; A non-primitive routine is of course "sofisticated" :-)


sBanner:
	.db	"Welcome to Collapse Forth...", 0
sOK:
	.db	0x0d, 0x0a, "OK", 0x0d, 0x0a, 0
sNotFound:
	.db	0x0d, 0x0a, "Word not found!", 0
