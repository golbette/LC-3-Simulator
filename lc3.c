/*
 * lc3.c
 *
 *  Date Due: Apr 8, 2018
 *  Authors:  Michael Josten, and Samantha Anderson
 *  version: 408a
 *
 * This Program will simulate an lc3 finite state machine
 * by taking the input of a hex file and simulating each
 * state based on the lc3 ISA and the hex instructions passed.
 */

 //imports
#include "lc3.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>

/*
* Simulates trap table lookup for now
*/
void trap(unsigned short vector, CPU_p *cpu, WINDOW * IO_Window)
{
    switch (vector)
    {
    case 0x25:

        
       // displayMem(cpu);
        //set the pc to the original starting location
        cpu->PC = cpu->originalPC;
        displayCPU(cpu);
        printf("==========HALT==========\n");
        exit(0);
        break;
        
        case 0x20:
    //getch
        TRAP_getch(cpu, IO_Window);
        break;
        
    case 0x21:
    //out
        TRAP_out(cpu, IO_Window);
        break;
        
    case 0x22:
    //puts
        TRAP_puts(cpu, IO_Window);
        break;
        

    default:
        printf("Err: Unknown Trap vector?\n");
        
    }
}

void TRAP_getch(CPU_p * cpu, WINDOW * IO_Window) {
    cpu->reg[0] = getch();
    
}

void TRAP_out(CPU_p * cpu, WINDOW * IO_Window) {
    mvwprintw(IO_Window, IO_START_X + 2, IO_START_Y + 3 , "%c", cpu->reg[0]);
    
}

void TRAP_puts(CPU_p * cpu, WINDOW * IO_Window) {
    
    char c = cpu->memory[cpu->reg[0]];
    unsigned short int memindex = cpu->reg[0];
    
    while (c != '\n')
    {
        mvwprintw(IO_Window, IO_START_X + 2, IO_START_Y + 3 , "%c", c);
        memindex++;
        c = cpu->memory[memindex];
    }
    
}

/*
* This function displays the first 32 spots in memory
* to the console
*/
void displayMem(CPU_p *cpu)
{
    printf("\nMemory:\n");
    int i;
    for(i = 0; i < 32; i++) {
        printf("%2u  0x%04X \n", i, cpu->memory[i]);
    }
}

/*
* This function simulates the finite state machine as it runs through a
* set of instructions loaded into memory.
*/
int controller(CPU_p *cpu, short step, WINDOW * IO_Window) {

    // check to make sure both pointers are not NULL
    // do any initializations here
    //fields for the IR
    unsigned int opcode, dr, sr1, sr2, bit5, immed, state, condition;
    //fields for the ALU
    short result, A, B, offset;
    //trap vector
    unsigned short vector8;



    state = FETCH;
    for (;;) { // efficient endless loop to be used in the next problem
        switch (state) {
        case FETCH: // microstates 18, 33, 35 in the book.

            cpu->mar = cpu->PC;          // Step 1: MAR is loaded with the contends of the PC,
            cpu->PC++;                  //         and also increment PC. Only done in the FETCH phase.
            cpu->mdr = cpu->memory[cpu->mar]; // Step 2: Interrogate memory, resulting in the instruction placed into the MDR.
            cpu->ir = cpu->mdr;          // Step 3: Load the IR with the contents of the MDR.
            state = DECODE;
            break;

        case DECODE: // microstate 32

            opcode = cpu->ir & MASK_OPCODE; // Input is the four-bit opcode IR[15:12]. The output line asserted is the one corresponding to the opcode at the input.
            opcode = opcode >> 12;

            state = EVAL_ADDR;
            break;

        case EVAL_ADDR:

            // This phase computes the address of the memory location that is needed to process the instruction.
            // NOTE: Study each opcode to determine what all happens this phase for that opcode.
            // Look at the LD instruction to see microstate 2 example.
            switch (opcode) {
                // different opcodes require different handling
                // compute effective address, e.g. add sext(immed7) to
                case OP_LD:
                    dr = cpu->ir & MASK_DR;
                    dr = dr >> 9;
                    offset = cpu->ir & MASK_PCOFFSET9;
                    //check for negative offset and change to negative if over (2^9-1) 255
                    if (offset > 255) {
                        offset = offset - 256;
                    }
                    cpu->mar = (cpu->PC) + offset; // microstate 2.

                    break;
                case OP_ST:
                    dr = cpu->ir & MASK_DR;// This is actually a source register, but still use dr.
                    dr = dr >> 9;
                    offset = cpu->ir & MASK_PCOFFSET9;
                    if (offset > 255) {
                        offset = offset - 256;
                    }
                    cpu->mar = (cpu->PC) + offset; // microstate 3.
                    break;
            }
            state = FETCH_OP;
            break;

        case FETCH_OP:

            switch (opcode) {
                // get operands out of registers into A, B of ALU
                // or get memory for load instr.

                case OP_ADD:
                case OP_AND:
                    dr = cpu->ir & MASK_DR;
                    dr = dr >> 9;
                    sr1 = cpu->ir & MASK_SR1;
                    sr1 = sr1 >> 6;
                    cpu->A = cpu->reg[sr1];
                    bit5 = cpu->ir & MASK_BIT5;
                    bit5 = bit5 >> 5;
                    if (bit5 == 0) {
                        sr2 = cpu->ir & MASK_SR2; // no shift needed.
                        B = cpu->reg[sr2];
                    } else if (bit5 == 1) {
                        immed = cpu->ir & MASK_IMMED5; // no shift needed.
                        cpu->B = immed;
                        //check if larger than 15 (2^4 - 1)
                        if (cpu->B > 15) {
                            // subtract by 32 to get the signed negative number.
                            cpu->B = cpu->B - 32;
                        }
                    }
                    break;

                case OP_NOT:
                    dr = cpu->ir & MASK_DR;
                    dr = dr >> 9;
                    sr1 = cpu->ir & MASK_SR1;
                    sr1 = sr1 >> 6;
                    cpu->A = cpu->reg[sr1];
                    break;

                case OP_TRAP:
                    vector8 = cpu->ir & MASK_TRAPVECT8;
                    break;

                case OP_LD:
                    cpu->mdr = cpu->memory[cpu->mar];
                    break;

                case OP_ST:
                    cpu->mdr = cpu->reg[dr];
                    break;

                case OP_JMP:
                    sr1 = cpu->ir & MASK_SR1;
                    sr1 = sr1 >> 6;
                    cpu->A = cpu->reg[sr1];
                    break;

                case OP_BR:
                    condition = cpu->ir & MASK_NZP;
                    condition = condition >> 9;
                    offset = cpu->ir & MASK_PCOFFSET9;
                    break;

                default:
                    break;
            }
            state = EXECUTE;

            break;

        case EXECUTE:

            switch (opcode) {
                case OP_ADD:
                    result = cpu->A + cpu->B;
                    cpu->cc = getConditionCode(result);
                    break;

                case OP_AND:
                    result = cpu->A & cpu->B;
                    cpu->cc = getConditionCode(result);
                    break;

                case OP_NOT:
                    result = ~cpu->A;
                    cpu->cc = getConditionCode(result);
                    break;

                case OP_TRAP:
                    trap(vector8, cpu, IO_Window);
                    break;

                case OP_JMP:
                    cpu->PC = cpu->A;
                    break;

                case OP_BR:
                     if (checkBranchEnabled(condition, cpu)) {
                        cpu->PC = cpu->PC + offset;
                    }
                    break;
            }

            state = STORE;
            break;

        case STORE:

            switch (opcode) {
            // write back to register or store MDR into memory
            case OP_ADD:
            case OP_AND: // Same as ADD
            case OP_NOT: // Same as AND and AND.
                cpu->reg[dr] = result;
                break;

            case OP_LD:
                cpu->reg[dr] = cpu->mdr; // Load into the register.
                cpu->cc = getConditionCode(cpu->reg[dr]);
                break;

            case OP_ST:
                cpu->memory[cpu->mar] = cpu->mdr;     // Store into memory.
                break;

            }

            // do any clean up here in prep for the next complete cycle

            state = FETCH;
            //printf("End of Instruction\n");

            break;
        } // end switch (state)
        
        //breaks out of loop when
        if (state == FETCH && step == 1) {
              break;
        }
    } // end for()

   // displayMem(cpu);
    return 0;
} // end controller()

/*
* This function returns the condition code based on the value passed.
* negative numbers are not used yet because everything is unsigned.
* so any number above 32,767 (2^15-1) would be negative if it was signed.
*/
unsigned short getConditionCode(unsigned short value) {
    unsigned short code = 0;
    unsigned short maxSignedValue = 32767;



    if (value == 0) { //set cc to zero
        code = CONDITION_Z;
    } else if (value > maxSignedValue) { //set cc to N
        code = CONDITION_N;
    } else if (value <= maxSignedValue) { // set cc to P
        code = CONDITION_P;
    }
    return code;
}

/*
* This function will take the condition code for the branch
* instruction and compare it to the cc in the cpu and decide if the branch
* will be taken.
*/
unsigned short checkBranchEnabled (unsigned short condition, CPU_p *cpu) {
    unsigned short result = 0;

    switch (condition) {

    case CONDITION_NZP:
        result = 1;
        break;
    case CONDITION_NP:
        if (cpu->cc == CONDITION_N || cpu->cc == CONDITION_P)
            result = 1;
        break;
    case CONDITION_NZ:
        if (cpu->cc == CONDITION_N || cpu->cc == CONDITION_Z)
            result = 1;
        break;
    case CONDITION_ZP:
        if (cpu->cc == CONDITION_Z || cpu->cc == CONDITION_P)
            result = 1;
        break;
    case CONDITION_N:
        if (cpu->cc == CONDITION_N)
            result = 1;
        break;
    case CONDITION_Z:
        if (cpu->cc == CONDITION_Z)
            result = 1;
        break;
    case CONDITION_P:
        if (cpu->cc == CONDITION_P)
            result = 1;
        break;
    }

    return result;
}


/*
 * Print out fields to the console for the CPU_p object.
 */
void displayCPU(CPU_p *cpu) {
    //int i;
    //printf("Registers: \n");
    //for (i=0; i<8; i++) {
     //   printf("Register[%d] = %04X\n", i, cpu->reg[i]);
    //}
    //printf("PC = %04X   cc = %04X   ir = %04X   mar = %04X   mdr = %04X\n\n", cpu->PC,
    //       cpu->cc, cpu->ir, cpu->mar, cpu->mdr);
    
    //start ncurses mode here, set up the simulator window
    initscr();
    cbreak();	
    keypad(stdscr, TRUE);		
    char ch; 
    refresh();
    WINDOW *local_win;
    local_win = newwin(WIN_HEIGHT, WIN_WIDTH, WIN_X, WIN_Y);

    
    refreshDisplay(cpu, local_win);
    
	
	char option = 0; 

    //Everything here is printed using ncurses, mvwprintw and mvwscanw are window-specific analogues to printf and scanf that take Y/X coords (not X/Y!)
    //ncurses requires manual refresh and can be finnicky, so there might be a few too many refresh() and wrefresh(window) calls
    while (option != PROMPT_QUIT)
    {
        mvwprintw(local_win, PROMPT_Y, PROMPT_X, "Select: 1) Load 2) Step 4) Run 5) Display mem 9) exit ");
	    refresh(); 
	    wrefresh(local_win);
        mvwscanw(local_win, CONSOLE_Y, CONSOLE_X,  "%c", &option);
        
        switch(option)
        {
            case PROMPT_LOAD:
            
                
                mvwprintw(local_win, PROMPT_Y, PROMPT_X, "Enter the file name:                                    ");
                wmove(local_win, CONSOLE_Y, CONSOLE_X);
                wclrtoeol(local_win);
                box(local_win, 0 , 0);
        	    refresh(); 
        	    wrefresh(local_win);
        	    char filename[100];
        	    mvwscanw(local_win,  CONSOLE_Y, CONSOLE_X,  "%s", filename);
        	    if (loadInstructions(cpu, filename) == -1)
        	    {
        	        noecho();
        	        mvwprintw(local_win, PROMPT_Y, PROMPT_X, "Error: File not found. Press <ENTER> to continue         ");
        	        int key = wgetch(local_win);
        	        while (key != 10)
        	        {
                        key = wgetch(local_win);
        	        }
        	        echo();
        	        
        	    }
        	    refreshDisplay(cpu, local_win);
        	    break;
        
        
            case PROMPT_RUN:
                controller(cpu, 0, local_win);
                refreshDisplay(cpu, local_win);
                break;
        
            case PROMPT_STEP:
            
                controller(cpu, 1, local_win);
                refreshDisplay(cpu, local_win);
                break;
        
        
         
            case PROMPT_MEM:
            
                mvwprintw(local_win, PROMPT_Y, PROMPT_X, "Enter the new start memory location:                                    ");
                wmove(local_win, CONSOLE_Y, CONSOLE_X);
                wclrtoeol(local_win);
                box(local_win, 0 , 0);
        	    refresh(); 
        	    wrefresh(local_win);
        	    unsigned int newPCStart;
        	    mvwscanw(local_win,  CONSOLE_Y, CONSOLE_X,  "0x%x", &newPCStart);
        	    if (newPCStart > 0xFFFF)
        	    {
        	        noecho();
        	        mvwprintw(local_win, PROMPT_Y, PROMPT_X, "Not a valid address <ENTER> to continue.                                    ");
        	        int key = wgetch(local_win);
        	        while (key != 10)
        	        {
        	            
                        key = wgetch(local_win);
        	        }
        	        echo();
        	    }
        	    else
        	    cpu->originalPC = newPCStart;
        	    
        	    refreshDisplay(cpu, local_win);
        	    //mvwprintw(local_win, WIN_HEIGHT - 7, 5, "0x%x", newPCStart);
                break;
        }
    }
    delwin(local_win);
    endwin();
}


/*Refreshes the CPU display. Call this after every STEP, memory change, instruction set load, etc. */
void refreshDisplay(CPU_p *cpu, WINDOW *local_win){
    //Wipe the window to get rid of artifacts
    werase(local_win);
    wrefresh(local_win);
    
    //Drawing REG and MEM values happens here
	unsigned short int i;
	mvwprintw(local_win, REG_START_Y, REG_START_X, "Registers");
	for (i = 0; i < REG_MAX; i++){
		mvwprintw(local_win, REG_START_Y+i, REG_START_X, "R%d: 0x%04x     ", i, cpu->reg[i]);
	}
	
	mvwprintw(local_win, MEM_START_Y, MEM_START_X, "Memory");
	for (i = 0; i < 16; i++) {
		mvwprintw(local_win, MEM_START_Y+i, MEM_START_X, "MEM 0x%04x:    0x%04x", i+cpu->originalPC, cpu->memory[cpu->originalPC+i]);
	}

    //figure out CC values
    char N = '0';
    char Z = '0';
    char P = '0';
    
    if (cpu->cc == CONDITION_N) N = '1';
    if (cpu->cc == CONDITION_Z) Z = '1';
    if (cpu->cc == CONDITION_P) P = '1';
    
    //Various other registers print here
	mvwprintw(local_win, PC_IR_Y, PC_A_X, "PC:  0x%04x", cpu->PC);
	mvwprintw(local_win, PC_IR_Y, IR_B_X, "IR:  0x%04x", cpu->ir);
	mvwprintw(local_win, A_B_Y, PC_A_X, "A:   0x%04x", cpu->A);
	mvwprintw(local_win, A_B_Y, IR_B_X, "B:   0x%04x", cpu->B);
	mvwprintw(local_win, MAR_MDR_Y, PC_A_X, "MAR: 0x%04x MDR: 0x%04x ", cpu->mar, cpu->mdr);
	mvwprintw(local_win, CC_Y, CC_X, "CC:  N: %c  Z: %c  P: %c ", N, Z, P);
	//wmove(local_win, CONSOLE_X, 5);
	//wmove(local_win, WIN_HEIGHT - 5, 5);
	
	//A little snippet area foR IO. Doesn't do much yet. 
	mvwprintw(local_win, IO_START_Y, IO_START_X, "-------------------------------------------------------------");
	mvwprintw(local_win, IO_START_Y+1, IO_START_X, "Input: ");
	mvwprintw(local_win, IO_START_Y+2, IO_START_X, "Output: ");
	
	//box makes a nice border. can substitute 0,0 for cuter windows
	box(local_win, 0 , 0);
	refresh();
	wrefresh(local_win);
}


/*
 * Helper function that sets all elements in an array to zero.
 */
void zeroOut(unsigned short *array, int quantity) {
    int i;
    for (i = 0; i <= quantity; i++) {
        array[i] = 0;
    }
}

/*
 * Initializes a CPU_p structure.
 */
CPU_p initializeCPU() {
//    CPU_p cpu = { 0    // PC
//                , 0    // cc
//                , { 0, 0, 0, 0, 0, 0, 0, 0 } //
//                , 0    // ir
//                , 0    // mar
//                , 0 }; // mdr
    CPU_p cpu;

    //zeroOut(cpu->memory, 65535);  //set memory to zero.


    return cpu;
}

/*
* This function will take the hex file name and
* a pointer to the cpu struct and will load each hex instruction
* into sequential memory. This function will also set the pc to
* the value of the first hex value in the hex file.
*/
int loadInstructions(CPU_p *cpu, char *fileName) {
    //number of characters to read from each line
    short instructionSize = 8;

    //check is the fileName is a null pointer
    if (fileName == NULL) {
       // printf("Argument is NULL\n");
        return -1;
    }
    //check if the file is null
    FILE *input;
    input = fopen(fileName, "r");

    if (input == NULL) {
      //  printf("File does not exist\n");
        return -1;
    }

    //first line in the instruction file is the pc
    char *instructionString = (char*) malloc(sizeof(char) * instructionSize);
    fgets(instructionString, instructionSize, input);
    cpu->PC = strtol(instructionString, NULL, 16);
    cpu->originalPC = cpu->PC;

    //load the rest of the instruction into the memory
    unsigned short int i = cpu->originalPC;
    while (fgets(instructionString, instructionSize, input) != NULL) {
        cpu->memory[i] = strtol(instructionString, NULL, 16);
        i++;
    }
    
    //set up some interesting register values for demo
    for (i = 0; i<REG_MAX; i++){
        cpu->reg[i] = i+1;
    }
    return cpu->PC;
}


/*
 * Driver for the program.
 */
int main(int argc, char* argv[]) {


    CPU_p cpu = initializeCPU();
    displayCPU(&cpu);
    //loadInstructions(&cpu, argv[1]);
    

    //if step = 0, regular program, if step = 1, just 1 instruction.
    //short step = 0;
    //controller(&cpu, step);


	return 0;
}
