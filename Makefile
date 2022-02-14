all:
	gcc Assembler/Assembler.c Assembler/main.c -o Assembler/asm.out
	gcc Processor/stack.c Processor/Processor.c Processor/main.c -lm -o Processor/proc.out
	Assembler/./asm.out Asm_Progs/picasm.txt Asm_Progs/pic.bin
	Processor/./proc.out Asm_Progs/pic.bin