# Copyright      2017  Dexter Gerig <dexgerig@gmail.com>
# Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
# Copyright      2011  Bernhard Urban <lewurm@gmail.com>
# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

.set r0, 0
.set r1, 1
.set r2, 2
.set r3, 3
.set r4, 4
.set r5, 5
.set r6, 6
.set r7, 7
.set r8, 8
.set r9, 9
.set r10, 10
.set r11, 11
.set r12, 12
.set r13, 13
.set r14, 14
.set r15, 15
.set r16, 16
.set r17, 17
.set r18, 18
.set r19, 19
.set r20, 20
.set r21, 21
.set r22, 22
.set r23, 23
.set r24, 24
.set r25, 25
.set r26, 26
.set r27, 27
.set r28, 28
.set r29, 29
.set r30, 30
.set r31, 31

	.section .start,"ax"
	retadr = 0x80003500

start:

	# Make high ram executable.
	lis r4, 0
	lis r3, 0
	mtspr 569, r4
	mtspr 568, r3
	isync
	mtspr 561, r4
	mtspr 560, r3
	isync
	lis r4, 0x1000 ; ori r4, r4, 0x0002
	lis r3, 0x9000 ; ori r3, r3, 0x1fff
	mtspr 569, r4
	mtspr 568, r3
	isync
	mtspr 561, r4
	mtspr 560, r3
	isync

	# Move code into place
	lis 3,main@h ; ori 3,3,main@l ; addi 5,3,-4
	lis 4,(retadr-4)@h ; ori 4,4,(retadr-4)@l
	addi 4,4,end-start

	li 0,0x2000 ; mtctr 0
0:	lwzu 0,4(4) ; stwu 0,4(5) ; bdnz 0b

	# Sync caches on it.
	li 0,0x0400 ; mtctr 0 ; mr 5,3
0:	dcbst 0,5 ; sync ; icbi 0,5 ; addi 5,5,0x20 ; bdnz 0b
	sync ; isync

	# Go for it!
	mtctr 3 ; bctr
end:
