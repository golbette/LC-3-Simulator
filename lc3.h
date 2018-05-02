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
//constants
#define FETCH     0
#define DECODE    1
#define EVAL_ADDR 2
#define FETCH_OP  3
#define EXECUTE   4
#define STORE     5

#define OP_ADD             1 // 0001 0000 0000 0000
#define OP_AND             5 // 0101 0000 0000 0000
#define OP_NOT             9 // 1001 0000 0000 0000
#define OP_TRAP           15 // 1111 0000 0000 0000
#define OP_LD              2 // 0010 0000 0000 0000
#define OP_ST              3 // 0011 0000 0000 0000
#define OP_JMP            12 // 1100 0000 0000 0000
#define OP_BR              0 // 0000 0000 0000 0000
#define OP_JSRR            4 // 0100 0000 0000 0000
#define OP_LEA            14 // 1110 0000 0000 0000

#define MASK_OPCODE    61440 // 1111 0000 0000 0000
#define MASK_DR         3584 // 0000 1110 0000 0000
#define MASK_SR1         448 // 0000 0001 1100 0000
#define MASK_SR2           7 // 0000 0000 0000 0111
#define MASK_PCOFFSET9   511 // 0000 0001 1111 1111
#define MASK_TRAPVECT8   255 // 0000 0000 1111 1111
#define MASK_BIT5         32 // 0000 0000 0010 0000
#define MASK_IMMED5       31 // 0000 0000 0001 1111
#define MASK_NZP        3584 // 0000 1110 0000 0000

#define CONDITION_N        4 // 0000 1000 0000 0000
#define CONDITION_Z        2 // 0000 0100 0000 0000
#define CONDITION_P        1 // 0000 0010 0000 0000
#define CONDITION_NZ       6 // 0000 1100 0000 0000
#define CONDITION_NP       5 // 0000 1010 0000 0000
#define CONDITION_ZP       3 // 0000 0110 0000 0000
#define CONDITION_NZP      7 // 0000 1110 0000 0000

//ncurses window constants
#define REG_START_Y  2
#define REG_START_X  5
#define REG_MAX      8
#define MEM_START_Y  2
#define MEM_START_X  40
#define PC_IR_Y      12
#define PC_A_X       3
#define IR_B_X       15
#define A_B_Y        13
#define MAR_MDR_Y    14
#define CC_Y         15
#define CC_X         3
#define WIN_HEIGHT   40
#define WIN_WIDTH    70
#define WIN_Y        0	/* Calculating for a center placement */
#define WIN_X        0	/* of the window		*/
#define PROMPT_Y     19
#define PROMPT_X     5
#define CONSOLE_Y    20
#define CONSOLE_X    5
#define IO_START_X   4
#define IO_START_Y   25

//ncurses prompt options
#define PROMPT_LOAD '1'
#define PROMPT_STEP '2'
#define PROMPT_RUN  '4'
#define PROMPT_MEM  '5'
#define PROMPT_QUIT '9'

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
