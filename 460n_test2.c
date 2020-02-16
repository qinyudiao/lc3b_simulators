//460n_test2.c

#include <stdio.h>
int main()
{
   // printf() displays the string inside quotation
   printf("Hello, World!");
   return 0;
}


// #include <stdio.h> /* standard input/output library */
// #include <stdlib.h> /* Standard C Library */
// #include <string.h> /* String operations library */
// #include <ctype.h> /* Library for useful character operations */
// #include <limits.h> /* Library for definitions of common variable type characteristics */

// #define MEMORY_SPACE 65536
// #define ADDRESSIBILITY 8
// #define WORD 16
// #define MAX_LINE_LENGTH 255
// #define MAX_LABEL_LENGTH 20
// #define MAX_LABEL_NUMBER 255
// #define OPCODE_NUMBER 28
// #define PSEUDO_OP_NUMBER 3
// #define OTHER_KEYWORD_NUMBER 4
// #define REGISTER_NUMBER 8

// const char * OPCODES[OPCODE_NUMBER] = {"ADD", "AND", "BR", "BRN", "BRZ", "BRP", "BRNZ",
//     "BRNP", "BRZP", "BRNZP", "HALT", "JMP", "JSR",
//     "JSRR", "LDB", "LDW", "LEA", "NOP", "NOT", "RET",
//     "LSHF", "RSHFL", "RSHFA", "RTI", "STB", "STW",
//     "TRAP", "XOR"};
// const char * PSEUDO_OPS[PSEUDO_OP_NUMBER] = {".ORIG", ".END", ".FILL"};
// const char * OTHER_KEYWORDS[OTHER_KEYWORD_NUMBER] = {"IN", "OUT", "GETC", "PUTS"};
// const char * REGISTERS[REGISTER_NUMBER] = {"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7"};

// /* Structures */
// struct TableEntry{
//     char label[MAX_LABEL_LENGTH+1];
//     int  address;
// };

// struct Instruction{
//     char *label;
//     char *opcode;
//     char *arg1;
//     char *arg2;
//     char *arg3;]
// };

// /* Global Variables */
// struct Instruction * instructions; /* an array of instructions of unknown size*/
// int instructionNum = 0;
// struct TableEntry symbolTable[MAX_LABEL_NUMBER];
// int tableSize = 0;

// /* Function Declarations */
// int power (int num, int power);
// char * toBinString(int dec, int bits, int isLabel); /* isLabel is 1 if if the dec passed in represent the offset in label calculation, 0 otherwise*/
// char * toBinStringNonNeg(int dec, int bits);
// char * binToHex(char * bin);
// int getTarget(char * label);
// int belongs(char *str, const char * list[], int num_of_elements);
// int isOpcode(char *str); /* return 1 if it's an opcode, 0 if it's an pseudo-op, and -1 for everything else*/
// void readAndParse(FILE * fin); /* fill out the instructions array*/
// int toNum(char * pStr); /* takes a number string, return the number*/
// int checkORIG(struct Instruction ins); /* returns the starting address if no error */
// void checkLabel(struct Instruction ins);
// void checkEnd(struct Instruction ins);
// char * translateRegister(char * reg);
// char * checkInstruction(struct Instruction ins, int address);
// void firstPass(void); /* construct the symbol table */
// char * secondPass();

// int main(int argc, char * argv[]) {
//     if (argc != 3) {
//         printf("%s\n", "Invalid command line arguments");
//         exit(4);
//     }

//     char *iFileName = argv[1];
//     char *oFileName = argv[2];

//     /* file i/o handling */
//     FILE * fin;
//     if ((fin = fopen(iFileName, "r")) == NULL) {
//         printf("%s %s\n", iFileName, "does not exist.");
//         exit(4);
//     }

//     readAndParse(fin);
//     // firstPass();
//     // char * mcHex = secondPass();
//     // FILE * fout = fopen(oFileName,"w");
//     // fprintf(fout, "%s\n", mcHex);
//     // free(mcHex);
//     fclose(fin);
//     fclose(fout);
// }
