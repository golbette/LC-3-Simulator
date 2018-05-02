/*
 * lc3.h
 *
 *  Date Due: Apr 8, 2018
 *  Authors:  Michael Josten, and Samantha Anderson
 *  version: 408a
 */
#include <ncurses.h>
#ifndef LC3_H_
#define LC3_H_

struct CPUType {
	unsigned short int PC;     // program counter.
	unsigned short int originalPC; // starting address for instructions.
	unsigned short cc;         // condition code for BR instruction.
	unsigned short int reg[8]; // registers.
	unsigned short int ir;     // instruction register.
	unsigned short mar;        // memory address register.
	unsigned short mdr;        // memory data register.
	unsigned short memory[65535]; //memory
	unsigned short A; //operand
	unsigned short B; //operand 
};

typedef struct CPUType CPU_p;
void refreshDisplay(CPU_p *cpu, WINDOW *local_win);
int controller (CPU_p*, short, WINDOW*);
void displayCPU(CPU_p*);
void zeroOut(unsigned short *array, int);
CPU_p initializeCPU();
unsigned short ZEXT(unsigned short);
unsigned short SEXT(unsigned short);
void TRAP_getch(CPU_p*, WINDOW*);
void TRAP_out(CPU_p*, WINDOW*);
void TRAP_puts(CPU_p*, WINDOW*);
void TRAP(unsigned short, CPU_p, WINDOW*);
unsigned short getConditionCode(unsigned short);
int loadInstructions(CPU_p *cpu, char *fileName);


#endif /* LC3_H_ */
