# Mystic2
Second version of my real-time operating system for ARM and AVR written in C

Project was under development in 2012-2013.

Really old stuff, made a few years ago when I was learning programming in C, especially microcontrollers.
But it was quite good. I made own IDE in C# with custom assembler compiler (similar to x86) and then aps were ecuted on ARM and AVR without problems.

#  Features:
- written in C for ARM and AVR
- real-time OS with simple memory management
- multitasking
- custom assembler with parser and compiler
- loaded applications from SD cards
- supported simple TFT screen with touchpad

Here is Hello World in my asm:

```asm
; main function
BLOCK MAIN:

section data:	
;; initialized data
text db "Hello World!",0

section text:
  ; Program execution starts here...
  
; print text on the screen
mov eax, 0 ; text lcoation on teh screen (should be Point struct defined in the API)
mov ebx, [tekst] ; move adress of the text to the register
mov ecx, 0 ; text color (black)
int 50

; while(true) {}
label noexit:
jump noexit

end
```