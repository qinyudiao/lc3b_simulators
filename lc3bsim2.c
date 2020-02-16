/*
    Name 1: Qinyu Diao
    UTEID 1: qd572
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

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
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
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
/* Purpose   : Simulate the LC-3b until HALTed                 */
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

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
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
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
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
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

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
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
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
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/
/* Global Functions                                           */
/***************************************************************/
void ADD();     // opcode : 0001   1
void AND();     // opcode : 0101   5
void BR();      // opcode : 0000   0 
void JMP();     // opcode : 1100   C
void JSR();     // opcode : 0100   4
void LDB();     // opcode : 0010   2
void LDW();     // opcode : 0110   6
void LEA();     // opcode : 1110   E
//void RTI();     // opcode : 0000   0  does not need for this lab
void SHF();     // opcode : 1101   D
void STB();     // opcode : 0011   3
void STW();     // opcode : 0111   7
void TRAP();    // opcode : 1111   F
void XOR();     // opcode : 1001   9

void setcc();
void setpc();

/***************************************************************/
/*                                                             */
/* Global Variables                                            */
/*                                                             */
/***************************************************************/

int inst;
int opcode;
int DR, SR1, SR2, BaseR, imm5, bit5, bit4, bit7, bit8, bit11, bit10, bit9, PCoffset9, PCoffset11, boffset6, offset6, amount4;

/*************************************************************************************/
/* AND bits of operands                                                              */
/*  DR              0x0E00 bit 11-9             9   DR = (inst & 0x0E00) >> 9;
    SR1             0x01C0 bit 8-6              6   SR1 = (inst & 0x01C0) >> 6;
    SR2             0x0007 bit 2-0                  SR2 = inst & 0x0007;
    BaseR           0x01C0 bit 8-6              6   BaseR = (inst & 0x01C0) >> 6;
    imm5            0x001F bit 4-0                  imm5 = inst & 0x001F;
    bit5            0x0020 bit 5                5   bit5 = (inst & 0x0020) >> 5;
    bit4            0x0010 bit 4                4   bit4 = (inst & 0x0010) >> 4;
    bit7            0x0080 bit 7                7   bit7 = (inst & 0x0080) >> 7;
    bit8            0x0100 bit 8                8   bit8 = (inst & 0x0100) >> 8;
    bit11           0x0800 bit 11  // n bit     11  bit11 = (inst & 0x0800) >> 11;
    bit10           0x0400 bit 10  // z bit     10  bit10 = (inst & 0x0400) >> 10;
    bit9            0x0200 bit 9   // p bit     9   bit9 = (inst & 0x0200) >> 9;
    PCoffset9       0x01FF bit 8-0                  PCoffset9 = inst & 0x01FF;
    PCoffset11      0x07FF bit 10-0                 PCoffset11 = inst & 0x07FF;
    boffset6        0x003F bit 5-0                  boffset6 =  inst & 0x003F;
    offset6         0x003F bit 5-0                  offset6 =  inst & 0x003F;
    amount4         0x000F bit 3-0                  amount4 = inst & 0x000F;
*/
/*************************************************************************************/

void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */     
   inst = (MEMORY[(CURRENT_LATCHES.PC)/2][1] << 8) + MEMORY[(CURRENT_LATCHES.PC)/2][0];
   opcode = (MEMORY[(CURRENT_LATCHES.PC)/2][1] >> 4) & 0x000F;
   switch(opcode){
    case 1: ADD(inst);
      //
      break;
    case 5: AND(inst);
      //
      break;
    case 0: BR(inst);
      //printf("BR \n");
      break;
    case 12: JMP(inst);
      //printf("JMP \n");
      break;
    case 4: JSR(inst);
      //printf("JSR \n");
      break;
    case 2: LDB(inst);
      //
      break;
    case 6: LDW(inst);
      //
      break;
    case 14: LEA(inst);
      //
      break;
    case 13: SHF(inst);
      //
      break;
    case 3: STB(inst);
      //
      break;
    case 7: STW(inst);
      //
      break;
    case 15: TRAP(inst);
      //printf("HALT \n");
      break;
    case 9: XOR(inst);
      //
      break;
  }
}

void ADD(int inst){
    bit5 = (inst & 0x0020) >> 5;      // 0x0020 bit 5
    DR = (inst & 0x0E00) >> 9;        // 0x0E00 bit 11-9
    SR1 = (inst & 0x01C0) >> 6;       // 0x01C0 bit 8-6

    if(bit5 == 0){
        SR2 = inst & 0x0007;            // 0x0007 bit 2-0
        NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] + CURRENT_LATCHES.REGS[SR2]);
    }
    else{
        imm5 = inst & 0x001F;           // 0x001F bit 4-0
        bit4 = (inst & 0x0010) >> 4;    // 0x0010 bit 4
        if(bit4 == 1){  // if negative
            imm5 += 0xFFE0;               
        }
        NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] + Low16bits(imm5));
    }

    setcc();
    setpc();  //PC increment 2
}

void AND(int inst){
    bit5 = (inst & 0x0020) >> 5;      // 0x0020 bit 5
    DR = (inst & 0x0E00) >> 9;        // 0x0E00 bit 11-9
    SR1 = (inst & 0x01C0) >> 6;       // 0x01C0 bit 8-6

    if(bit5 == 0){
        SR2 = inst & 0x0007;            // 0x0007 bit 2-0
        NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] & CURRENT_LATCHES.REGS[SR2]);
    }
    else if(bit5 == 1){
        imm5 = inst & 0x001F;           // 0x001F bit 4-0
        bit4 = (inst & 0x0010) >> 4;    // 0x0010 bit 4
        if(bit4 == 1){  // if negative
            imm5 += 0xFFE0;               
        }
        NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] & Low16bits(imm5));
    }

    setcc();

    setpc();  //PC increment 2
}

void BR(){
    bit11 = (inst & 0x0800) >> 11;    // n
    bit10 = (inst & 0x0400) >> 10;    // z
    bit9 = (inst & 0x0200) >> 9;      // p
    PCoffset9 = inst & 0x01FF;

    int isBranch = 0;
    if(bit11 == 1 && CURRENT_LATCHES.N == 1)
        isBranch = 1;
    else if(bit10 == 1 && CURRENT_LATCHES.Z == 1)
        isBranch = 1;
    else if(bit9 == 1 && CURRENT_LATCHES.P == 1)
        isBranch = 1;

    bit8 = (inst & 0x0100) >> 8;  // check +/- of PCoffset9 
    if(bit8 == 1){
      PCoffset9 += 0xFE00;
      PCoffset9 -= 65536;
    }
    PCoffset9 = PCoffset9 << 1;

    if(isBranch)
        NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2 + Low16bits(PCoffset9));   //PC = PC† + LSHF(SEXT(PCoffset9),1);
    else
        setpc();
}

void JMP(){
    BaseR = (inst & 0x01C0) >> 6;

    NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[BaseR]);
}

/*
	TEMP = PC†;
	if (bit(11)==0)
	    PC = BaseR;
	else
	    PC = PC† + LSHF(SEXT(PCoffset11), 1);
	R7 = TEMP;
*/
void JSR(){
    int temp_pc = Low16bits(CURRENT_LATCHES.PC + 2);
    //printf("temp_pc : %d \n", temp_pc);
    bit11 = (inst & 0x0800) >> 11;
    if(bit11 == 0){ //JSRR
        BaseR = (inst & 0x01C0) >> 6;
        //if((CURRENT_LATCHES.REGS[BaseR] % 2) == 0)
        NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[BaseR]);
    }
    else{   //JSR
        PCoffset11 = inst & 0x07FF;
        bit10 = (inst & 0x0400) >> 10;
        if(bit10 == 1){
            PCoffset11 += 0xF800;
            PCoffset11 -= 65536;
        }
        PCoffset11 = PCoffset11 << 1;
        NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2 + Low16bits(PCoffset11));
    }
    NEXT_LATCHES.REGS[7] = Low16bits(temp_pc);
    //printf("R7 : %d \n", CURRENT_LATCHES.REGS[7]);
}

void LDB(){
    DR = (inst & 0x0E00) >> 9;
    BaseR = (inst & 0x01C0) >> 6;
    boffset6 =  inst & 0x003F;
/*   
    printf("boffset6 : %d \n", boffset6);
    printf("BaseR : %d \n", BaseR);
    printf("DR : %d \n", DR);
*/
/*******************NEED VERIFICATION************************/
    bit5 = (inst & 0x0020) >> 5;
    if(bit5 == 1){  //check negative
        boffset6 += 0xFFC0;
        boffset6 -= 65536;
    }

    int content = MEMORY[(CURRENT_LATCHES.REGS[BaseR] + boffset6)/2][Low16bits(boffset6) % 2];

    //printf("content : %d \n", content);
    bit7 = (content & 0x0080) >> 7;
    if(bit7 == 1){  // SEXT
        content |= 0xFF00;
        boffset6 -= 65536; 
    }
    else content &= 0x00FF;

    NEXT_LATCHES.REGS[DR] = Low16bits(content); // DR = mem;

    setcc();
    setpc();
}

void LDW(){
    DR = (inst & 0x0E00) >> 9;
    BaseR = (inst & 0x01C0) >> 6;
    offset6 =  inst & 0x003F;

    bit5 = (inst & 0x0020) >> 5;
    if(bit5 == 1){  //check negative
        offset6 += 0xFFC0;
        offset6 -= 65536;
    }
    offset6 = offset6 << 1;
    //printf("offset6 : %d \n", offset6);
    int content0 = MEMORY[(CURRENT_LATCHES.REGS[BaseR] + offset6)/2][0];
    int content1 = MEMORY[(CURRENT_LATCHES.REGS[BaseR] + offset6)/2][1] << 8;
    int content = Low16bits(content0) + Low16bits(content1);

    NEXT_LATCHES.REGS[DR] = Low16bits(content); // DR = mem;

    setcc();
    setpc();
}

void LEA(int inst){
    DR = (inst & 0x0E00) >> 9;
    PCoffset9 = inst & 0x01FF;
    bit8 = (inst & 0x0100) >> 8;
    if(bit8 == 1){
        PCoffset9 += 0xFE00;
        PCoffset9 -= 65536;
    }

    NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.PC + 2 + Low16bits(PCoffset9 << 1));

    setpc();
}

void SHF(int inst){
    DR = (inst & 0x0E00) >> 9;
    int SR = (inst & 0x01C0) >> 6;
    amount4 = inst & 0x000F;

    bit4 = (inst & 0x0010) >> 4;
    if(bit4 == 0)   //LSHF
        NEXT_LATCHES.REGS[DR] = Low16bits(Low16bits(CURRENT_LATCHES.REGS[SR]) << Low16bits(amount4));
    else{
        bit5 = (inst & 0x0020) >> 5;
        if(bit5 == 0){  //RSHFL
            NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR] >> Low16bits(amount4));
        }
        else{           //RSHFA
            int sign_bit_15 = CURRENT_LATCHES.REGS[SR] >> 15;
            int temp_shift = Low16bits(CURRENT_LATCHES.REGS[SR]);
            if(sign_bit_15 == 1){
                for(int i = amount4; i > 0; i--)
                    temp_shift = Low16bits((temp_shift >> 1) + 0x8000);
            NEXT_LATCHES.REGS[DR] = Low16bits(temp_shift);
            }else NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR] >> Low16bits(amount4));
        }
    }

    setcc();
    setpc();
}

void STB(int inst){
    int SR = (inst & 0x0E00) >> 9;
    int temp_mem = Low16bits(CURRENT_LATCHES.REGS[SR]) & 0x00FF;
    BaseR = (inst & 0x01C0) >> 6;
    boffset6 = inst & 0x003F;

    bit5 = (inst & 0x0020) >> 5;
    if(bit5 == 1){
        boffset6 += 0xFFC0;  // SEXT
        boffset6 -= 65536;
    }

    MEMORY[Low16bits(CURRENT_LATCHES.REGS[BaseR] + boffset6)/2][Low16bits(CURRENT_LATCHES.REGS[BaseR] + boffset6) % 2] = Low16bits(temp_mem);
    
    setpc();
}

void STW(int inst){
    int SR = (inst & 0x0E00) >> 9;
    int temp_mem_0 = CURRENT_LATCHES.REGS[SR] & 0x00FF;
    int temp_mem_1 = (CURRENT_LATCHES.REGS[SR] >> 8) & 0x00FF;
    BaseR = (inst & 0x01C0) >> 6;
    offset6 = inst & 0x003F;
    //printf("offset6 : %d \n", offset6);

    bit5 = (inst & 0x0020) >> 5;
    if(bit5 == 1){
        offset6 += 0xFFC0;  // SEXT
        //printf("offset6_0 : %d \n", offset6);
        offset6 -= 65536;
        //printf("offset6_1 : %d \n", offset6);
    }

    offset6 = offset6 << 1;
    //printf("offset6_2 : %d \n", offset6);

    MEMORY[Low16bits(CURRENT_LATCHES.REGS[BaseR]+ offset6)/2][0] = Low16bits(temp_mem_0);
    MEMORY[Low16bits(CURRENT_LATCHES.REGS[BaseR] + offset6)/2][1] = Low16bits(temp_mem_1);

    setpc();
}

void TRAP(int inst){
    NEXT_LATCHES.REGS[7] = Low16bits(CURRENT_LATCHES.PC + 2);
    NEXT_LATCHES.PC = 0;
}

void XOR(int inst){
    bit5 = (inst & 0x0020) >> 5;
    DR = (inst & 0x0E00) >> 9;
    SR1 = (inst & 0x01C0) >> 6;

    if(bit5 == 0){
        SR2 = inst & 0x0007;
        NEXT_LATCHES.REGS[DR] = Low16bits(Low16bits(NEXT_LATCHES.REGS[SR1]) ^ Low16bits(NEXT_LATCHES.REGS[SR2]));
    }

    else{
        imm5 = inst & 0x001F;
        bit4 = (inst & 0x0010) >> 4;
        if(bit4 == 1){  // if negative
            imm5 += 0xFFE0;               
        }
        NEXT_LATCHES.REGS[DR] = Low16bits(Low16bits(CURRENT_LATCHES.REGS[SR1]) ^ Low16bits(imm5));
    }

    setcc();
    setpc();
}

void setcc(){
    //printf("content[DR] : %d \n", Low16bits(NEXT_LATCHES.REGS[DR]));
    int temp = Low16bits(NEXT_LATCHES.REGS[DR]);
    if (temp > 32767)   // convert unsigned to signed
        temp = -1;
    if(temp > 0){
        NEXT_LATCHES.N = Low16bits(0);
        NEXT_LATCHES.Z = Low16bits(0);
        NEXT_LATCHES.P = Low16bits(1);
    }
    else if(temp < 0){
        NEXT_LATCHES.N = Low16bits(1);
        NEXT_LATCHES.Z = Low16bits(0);
        NEXT_LATCHES.P = Low16bits(0);
    }
    else{
        NEXT_LATCHES.N = Low16bits(0);
        NEXT_LATCHES.Z = Low16bits(1);
        NEXT_LATCHES.P = Low16bits(0);
    }
}

/* PC++ */
void setpc(){
    NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);
}