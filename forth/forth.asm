.inc "user.h"

.org USER_CODE

        jp start

.dbg:
	; This is just instead of a list file in the assembly which zasm
	; does not have apperently :-(
	.dw evalForth
	.dw forthretcode
	.dw testfn
	
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
	; Now prepare all registers
	ld      IX,ixrethere
	inc     HL
	push	HL
	pop     IY	; IY now contains address to the function definition
			; In the primitive case, jp (IY) just executes there
			; whilst in the non-prim functions we immediately
			; do an jp (HL) to an eval function
	ld	HL,evalForth

	jp	(IY)
ixrethere:
	pop     HL
	; Done with this word, now we skip it and take on the next
skipword:

	jr      next

	xor	a		; success
	ret



sBanner:
	.db	"Welcome to Collapse Forth...", 0
sOK:
	.db	0x0d, 0x0a, "OK", 0x0d, 0x0a, 0
sNotFound:
	.db	0x0d, 0x0a, "Word not found!", 0
