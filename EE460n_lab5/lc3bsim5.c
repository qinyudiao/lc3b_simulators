/*
    Name 1: Qinyu Diao
    UTEID 1: qd572
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N - Lab 5                                           */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         pagetable    page table in LC-3b machine language   */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    AMSD,IRD,
    COND2, COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
/* MODIFY: you have to add all your new control signals */
    LD_SSP_R,
    LD_USP_R,
    LD_R6,
    LD_EXCV,
    LD_VR,
    LD_PSR_MODE,
    GATE_PCMIN2,
    GATE_PSR,
    GATE_VR,
    GATE_R6,
    R6MUX1, R6MUX0,
    CCMUX,
    VRMUX,
    LD_VA,
    LD_MDR_BIT,
    LD_AMSR,
    LD_WSR,
    CL_WSR,
    GATE_VA_PA,
    GATE_ADDR_PTE,
    GATE_WSR,
    GATE_MDR_0_R,
    PAGE_FAULT,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return((x[AMSD] << 1) + x[IRD]); }
int GetCOND(int *x)          { return((x[COND2] << 2) + (x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */
int GetLD_SSP_R(int *x)      { return(x[LD_SSP_R]); }
int GetLD_USP_R(int *x)      { return(x[LD_USP_R]); }
int GetLD_R6(int *x)         { return(x[LD_R6]); }
int GetLD_EXCV(int *x)       { return(x[LD_EXCV]); }
int GetLD_VR(int *x)         { return(x[LD_VR]); }
int GetLD_PSR_MODE(int *x)   { return(x[LD_PSR_MODE]); }
int GetGATE_PCMIN2(int *x)   { return(x[GATE_PCMIN2]); }
int GetGATE_PSR(int *x)      { return(x[GATE_PSR]); }
int GetGATE_VR(int *x)       { return(x[GATE_VR]); }
int GetGATE_R6(int *x)       { return(x[GATE_R6]); }
int GetR6MUX(int *x)         { return((x[R6MUX1] << 1) + x[R6MUX0]); }
int GetCCMUX(int *x)         { return(x[CCMUX]); }
int GetVRMUX(int *x)         { return(x[VRMUX]); }
int GetLD_VA(int *x)         { return(x[LD_VA]); }
int GetLD_MDR_BIT(int *x)    { return(x[LD_MDR_BIT]); }
int GetLD_AMSR(int *x)       { return(x[LD_AMSR]); }
int GetLD_WSR(int *x)        { return(x[LD_WSR]); }
int GetCL_WSR(int *x)        { return(x[CL_WSR]); }
int GetGATE_VA_PA(int *x)    { return(x[GATE_VA_PA]); }
int GetGATE_ADDR_PTE(int *x) { return(x[GATE_ADDR_PTE]); }
int GetGATE_WSR(int *x)      { return(x[GATE_WSR]); }
int GetGATE_MDR_0_R(int *x)  { return(x[GATE_MDR_0_R]); }
int GetPage_Fault(int *x)    { return(x[PAGE_FAULT]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x2000 /* 32 frames */ 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
/* MODIFY: you should add here any other registers you need to implement interrupts and exceptions */
int I;
int E;
int VR;
int USP; /* Initial value of user stack pointer */
int SSP_R; //Supervisor_Stack_Pointer_Register
int USP_R; //User_Stack_Pointer_Register
int PSR; // Program status register mode:[15] conditional codes:[2-0]

/* For lab 5 */
int PTBR; /* This is initialized when we load the page table */
int VA;   /* Temporary VA register */
int AMSR;
int WSR;
/* MODIFY: you should add here any other registers you need to implement virtual memory */

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/* For lab 5 */
#define PAGE_NUM_BITS 9
#define PTE_PFN_MASK 0x3E00
#define PTE_VALID_MASK 0x0004
#define PAGE_OFFSET_MASK 0x1FF

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
    int address; /* this is a byte address */

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
    int k; 

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename, int is_virtual_base) {                   
    FILE * prog;
    int ii, word, program_base, pte, virtual_pc;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    if (is_virtual_base) {
      if (CURRENT_LATCHES.PTBR == 0) {
	printf("Error: Page table base not loaded %s\n", program_filename);
	exit(-1);
      }

      /* convert virtual_base to physical_base */
      virtual_pc = program_base << 1;
      pte = (MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][1] << 8) | 
	     MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][0];

      printf("virtual base of program: %04x\npte: %04x\n", program_base << 1, pte);
		if ((pte & PTE_VALID_MASK) == PTE_VALID_MASK) {
	      program_base = (pte & PTE_PFN_MASK) | ((program_base << 1) & PAGE_OFFSET_MASK);
   	   printf("physical base of program: %x\n\n", program_base);
	      program_base = program_base >> 1; 
		} else {
   	   printf("attempting to load a program into an invalid (non-resident) page\n\n");
			exit(-1);
		}
    }
    else {
      /* is page table */
     CURRENT_LATCHES.PTBR = program_base << 1;
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0 && is_virtual_base) 
      CURRENT_LATCHES.PC = virtual_pc;

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine         */
/*                                                             */
/***************************************************************/
void initialize(char *argv[], int num_prog_files) { 
    int i;
    init_control_store(argv[1]);

    init_memory();
    load_program(argv[2],0);
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(argv[i + 3],1);
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */

/* MODIFY: you can add more initialization code HERE */

    /* modified down*/
    CURRENT_LATCHES.USP_R = 0xFFE0; /* Initial value of user stack pointer */
    CURRENT_LATCHES.PSR = 0x8002;

    CURRENT_LATCHES.INTV = 0x0001;
    /* modified up*/

    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 4) {
	printf("Error: usage: %s <micro_code_file> <page table file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv, argc - 3);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/

#define MASK_ONE_BIT   0x0001
#define MASK_TWO_BITS  0x0003
#define MASK_FOUR_BITS 0x000F

int gate_Num = 0;
int count_cycle_mem = 0; // count 5 cycles for memory access

int shift_mask(int input, int shift_bits, int mask_bits);
void printCurrentRowOfCS();

void generate_Interrupt() {
    if(CYCLE_COUNT == 300){
        NEXT_LATCHES.I = 1;
        printf(" ******* Interrupt ********** ");
    }
}




void eval_micro_sequencer() {
  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */

    //printCurrentRowOfCS();
    // 
    //printf("%d", CURRENT_LATCHES.E);

    // int temp_PSR_15 = (CURRENT_LATCHES.PSR >> 15) & 0x01;
    // printf(" :%d: ", temp_PSR_15);
     printf("%d ", CURRENT_LATCHES.STATE_NUMBER);
    printf(":%d: ", CURRENT_LATCHES.E);
     //printf("MR%d ", CURRENT_LATCHES.MDR);
     
    // if(CURRENT_LATCHES.E != 0){
         //printf(":%d: ", CURRENT_LATCHES.MDR);
    //    printf("STATE: %d ", CURRENT_LATCHES.STATE_NUMBER);
    // }

    // if(CURRENT_LATCHES.STATE_NUMBER == 38){
    //     printf(" HIT THE RTI,");
    //     printf(" CYCLE_COUNT is: %d ", CYCLE_COUNT);
    // }
    // printf("  VR :%d ", CURRENT_LATCHES.VR);
    //  printf(" %d", CURRENT_LATCHES.PSR);
    //  printf(" PC:%d ", CURRENT_LATCHES.PC);
    //  printf(" MDR : %d ", CURRENT_LATCHES.MDR);
    //  printf(" MAR : %d ", CURRENT_LATCHES.MAR);
    //  printf(" R6 : %d ", CURRENT_LATCHES.REGS[6]);

    //  printf("%d", CURRENT_LATCHES.I);
     

    generate_Interrupt();


  int temp_IRD = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
  if(temp_IRD == 1){ // 0,0,IR[15:12] to Address of Next State
    int temp_state_num = shift_mask(CURRENT_LATCHES.IR, 12, MASK_FOUR_BITS);
    NEXT_LATCHES.STATE_NUMBER = Low16bits(temp_state_num);
  }
  else if((temp_IRD == 2) | (temp_IRD == 3)){ // AMSR to Address of Next State
    NEXT_LATCHES.STATE_NUMBER = CURRENT_LATCHES.AMSR;
  }
  else if(temp_IRD == 0){ // J[5:0] to Address of Next State
    int temp_COND = GetCOND(CURRENT_LATCHES.MICROINSTRUCTION);
    //printf("temp__COND is: %d\n", temp_COND);
    int temp_J = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
    int temp_IR_11 = shift_mask(CURRENT_LATCHES.IR, 11, MASK_TWO_BITS);
    // printf("temp__COND is: %d\n", temp_COND);
    // printf("tempJ is: %d\n", temp_J);
    if((temp_COND == 2) && (CURRENT_LATCHES.BEN == 1)){
        temp_J |= 0x04;
        //printf("tempJ after is: %d\n", temp_J);
    }
    //printf(" :%d: ", CURRENT_LATCHES.READY);
    if((temp_COND == 1) && (CURRENT_LATCHES.READY == 1))
        temp_J |= 0x02;
    if((temp_COND == 3) && temp_IR_11 == 1)
        temp_J |= 0x01;
        
    /* For lab 4 */
    // I
    if((temp_COND == 7) && CURRENT_LATCHES.I == 1)
        temp_J |= 0x08;
    // E
    int temp_E_0 = CURRENT_LATCHES.E & 0x0001;
    int temp_J_4 = (temp_J >> 4) & 0x01;
    //printf("tempJ after is: %d\n", temp_J); 
    if((temp_COND == 5) && (temp_E_0 == 1)){
        temp_J |= 0x10;
        if(temp_J_4 == 1){
            temp_J &= 0x00EF; //J4=0
            temp_J |= 0x20;   //J5=1
        }
    }
    /* For lab 4 */
    NEXT_LATCHES.STATE_NUMBER = temp_J;
  }

    for(int i=0; i < CONTROL_STORE_BITS; i++){
        NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][i];
    }
}

void cycle_memory() {
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */
  //printf("CONDa is: %d\n", GetCOND(CURRENT_LATCHES.MICROINSTRUCTION));
  if(GetCOND(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
      count_cycle_mem++;
      if(count_cycle_mem == 4){
          NEXT_LATCHES.READY = Low16bits(1);    //READY BIT is asserted at the end of 4th cycle
          count_cycle_mem = -1;
      }
  }
}

void eval_bus_drivers() {

  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *             Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   */    

  int Gate_Bits = (GetGATE_PCMIN2(CURRENT_LATCHES.MICROINSTRUCTION) << 8) +
                  (GetGATE_R6(CURRENT_LATCHES.MICROINSTRUCTION) << 7) +
			      (GetGATE_PSR(CURRENT_LATCHES.MICROINSTRUCTION) << 6) +
			      (GetGATE_VR(CURRENT_LATCHES.MICROINSTRUCTION) << 5) +
                  (GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION) << 4) +
			      (GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION) << 3) +
			  	  (GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION) << 2) +
				  (GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION) << 1) +
				  (GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION));
  if (Gate_Bits == 0) {
      gate_Num = 0;
      return;
  }
  gate_Num = 1;
  while ((Gate_Bits & 0x100) == 0) { //if not current gate, check next
      Gate_Bits = Gate_Bits << 1;
      gate_Num++;
  }
}


void drive_bus() {

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */       

  	int sr1;
	int sr2;
    int amount4;
    int temp_leftmost;

    /* gate 1-9 is PCMIN2, R6, PSR, VR, PC, MDR, ALU, MARMUX, SHF */
    //printf("BUS3:%d ", BUS);
	switch(gate_Num) {
	case 0:
        if(GetGATE_WSR(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
            //printf("WSR:%d ", CURRENT_LATCHES.WSR);
            BUS |= Low16bits(CURRENT_LATCHES.WSR << 1) & 0x02;
            //printf("BUS:%d ", BUS);
        }
        if(GetGATE_MDR_0_R(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
            if(CURRENT_LATCHES.VR == 1)
                BUS = Low16bits(BUS & 0x02);
            else
                BUS = Low16bits((BUS & 0x02) | 0x01);
        }

        else if(GetGATE_ADDR_PTE(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
            BUS = Low16bits(CURRENT_LATCHES.PTBR + (CURRENT_LATCHES.MAR >> 9 << 1));
        }

        else if(GetGATE_VA_PA(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
            //printf("BUS1%d ", BUS);
            //printf("MDR%d ", CURRENT_LATCHES.MDR);
            BUS =  (((CURRENT_LATCHES.MDR>>9)-(CURRENT_LATCHES.MDR>>14<<5))<<9)
                                     + (CURRENT_LATCHES.VA & PAGE_OFFSET_MASK);
            //printf("BUS2%d ", BUS);
        }
        //printf("\n reaches clear bus, current bus : %d \n", BUS);
		else BUS = Low16bits(0);
        //printf("\n reaches clear bus, current bus : %d \n", BUS);
        
		break;

/*   lab4   */
    /*GatePCMIN2*/
	case 1:
		BUS = Low16bits(CURRENT_LATCHES.PC - 2);
		break;
    /*GateR6*/
	case 2:
        if(GetR6MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 0) //USP
            BUS = Low16bits(CURRENT_LATCHES.USP_R);
        if(GetR6MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1) //SSP
            BUS = Low16bits(CURRENT_LATCHES.SSP_R);
        if(GetR6MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 2) //+2
            BUS = Low16bits(CURRENT_LATCHES.REGS[6]+2);
        if(GetR6MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 3) //-2
            BUS = Low16bits(CURRENT_LATCHES.REGS[6]-2);
		break;
	/*GatePSR*/
	case 3:
		BUS = Low16bits(CURRENT_LATCHES.PSR);
		break;
	/*GateVR*/
	case 4:
		BUS = Low16bits((CURRENT_LATCHES.VR << 1) + 0x200);
		break;
/*   lab4   */
	/*GatePC*/
	case 5:
		BUS = Low16bits(CURRENT_LATCHES.PC);
		break;
	/*GateMDR*/
	case 6:
		BUS = Low16bits(CURRENT_LATCHES.MDR);
		break;
	/*GateALU*/
	case 7:
        if(GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION) == 0){
            if(((CURRENT_LATCHES.IR >> 5) & 0x01) == 1) {
                if(((CURRENT_LATCHES.IR >> 4) & 0x01) == 0) {
                    sr2 = CURRENT_LATCHES.IR & 0x001F;
                }
                else {
                    sr2 = (CURRENT_LATCHES.IR | 0xFFFFFFE0);
                }
            }
            else if(((CURRENT_LATCHES.IR >> 5) & 0x01) == 0){
                sr2 = CURRENT_LATCHES.IR & 0x0007;
                sr2 = CURRENT_LATCHES.REGS[sr2];
            }
            sr1 = (CURRENT_LATCHES.IR >> 6) & 0x0007;
            sr1 = CURRENT_LATCHES.REGS[sr1];

        switch(GetALUK(CURRENT_LATCHES.MICROINSTRUCTION)) {
            /*ADD*/
            case 0:
                BUS = Low16bits(sr1 + sr2);
                break;
            /*AND*/
            case 1:
                BUS = Low16bits(sr1 & sr2);
                break;
            /*XOR*/
            case 2:
                BUS = Low16bits(sr1 ^ sr2);
                break;
            /*PASSA*/
            case 3:
                BUS = Low16bits(sr2);
                break;
            }
        }
        else if(GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION) == 1){   //ld_MDR = 1; stae 23,24
            if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) == 1) {
                int SR = (CURRENT_LATCHES.IR >> 9) & 0x0007;
				BUS = CURRENT_LATCHES.REGS[SR];
			}
		    else {
                int SR = (CURRENT_LATCHES.IR >> 9) & 0x0007;
                BUS = CURRENT_LATCHES.REGS[SR] & 0x00FF;
				//NEXT_LATCHES.MDR = BUS & 0x00FF;
                //printf("MDR: %d \n", NEXT_LATCHES.MDR);
                //printf("BUS: %d \n", BUS);
                
		    }
        }
        break;
	/*GateMARMUX*/
	case 8:
		if(GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 0){
			BUS = Low16bits((CURRENT_LATCHES.IR & 0x00FF) << 1);
		}
		else if(GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
			if(GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 0){
				sr1 = CURRENT_LATCHES.PC;
			}
			else if(GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
				if(GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
					sr1 = (CURRENT_LATCHES.IR >> 6) & 0x0007;
				}
				else{
					sr1 = (CURRENT_LATCHES.IR >> 9) & 0x0007;
				}
				sr1 = CURRENT_LATCHES.REGS[sr1];
			}
			switch(GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION)){
                case 0:
                    sr2 = 0x0000;
                    break;
                case 1:
                    sr2 = CURRENT_LATCHES.IR & 0x003F;
                    temp_leftmost = (CURRENT_LATCHES.IR >> 5) & 0x01;
                    if (temp_leftmost == 1) {      // if (sr2 < 0)
                        sr2 = sr2 - 64;
                    }
                    break;
                case 2:
                    sr2 = CURRENT_LATCHES.IR & 0x01FF;
                    temp_leftmost = (CURRENT_LATCHES.IR >> 8) & 0x01;
                    if (temp_leftmost == 1) {      // if (sr2 < 0)
                        sr2 = sr2 - 512;
                    }
                    break;
                case 3:
                    sr2 = CURRENT_LATCHES.IR & 0x07FF;
                    temp_leftmost = (CURRENT_LATCHES.IR >> 10) & 0x01;
                    if (temp_leftmost == 1) {      // if (sr2 < 0)
                        sr2 = sr2 - 2048;
                    }
				break;
			}
			if(GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
				sr2 = sr2 << 1;
			}
			BUS = sr1 + sr2;
		}
		break;
	/*GateSHF*/
	case 9:
		amount4 = CURRENT_LATCHES.IR & 0x000F;      //amount4
		sr1 = (CURRENT_LATCHES.IR >> 6) & 0x0007;
		sr1 = CURRENT_LATCHES.REGS[sr1];
		switch((CURRENT_LATCHES.IR >> 4) & 0x0003){ // bits 4,5 of IR
		case 0: // LSHF
			BUS = Low16bits(sr1 << amount4);
			break;
		case 1: // RSHFL ;DR = RSHF(SR, amount4, 0);
            BUS = Low16bits(sr1 >> amount4);
			break;
		case 3: // RSHFA ;DR = RSHF(SR, amount4, SR[15]);
            temp_leftmost = shift_mask(sr1, 15, MASK_ONE_BIT);
            if(temp_leftmost == 1){
                for(int i = amount4; i > 0; i--) 
					temp_leftmost = Low16bits((temp_leftmost >> 1) + 0x8000);
                BUS = Low16bits( (sr1 >> amount4) | (temp_leftmost) );
            }
			else{
				BUS = Low16bits(sr1 >> amount4);
			}
			break;
		break;
        }        
	}
}


void latch_datapath_values() {

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */
    int PFG = GetPage_Fault(CURRENT_LATCHES.MICROINSTRUCTION);
    //printf("MDR:%d ", CURRENT_LATCHES.MDR);
   if(CURRENT_LATCHES.STATE_NUMBER == 60){
       NEXT_LATCHES.VR = 0;
   }
    /*LD_MAR*/
    if (GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION) == 1) {
		NEXT_LATCHES.MAR = BUS;
        int temp_PSR_15 = (CURRENT_LATCHES.PSR >> 15) & 0x01;
                    //printf("MAR:%d ", NEXT_LATCHES.MAR);
        if(CURRENT_LATCHES.STATE_NUMBER != 15){ //do not check exception in Trap
            if((NEXT_LATCHES.MAR & 0x01) == 1){ //unaligned access
                //printf("UA EXCEPTION ");
                if((GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) == 1) ){ //check if it is word
                    NEXT_LATCHES.E = 5; //generate the UA exception in state 2,3,6,7
                }
                else if(CURRENT_LATCHES.STATE_NUMBER == 50){
                    NEXT_LATCHES.E = 5; //generate the UA exception in state 50
                }
            }
            else if(((NEXT_LATCHES.MAR < 0xC00) & temp_PSR_15) == 1){
               if(CURRENT_LATCHES.STATE_NUMBER == 43) 
                    NEXT_LATCHES.E = 3; //protection
                //printf("PROTECTION EXCEPTION :%d ", NEXT_LATCHES.E);
            }
         }
	}
    else if( ( (!(CURRENT_LATCHES.MDR >> 2)) & (PFG == 1) ) == 1 ) {//state 45
        
        NEXT_LATCHES.E = 1; //page fault
    }
	/*LD_MDR*/
	if (GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION) == 1) {
		if (GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION) == 0) {
            NEXT_LATCHES.MDR = Low16bits(BUS);            
		}
		else {
			if (CURRENT_LATCHES.READY == 1) {
				if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) == 1) {
					NEXT_LATCHES.MDR = (MEMORY[CURRENT_LATCHES.MAR/2][0] & 0x00FF) + ((MEMORY[CURRENT_LATCHES.MAR/2][1] << 8) & 0xFF00);
				}
				else {  //Byte access
					if (CURRENT_LATCHES.MAR & 0x01) {
						NEXT_LATCHES.MDR = MEMORY[CURRENT_LATCHES.MAR/2][1] & 0x00FF;
					}
					else {
						NEXT_LATCHES.MDR = MEMORY[CURRENT_LATCHES.MAR/2][0] & 0x00FF;
					}
					if ((NEXT_LATCHES.MDR >> 7) == 1) {    // check negativity
						NEXT_LATCHES.MDR = NEXT_LATCHES.MDR | 0xFF00;
					}
				}
				NEXT_LATCHES.READY = 0;
			}
                        //printf("MDR: %d \n", NEXT_LATCHES.MDR);
		}
	}
	/*LD_IR*/
	if(GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
		NEXT_LATCHES.IR = BUS;
	}
	/*LD_BEN*/
	if(GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
		NEXT_LATCHES.BEN = (((CURRENT_LATCHES.IR >> 11) & 0x0001) & CURRENT_LATCHES.N) | (((CURRENT_LATCHES.IR >> 10) & 0x0001) &
			CURRENT_LATCHES.Z) | (((CURRENT_LATCHES.IR >> 9) & 0x0001) & CURRENT_LATCHES.P);
	}
	/*LD_REG*/
	if(GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
		if(GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 0){
			NEXT_LATCHES.REGS[(CURRENT_LATCHES.IR >> 9) & 0x0007] = BUS;
		}
		else{
			NEXT_LATCHES.REGS[7] = BUS;
		}
	}
	/*LD_CC*/
	if(GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
        //printf("currentBUS: %d \n", BUS);
        if(GetCCMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 0){
            if(BUS > 0 && BUS < 32768){
                NEXT_LATCHES.N = 0;
                NEXT_LATCHES.Z = 0;
                NEXT_LATCHES.P = 1;
            }
            else if(BUS == 0){
                NEXT_LATCHES.N = 0;
                NEXT_LATCHES.Z = 1;
                NEXT_LATCHES.P = 0;
            }
            else{
                NEXT_LATCHES.N = 1;
                NEXT_LATCHES.Z = 0;
                NEXT_LATCHES.P = 0;
            }
        }
        //in state 46
        else{
            NEXT_LATCHES.N = (CURRENT_LATCHES.PSR >> 2) & 0x01;
            NEXT_LATCHES.Z = (CURRENT_LATCHES.PSR >> 1) & 0x01;
            NEXT_LATCHES.P = CURRENT_LATCHES.PSR & 0x01;
        }
        NEXT_LATCHES.PSR = (CURRENT_LATCHES.PSR & 0x8000)
                            + (NEXT_LATCHES.N << 2)
                            + (NEXT_LATCHES.Z << 1)
                            + (NEXT_LATCHES.P);
	}
	/*LD_PC*/
	int sr1;
	int sr2;
	if(GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
        //printf("current PCMUX: %d \n", GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION));
		switch (GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION)){
		case 0:
			NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 0x0002;
			break;

        case 3:
			NEXT_LATCHES.PC = CURRENT_LATCHES.PC - 0x0002;
			break;

		case 1:
            //printf("currentbus: %d \n", BUS);
			NEXT_LATCHES.PC = BUS;
			break;

		case 2:
            //printf("current ADDR1MUX: %d \n", GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION));
			if(GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1){  //ADDR1MUX = 1 -> BASER
				if (GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
					sr1 = (CURRENT_LATCHES.IR >> 6) & 0x0007;
                    sr1 = CURRENT_LATCHES.REGS[sr1];
				}
				else{
					sr1 = (CURRENT_LATCHES.IR >> 9) & 0x0007;
                    sr1 = CURRENT_LATCHES.REGS[sr1];
				}
			}
			else{
				sr1 = CURRENT_LATCHES.PC;           //ADDR1MUX = 0 -> PC
			}
			switch(GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION)){
			case 0:
				sr2 = 0x0000;
				break;

			case 1:
				if (((CURRENT_LATCHES.IR >> 5) & 0x0001) == 1){
					sr2 = CURRENT_LATCHES.IR | 0xFFFFFFE0;
				}
				else {
					sr2 = CURRENT_LATCHES.IR & 0x001F;
				}
				break;

			case 2:
				if (((CURRENT_LATCHES.IR >> 8) & 0x0001) == 1){
					sr2 = CURRENT_LATCHES.IR | 0xFFFFFE00;
				}
				else {
					sr2 = CURRENT_LATCHES.IR & 0x01FF;
				}

				break;

			case 3:
				if(((CURRENT_LATCHES.IR >> 10) & 0x0001) == 1){
					sr2 = CURRENT_LATCHES.IR | 0xFFFFF800;
				}
				else{
					sr2 = CURRENT_LATCHES.IR & 0x07FF;
				}
				break;
			}
			if(GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
				sr2 = sr2 << 1;
			}
			NEXT_LATCHES.PC = Low16bits(sr1 + sr2);
			break;
		}
	}
	/*R_W*/
	if(GetR_W(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
		if (CURRENT_LATCHES.READY == 1){
			if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
				MEMORY[CURRENT_LATCHES.MAR/2][0] = (CURRENT_LATCHES.MDR & 0x00FF);
				MEMORY[CURRENT_LATCHES.MAR/2][1] = (CURRENT_LATCHES.MDR >> 8) & 0x00FF;
			}
			else{   //byte access
				if((CURRENT_LATCHES.MAR & 0x01) == 1){
					MEMORY[CURRENT_LATCHES.MAR/2][1] = CURRENT_LATCHES.MDR & 0x00FF;
				}
				else{
					MEMORY[CURRENT_LATCHES.MAR/2][0] = CURRENT_LATCHES.MDR & 0x00FF;
				}
			}
            NEXT_LATCHES.READY = 0;
		}
    }
    /*LD_SSP_R*/
    if(GetLD_SSP_R(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
        NEXT_LATCHES.SSP_R = CURRENT_LATCHES.REGS[6];
    }
    /*LD_USP_R*/
    if(GetLD_USP_R(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
        NEXT_LATCHES.USP_R = CURRENT_LATCHES.REGS[6];
    }
    /*LD_R6*/
    if(GetLD_R6(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
            NEXT_LATCHES.REGS[6] = Low16bits(BUS);
    }
    /*LD_EXCV*/
    if(GetLD_EXCV(CURRENT_LATCHES.MICROINSTRUCTION) == 1){


        if(CURRENT_LATCHES.E == 0)   //Unknown opcode
            NEXT_LATCHES.EXCV = 5;
        if(CURRENT_LATCHES.E >= 4)   //unaligned access
            NEXT_LATCHES.EXCV = 3;
        if(CURRENT_LATCHES.E == 3)   //protection
            NEXT_LATCHES.EXCV = 4;
        if(CURRENT_LATCHES.E == 1)   //page fault
            NEXT_LATCHES.EXCV = 2;
    }
    /*LD_VR*/
    if(GetLD_VR(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
        if(GetVRMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 0)
            NEXT_LATCHES.VR = CURRENT_LATCHES.EXCV;
        else if(GetVRMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
            NEXT_LATCHES.VR = CURRENT_LATCHES.INTV;

        NEXT_LATCHES.I = 0;
        NEXT_LATCHES.E = 0;
    }
    /*LD_PSR_MODE*/
    if(GetLD_PSR_MODE(CURRENT_LATCHES.MICROINSTRUCTION) == 1){

        if(GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
            NEXT_LATCHES.PSR |= 0x8000;
        }
        else{
            NEXT_LATCHES.PSR &= 0x7FFF;
        }
    }
    /*LD_VA*/
    if(GetLD_VA(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
            NEXT_LATCHES.VA = BUS;
    }
    /*LD_MDR_BIT*/
    if(GetLD_MDR_BIT(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
            if(CURRENT_LATCHES.VR == 1)
                NEXT_LATCHES.MDR |= Low16bits(BUS & 0xFFFE);
            else
            
                NEXT_LATCHES.MDR |= BUS;
    }
    /*LD_AMSR*/
    if(GetLD_AMSR(CURRENT_LATCHES.MICROINSTRUCTION) == 1){

        if(CURRENT_LATCHES.STATE_NUMBER == 2){
            NEXT_LATCHES.AMSR = 29;
        }
        else if(CURRENT_LATCHES.STATE_NUMBER == 6){
            NEXT_LATCHES.AMSR = 25;
        }
        else if(CURRENT_LATCHES.STATE_NUMBER == 7){
            NEXT_LATCHES.AMSR = 23;
        }
        else if(CURRENT_LATCHES.STATE_NUMBER == 3){
            NEXT_LATCHES.AMSR = 24;
        }
        else if((CURRENT_LATCHES.STATE_NUMBER == 18) | (CURRENT_LATCHES.STATE_NUMBER == 19)){
            NEXT_LATCHES.AMSR = 33;
        }
    }
    /*LD_WSR*/
    if(GetLD_WSR(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
            NEXT_LATCHES.WSR = 1;
    }
    /*CL_WSR*/
    if(GetCL_WSR(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
            NEXT_LATCHES.WSR = 0;
    }

}

int shift_mask(int input, int shift_bits, int mask_bits){
    int temp = input >> shift_bits;
    temp &= mask_bits;
    return temp;
}

void printCurrentRowOfCS(){
    int temp = 0;
    printf("\nControl Store for State %d is : ", CURRENT_LATCHES.STATE_NUMBER);
    for(int i = 0; i < CONTROL_STORE_BITS; i++){
        temp = CURRENT_LATCHES.MICROINSTRUCTION[i];
        printf("%d", temp);
    }
    printf("\n");
}
