/*
    Remove all unnecessary lines (including this one) 
    in this comment.
    REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

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
void ADD();
void AND();
void BR();
void JMP();
void JSR();
void LDB();
void LDW();
void LEA();
void SHF();
void STB();
void STW();
void TRAP();
void XOR();
void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */
   int inst = (MEMORY[(CURRENT_LATCHES.PC)/2][1] << 8) + MEMORY[(CURRENT_LATCHES.PC)/2][0];
   int opcode = MEMORY[(CURRENT_LATCHES.PC)/2][1] >> 4;
   switch(opcode){
    case 0001: ADD(inst);
      break;
    case 0101: AND(inst);
      break;
    case 0000: BR(inst);
      break;
    case 1100: JMP(inst);
      break;
    case 0100: JSR(inst);
      break;
    case 0010: LDB(inst);
      break;
    case 0110: LDW(inst);
      break;
    case 1110: LEA(inst);
      break;
    case 1101: SHF(inst);
      break;
    case 0011: STB(inst);
      break;
    case 0111: STW(inst);
      break;
    case 1111: TRAP(inst);
      break;
    case 1001: XOR(inst);
      break;
  }
}

void ADD(int inst){
  int bit5 = (inst & 0x0020) >> 5;
  int DR = (inst & 0x0E00) >> 9;
  int SR1 = (inst & 0x01C0) >> 6;
  if(bit5 == 0){
    int SR2 = inst & 0x0007;
    NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] + CURRENT_LATCHES.REGS[SR2]);
  }
  else if(bit5 == 1){
    int imm5 = inst & 0x001F;
    int bit4 = (inst & 0x0010) >> 4;
    if(bit4 == 1){
      imm5 += 0xFFE0;
    }
    NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] + Low16bits(imm5));
  }
  if(NEXT_LATCHES.REGS[DR] > 0){
    NEXT_LATCHES.N = Low16bits(0);
    NEXT_LATCHES.Z = Low16bits(0);
    NEXT_LATCHES.P = Low16bits(1);
  }
  else if(NEXT_LATCHES.REGS[DR] < 0){
    NEXT_LATCHES.N = Low16bits(1);
    NEXT_LATCHES.Z = Low16bits(0);
    NEXT_LATCHES.P = Low16bits(0);
  }
  else{
    NEXT_LATCHES.N = Low16bits(0);
    NEXT_LATCHES.Z = Low16bits(1);
    NEXT_LATCHES.P = Low16bits(0);
  }

  NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);
}

void AND(int inst){
  int bit5 = (inst & 0x0020) >> 5;
  int DR = (inst & 0x0E00) >> 9;
  int SR1 = (inst & 0x01C0) >> 6;
  if(bit5 == 0){
    int SR2 = inst & 0x0007;
    NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] & CURRENT_LATCHES.REGS[SR2]);
  }
  else if(bit5 == 1){
    int imm5 = inst & 0x001F;
    int bit4 = (inst & 0x0010) >> 4;
    if(bit4 == 1){
      imm5 += 0xFFE0;
    }
    NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] & Low16bits(imm5));
  }
  if(NEXT_LATCHES.REGS[DR] > 0){
    NEXT_LATCHES.N = Low16bits(0);
    NEXT_LATCHES.Z = Low16bits(0);
    NEXT_LATCHES.P = Low16bits(1);
  }
  else if(NEXT_LATCHES.REGS[DR] < 0){
    NEXT_LATCHES.N = Low16bits(1);
    NEXT_LATCHES.Z = Low16bits(0);
    NEXT_LATCHES.P = Low16bits(0);
  }
  else{
    NEXT_LATCHES.N = Low16bits(0);
    NEXT_LATCHES.Z = Low16bits(1);
    NEXT_LATCHES.P = Low16bits(0);
  }

   NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);
}

void BR(int inst){
  int bitn = (inst & 0x0800) >> 11;
  int bitz = (inst & 0x0400) >> 10;
  int bitp = (inst & 0x0200) >> 9;
  int PCoffset9 = inst & 0x01FF;
  int bit8 = (inst & 0x0100) >> 8;
  if(bit8 == 1){
      PCoffset9 += 0xFE00;
  }
  PCoffset9 = PCoffset9 << 1;

  if((bitn & CURRENT_LATCHES.N) | (bitz & CURRENT_LATCHES.Z) | (bitn & CURRENT_LATCHES.P)){
    NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + Low16bits(PCoffset9) + 2); 
  }
}

void JMP(int inst){
  int BaseR = (inst & 0x01C0) >> 6;
  NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[BaseR]);
}

void JSR(int inst){
  NEXT_LATCHES.REGS[7] = Low16bits(CURRENT_LATCHES.PC + 2);

  int bit11 = (inst & 0x0800) >> 11;
  if(bit11 == 0){
    int BaseR = (inst & 0x01C0) >> 6;
    NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[BaseR]);
  }
  else if(bit11 == 1){
    int PCoffset11 = inst & 0x07FF;
    int bit10 = (inst & 0x0400) >> 10;
    if(bit10 == 1){
      PCoffset11 += 0xF800;
    }
    PCoffset11 = PCoffset11 << 1;
    NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + Low16bits(PCoffset11) + 2);
  }
}

void LDB(int inst){
  int DR = (inst & 0x0E00) >> 9;
  int BaseR = (inst & 0x01C0) >> 6;
  int boffset6 =  inst & 0x003F;
  int bit6 = (inst & 0x0040) >> 6;
  if(bit6 == 1){
    boffset6 += 0xFF80;
  }
  int content = MEMORY[Low16bits(BaseR + boffset6)/2][Low16bits(BaseR + boffset6) % 2];
  int content7 = (inst & 0x0080) >> 7;
  if(content7 == 1){
    content |= 0xFF00;  
  }
  NEXT_LATCHES.REGS[DR] = Low16bits(content);

  /*  setcc and set PC  */
  NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);

  if(NEXT_LATCHES.REGS[DR] > 0){
    NEXT_LATCHES.N = Low16bits(0);
    NEXT_LATCHES.Z = Low16bits(0);
    NEXT_LATCHES.P = Low16bits(1);
  }
  else if(NEXT_LATCHES.REGS[DR] < 0){
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

void LDW(int inst){
  int DR = (inst & 0x0E00) >> 9;
  int BaseR = (inst & 0x01C0) >> 6;
  int boffset6 =  inst & 0x003F;
  int bit6 = (inst & 0x0040) >> 6;
  if(bit6 == 1){
    boffset6 += 0xFF80;
  }
  int content0 = MEMORY[Low16bits(BaseR + boffset6)/2][0];
  int content1 = MEMORY[Low16bits(BaseR + boffset6)/2][1];
  int content = (MEMORY[Low16bits(BaseR + boffset6)/2][1] << 8) + MEMORY[Low16bits(BaseR + boffset6)/2][0];;
  NEXT_LATCHES.REGS[DR] = Low16bits(content);

  /*  setcc and set PC  */
  NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);

  if(NEXT_LATCHES.REGS[DR] > 0){
    NEXT_LATCHES.N = Low16bits(0);
    NEXT_LATCHES.Z = Low16bits(0);
    NEXT_LATCHES.P = Low16bits(1);
  }
  else if(NEXT_LATCHES.REGS[DR] < 0){
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

void LEA(int inst){
  int DR = (inst & 0x0E00) >> 9;
  int PCoffset9 = inst & 0x01FF;
  int bit8 = (inst & 0x0100) >> 8;
  if(bit8 == 1){
      PCoffset9 += 0xFE00;
  }
  PCoffset9 = PCoffset9 << 1;
  NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);
  NEXT_LATCHES.REGS[DR] = Low16bits(NEXT_LATCHES.PC + PCoffset9);
}

void SHF(int inst){
  int DR = (inst & 0x0E00) >> 9;
  int SR = (inst & 0x01C0) >> 6;
  int bit5 = (inst & 0x0020) >> 5;
  int bit4 = (inst & 0x0010) >> 4;
  int amount4 = inst & 0x000F;
  if(bit4 == 0){
    NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR] << (amount4));
  }
  else{
    if(bit5 == 0){
      NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR] >> (amount4));
    }
    else if(bit5 == 1){
      int i;
      int tempSR;
      for(i = 0; i < amount4; i++){
        int bit15 = inst & 0x8000;
        tempSR = (tempSR >> 1) + bit15;
      }
      NEXT_LATCHES.REGS[DR] = Low16bits(tempSR);
    }
  }

  if(NEXT_LATCHES.REGS[DR] > 0){
    NEXT_LATCHES.N = Low16bits(0);
    NEXT_LATCHES.Z = Low16bits(0);
    NEXT_LATCHES.P = Low16bits(1);
  }
  else if(NEXT_LATCHES.REGS[DR] < 0){
    NEXT_LATCHES.N = Low16bits(1);
    NEXT_LATCHES.Z = Low16bits(0);
    NEXT_LATCHES.P = Low16bits(0);
  }
  else{
    NEXT_LATCHES.N = Low16bits(0);
    NEXT_LATCHES.Z = Low16bits(1);
    NEXT_LATCHES.P = Low16bits(0);
  }
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
}

void STB(int inst){
  int SR = (inst & 0x0E00) >> 9;
  int BaseR = (inst & 0x01C0) >> 6;
  int boffset6 =  inst & 0x003F;
  int bit6 = (inst & 0x0040) >> 6;
  if(bit6 == 1){
    boffset6 += 0xFF80;
  }
  MEMORY[Low16bits(BaseR + boffset6)/2][Low16bits(BaseR + boffset6) % 2] = Low16bits(CURRENT_LATCHES.REGS[SR]);
}

void STW(int inst){
  int SR = (inst & 0x0E00) >> 9;
  int BaseR = (inst & 0x01C0) >> 6;
  int boffset6 = inst & 0x003F;
  int content0 = CURRENT_LATCHES.REGS[SR] & 0x00FF;
  int content1 = (CURRENT_LATCHES.REGS[SR] >> 8) & 0x00FF;
  int bit6 = (inst & 0x0040) >> 6;
  if(bit6 == 1){
    boffset6 += 0xFF80;
  }
  MEMORY[Low16bits(BaseR + boffset6)/2][0] = content0;
  MEMORY[Low16bits(BaseR + boffset6)/2][1] = content1;
}

void TRAP(int inst){
  NEXT_LATCHES.REGS[7] = Low16bits(CURRENT_LATCHES.PC + 2);
  int trapvect8 = inst & 0x00FF;
  trapvect8 = trapvect8 << 1;
  NEXT_LATCHES.PC = Low16bits(MEMORY[Low16bits(trapvect8/2)][Low16bits(trapvect8 % 2)]);
}

void XOR(int inst){
  int bit5 = (inst & 0x0020) >> 5;
  int DR = (inst & 0x0E00) >> 9;
  int SR = (inst & 0x01C0) >> 6;
  
  if(bit5 == 0){
    int SR2 = inst & 0x0007;
    NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR] ^ CURRENT_LATCHES.REGS[SR2]);
  }
  else if(bit5 == 1){
    int imm5 = inst & 0x001F;
    int bit4 = (inst & 0x0010) >> 4;
    if(bit4 == 1){
      imm5 += 0xFFE0;
    }
    NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR] ^ imm5);
  }

  if(NEXT_LATCHES.REGS[DR] > 0){
    NEXT_LATCHES.N = Low16bits(0);
    NEXT_LATCHES.Z = Low16bits(0);
    NEXT_LATCHES.P = Low16bits(1);
  }
  else if(NEXT_LATCHES.REGS[DR] < 0){
    NEXT_LATCHES.N = Low16bits(1);
    NEXT_LATCHES.Z = Low16bits(0);
    NEXT_LATCHES.P = Low16bits(0);
  }
  else{
    NEXT_LATCHES.N = Low16bits(0);
    NEXT_LATCHES.Z = Low16bits(1);
    NEXT_LATCHES.P = Low16bits(0);
  }
  NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);
}
