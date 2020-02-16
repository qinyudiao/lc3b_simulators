
/*
 Assume there will not be any .ORIG in between a .ORIG and a .END
 */

#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

#define MEMORY_SPACE 65536
#define ADDRESSIBILITY 8
#define WORD 16
#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LENGTH 20
#define MAX_LABEL_NUMBER 255
#define OPCODE_NUMBER 28
#define PSEUDO_OP_NUMBER 3
#define OTHER_KEYWORD_NUMBER 4
#define REGISTER_NUMBER 8

const char * OPCODES[OPCODE_NUMBER] = {"ADD", "AND", "BR", "BRN", "BRZ", "BRP", "BRNZ",
    "BRNP", "BRZP", "BRNZP", "HALT", "JMP", "JSR",
    "JSRR", "LDB", "LDW", "LEA", "NOP", "NOT", "RET",
    "LSHF", "RSHFL", "RSHFA", "RTI", "STB", "STW",
    "TRAP", "XOR"};
const char * PSEUDO_OPS[PSEUDO_OP_NUMBER] = {".ORIG", ".END", ".FILL"};
const char * OTHER_KEYWORDS[OTHER_KEYWORD_NUMBER] = {"IN", "OUT", "GETC", "PUTS"};
const char * REGISTERS[REGISTER_NUMBER] = {"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7"};

/* Structures */
struct TableEntry{
    char label[MAX_LABEL_LENGTH+1];
    int  address;
};

struct Instruction{
    char *label;
    char *opcode;
    char *arg1;
    char *arg2;
    char *arg3;
};

/* Global Variables */
struct Instruction * instructions; /* an array of instructions of unknown size*/
int instructionNum = 0;
struct TableEntry symbolTable[MAX_LABEL_NUMBER];
int tableSize = 0;

/* Function Declarations */
int power (int num, int power);
char * toBinString(int dec, int bits, int isLabel); /* isLabel is 1 if if the dec passed in represent the offset in label calculation, 0 otherwise*/
char * toBinStringNonNeg(int dec, int bits);
char * binToHex(char * bin);
int getTarget(char * label);
int belongs(char *str, const char * list[], int num_of_elements);
int isOpcode(char *str); /* return 1 if it's an opcode, 0 if it's an pseudo-op, and -1 for everything else*/
void readAndParse(FILE * fin); /* fill out the instructions array*/
int toNum(char * pStr); /* takes a number string, return the number*/
int checkORIG(struct Instruction ins); /* returns the starting address if no error */
void checkLabel(struct Instruction ins);
void checkEnd(struct Instruction ins);
char * translateRegister(char * reg);
char * checkInstruction(struct Instruction ins, int address);
void firstPass(void); /* construct the symbol table */
char * secondPass();

int main(int argc, char * argv[]) {
    if (argc != 3) {
        printf("%s\n", "Invalid command line arguments, you idiot");
        printf("%d\n", argc);
        exit(4);
    }

    printf("%d\n", argc);

    char *iFileName = argv[1];
    char *oFileName = argv[2];

    /* file i/o handling */
    FILE * fin;
    if ((fin = fopen(iFileName, "r")) == NULL) {
        printf("%s %s\n", iFileName, "does not exist.");
        exit(4);
    }

    readAndParse(fin);
    // firstPass();
    // char * mcHex = secondPass();
    // FILE * fout = fopen(oFileName,"w");
    // fprintf(fout, "%s\n", mcHex);
    // free(mcHex);
    fclose(fin);
    //fclose(fout);
}

void readAndParse(FILE * file) {
    char line[MAX_LINE_LENGTH];
    /* first, count the number of lines, and allocate memory for instructions*/
    int numOfLines = 0;
    while (fgets(line, MAX_LINE_LENGTH+1, file) != NULL) //get the number of total lines
        numOfLines++;
    instructions = (struct Instruction *) malloc(numOfLines * sizeof(struct Instruction) +1);
    rewind(file);

    while (fgets(line, MAX_LINE_LENGTH+1, file) != NULL) {
      /* convert entire line to lowercase using toupper*/
      int i;
      for (i = 0; i < strlen(line); i++)
        line[i] = toupper(line[i]);

      /* get rid of the comments*/
      char * temp = line;
      while( *temp != ';' && *temp != '\0' && *temp != '\n') /* stops when hit a semicolon, end of string (never should), or newline*/
        temp++;
        *temp = '\0';

      /* split the line to tokens and store as instruction */
      char * token;
      if(!(token = strtok(line, "\t\n ,")))
      continue; /* empty line */

      /* Initialize everything in this instruction */
      instructions[instructionNum].label = NULL;
      instructions[instructionNum].opcode = NULL;
      instructions[instructionNum].arg1 = NULL;
      instructions[instructionNum].arg2 = NULL;
      instructions[instructionNum].arg3 = NULL;
      // instructions[instructionNum].arg4 = NULL;

      if(isOpcode(token) == -1) {/* found a label */
         instructions[instructionNum].label = (char *) malloc(strlen(token)+1);
         strcpy(instructions[instructionNum].label, token);
         if(!(token = strtok(NULL, "\t\n ,"))) {
            /* impossible that a line contains only a label*/
            /* that "label" is actually the invalid opcode */
            printf("Invalid opcode, %s\n",instructions[instructionNum].label);
            exit(2);
         }
      }
      instructions[instructionNum].opcode = (char *) malloc(strlen(token)+1);
      strcpy(instructions[instructionNum].opcode, token);

      if(!(token = strtok(NULL, "\t\n ,"))) {
         instructionNum++;
         continue;
      }
      instructions[instructionNum].arg1 = (char *) malloc(strlen(token)+1);
      strcpy(instructions[instructionNum].arg1, token);

      if(!(token = strtok(NULL, "\t\n ,"))) {
         instructionNum++;
         continue;
      }
      instructions[instructionNum].arg2 = (char *) malloc(strlen(token)+1);
      strcpy(instructions[instructionNum].arg2, token);

      if(!(token = strtok(NULL, "\t\n ,"))) {
         instructionNum++;
         continue;
      }
      instructions[instructionNum].arg3 = (char *) malloc(strlen(token)+1);
      strcpy(instructions[instructionNum].arg3, token);

      if(!(token = strtok(NULL, "\t\n ,"))) {
         instructionNum++;
         continue;
      }
      //instructions[instructionNum].arg4= (char *) malloc(strlen(token)+1);
      //strcpy(instructions[instructionNum].arg4, token);

      instructionNum++;

        /* This part is unnecessary because we included arg4, while arg4 should never appear in any of the instructions
         if there are more tokens left, then it must have too many operands
         if((token = strtok(NULL, "\t\n ,")) != NULL) {
         printf("Invalid number of operands\n");
         exit(4);
         }
         */
    }
}

int belongs(char *str, const char * list[], int num_of_elements) {
    int i;
    for (i=0; i<num_of_elements; i++) {
        if (strcmp(str, list[i]) == 0)
            return 1;
    }
    return 0;
}

int isOpcode(char *str){
    if (belongs(str, OPCODES, OPCODE_NUMBER)) return 1;
    if (belongs(str, PSEUDO_OPS, PSEUDO_OP_NUMBER)) return 0;
    return -1;
}

int checkORIG(struct Instruction ins) {
    /* labels above or in front of .ORIG are ignored */
    if (ins.arg1 == NULL || ins.arg2 != NULL) {
        printf("%s\n", "Invalid number of oprands for .ORIG");
        exit(4);
    }
    int address = toNum(ins.arg1);
    if (!(address >= 0 && address < MEMORY_SPACE && address % 2 == 0)) {
        printf("Invalid constant, %s.\n", ins.arg1);
        exit(3);
    }
    return address;
}

void checkLabel(struct Instruction ins) {
    /* label format */
    char * label = ins.label;
    if (strlen(label) > MAX_LABEL_LENGTH) {
        printf("Illegal label, %s.\n", label);
        exit(4);
    }
    /* if things go right, label will never be the same as opcodes or pseudo-ops */
    if (belongs(label, OTHER_KEYWORDS,OTHER_KEYWORD_NUMBER)) {
        printf("Illegal label, %s.\n", label);
        exit(4);
    }
    if (!(isalpha(label[0]) && label[0] !='X' && label[0] != 'x')) {
        printf("Illegal label, %s.\n", label);
        exit(4);
    }
    int i;
    for (i=1; i<strlen(label); i++) {
        if (!isalnum(label[i])) {
            printf("Illegal label, %s.\n", label);
            exit(4);
        }
    }
}


void checkEnd(struct Instruction ins) {
    if (ins.arg1 != NULL) {
        printf("%s\n", "Invalid number of oprands for .END");
        exit(4);
    }
}

int toNum(char * pStr) {
    char * t_ptr;
    char * orig_pStr;
    int t_length,k;
    int lNum, lNeg = 0;
    long int lNumLong;

    orig_pStr = pStr;
    if( *pStr == '#' )				/* decimal */
    {
        pStr++;
        if( *pStr == '-' )				/* dec is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++)
        {
            if (!isdigit(*t_ptr))
            {
                printf("Error: invalid decimal operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg)
            lNum = -lNum;

        return lNum;
    }
    else if(*pStr == 'X' || *pStr == 'x')	/* hex     */
    {
        pStr++;
        if( *pStr == '-' )				/* hex is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++)
        {
            if (!isxdigit(*t_ptr))
            {
                printf("Error: invalid hex operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
        lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong; /* if the hex number is too big, just return the maximum int? */
        if( lNeg )
            lNum = -lNum;
        return lNum;
    }
    else
    {
        printf( "Error: invalid operand, %s\n", orig_pStr);
        exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}
