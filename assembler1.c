/*
 Assume there will be only one .ORIG and one .END
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
    char *arg4;
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
        printf("%s\n", "Invalid command line, the number of arguments should be 3");
        printf("%s%d\n", "The number of arguments you input is: ", argc);
        exit(4);
    }

    char *iFileName = argv[1];
    char *oFileName = argv[2];
    /* print out the three arguments of the main*/
    //char *prgName   = argv[0];   
    // printf("program name = '%s'\n", prgName);
    // printf("input file name = '%s'\n", iFileName);
    // printf("output file name = '%s'\n", oFileName);

    /* file i/o handling */
    FILE * fin;
    if ((fin = fopen(iFileName, "r")) == NULL) {
        printf("%s %s\n", iFileName, "does not exist.");
        exit(4);
    }

    readAndParse(fin);
    firstPass();
    char * mcHex = secondPass();
    FILE * fout = fopen(oFileName,"w");
    fprintf(fout, "%s\n", mcHex);
    free(mcHex);
    fclose(fin);
    fclose(fout);
}

/* works only if there is only one .ORIG in the program */
void firstPass(){
    int lc = -1;    /*lc is the location of .ORIG which is where the program starts at*/
    int i;
    for (i=0; i<instructionNum; i++) {
        if (strcmp(instructions[i].opcode,".ORIG") == 0) {
            lc = checkORIG(instructions[i]);
            break;
        }
    }
    /* if no .ORIG found */
    if (lc == -1) {
        printf("No .ORIG found.\n");
        exit(4);
    }
    /* construct symbol table */
    for (i=i+1; i<instructionNum; i++){
      if (lc > MEMORY_SPACE) {
          printf("Program size exceeds memory space\n");
          exit(4);
      }
        /* this assumes no label is on the same line as .END */
        if (strcmp(instructions[i].opcode, ".END")==0) {
            checkEnd(instructions[i]);
            break;
        }
        if (instructions[i].label != NULL) {
            checkLabel(instructions[i]);
            /* check if the label is already used */
            int j;
            for (j=0; j<tableSize;j++) {
                if (strcmp(instructions[i].label, symbolTable[j].label)==0) {
                    printf("Label %s already used\n", instructions[i].label);
                    exit(4);
                }
            }
            strcpy(symbolTable[tableSize].label,instructions[i].label);
            symbolTable[tableSize].address = lc;
            tableSize++;
        }
        lc += 2;
    }
    /* check if .END is used in the program. NOT SURE if this is necessary */
    if (i == instructionNum) {
        printf("No .END found\n");
        exit(4);
    }
}

char * secondPass(){
    int lc;
    int i;
    char * mcHex = (char *) malloc(7*instructionNum+1); /* 7 chars each line: 0xXXXX\n*/
    for (i=0; i<instructionNum; i++) {
        if (strcmp(instructions[i].opcode,".ORIG") == 0) {
            lc = checkORIG(instructions[i]);
            char * bin = toBinStringNonNeg(lc, 16);
            char * hex = binToHex(bin);

            strcat(mcHex, hex);
            strcat(mcHex, "\n");
            free(bin);
            free(hex);
            break;
        }
    }
    for (i=i+1; i<instructionNum; i++){
        if (strcmp(instructions[i].opcode, ".END")==0) {
            break;
        }
        char * bin = checkInstruction(instructions[i], lc);
        char * hex = binToHex(bin);
        strcat(mcHex, hex);
        strcat(mcHex, "\n");
        free(bin);
        free(hex);
        lc += 2;
    }
    return mcHex;
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
      instructions[instructionNum].arg4 = NULL;

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
      instructions[instructionNum].arg4= (char *) malloc(strlen(token)+1);
      strcpy(instructions[instructionNum].arg4, token);

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

/* compares if input string is opcode, label, or peseudo_op */ 
/* by calling isOpcode, checkOrig, checkLabel, checkEnd */
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


char * checkInstruction(struct Instruction ins, int address) {
    char *opcode = ins.opcode;
    char *arg1 = ins.arg1;
    char *arg2 = ins.arg2;
    char *arg3 = ins.arg3;
    char *arg4 = ins.arg4;
    char * mc = malloc(16+1);

    if (!strcmp(opcode,"ADD")) {
        if (arg3 == NULL || arg4 != NULL) {
            printf("%s\n", "Invalid number of operands for ADD");
            exit(4);
        }
        strcat(mc, "0001");
        strcat(mc, translateRegister(arg1));
        strcat(mc, translateRegister(arg2));
        if (belongs(arg3,REGISTERS,REGISTER_NUMBER)){
            strcat(mc, "000");
            strcat(mc, translateRegister(arg3));
        } else {
            int imm5 = toNum(arg3);
            strcat(mc, "1");
            char * bin = toBinString(imm5, 5, 0);
            strcat(mc, bin);
        }
        return mc;
    }
    if (!strcmp(opcode,"AND")) {
        if (arg3 == NULL || arg4 != NULL) {
            printf("%s\n", "Invalid number of operands for AND");
            exit(4);
        }
        strcat(mc, "0101");
        strcat(mc, translateRegister(arg1));
        strcat(mc, translateRegister(arg2));
        if (belongs(arg3,REGISTERS,REGISTER_NUMBER)){
            strcat(mc, "000");
            strcat(mc, translateRegister(arg3));
        } else {
            int imm5 = toNum(arg3);
            strcat(mc, "1");
            char * bin = toBinString(imm5, 5, 0);
            strcat(mc, bin);
        }
        return mc;
    }
    if (!strcmp(opcode,"BR")) {
        if (arg1 == NULL || arg2 != NULL) {
            printf("%s\n", "Invalid number of operands for BR");
            exit(4);
        }
        strcat(mc, "0000111");
        /* check if the label is in the symbol table */
        int target = getTarget(arg1);
        int offsetShift = target - (address + 2); /* this should be even */
        int pcoffset9 = offsetShift/2;

        char * bin = toBinString(pcoffset9, 9, 1);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"BRN")) {
        if (arg1 == NULL || arg2 != NULL) {
            printf("%s\n", "Invalid number of operands for BRn");
            exit(4);
        }
        strcat(mc, "0000100");
        /* check if the label is in the symbol table */
        int target = getTarget(arg1);
        int offsetShift = target - (address + 2); /* this should be even */
        int pcoffset9 = offsetShift/2;
        char * bin = toBinString(pcoffset9, 9, 1);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"BRZ")) {
        if (arg1 == NULL || arg2 != NULL) {
            printf("%s\n", "Invalid number of operands for BRz");
            exit(4);
        }
        strcat(mc, "0000010");
        /* check if the label is in the symbol table */
        int target = getTarget(arg1);
        int offsetShift = target - (address + 2); /* this should be even */
        int pcoffset9 = offsetShift/2;
        char * bin = toBinString(pcoffset9, 9, 1);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"BRP")) {
        if (arg1 == NULL || arg2 != NULL) {
            printf("%s\n", "Invalid number of operands for BRp");
            exit(4);
        }
        strcat(mc, "0000001");
        /* check if the label is in the symbol table */
        int target = getTarget(arg1);
        int offsetShift = target - (address + 2); /* this should be even */
        int pcoffset9 = offsetShift/2;
        char * bin = toBinString(pcoffset9, 9, 1);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"BRNZ")) {
        if (arg1 == NULL || arg2 != NULL) {
            printf("%s\n", "Invalid number of operands for BRnz");
            exit(4);
        }
        strcat(mc, "0000110");
        /* check if the label is in the symbol table */
        int target = getTarget(arg1);
        int offsetShift = target - (address + 2); /* this should be even */
        int pcoffset9 = offsetShift/2;
        char * bin = toBinString(pcoffset9, 9, 1);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"BRNP")) {
        if (arg1 == NULL || arg2 != NULL) {
            printf("%s\n", "Invalid number of operands for BRnp");
            exit(4);
        }
        strcat(mc, "0000101");
        /* check if the label is in the symbol table */
        int target = getTarget(arg1);
        int offsetShift = target - (address + 2); /* this should be even */
        int pcoffset9 = offsetShift/2;
        char * bin = toBinString(pcoffset9, 9, 1);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"BRZP")) {
        if (arg1 == NULL || arg2 != NULL) {
            printf("%s\n", "Invalid number of operands for BRzp");
            exit(4);
        }
        strcat(mc, "0000011");
        /* check if the label is in the symbol table */
        int target = getTarget(arg1);
        int offsetShift = target - (address + 2); /* this should be even */
        int pcoffset9 = offsetShift/2;
        char * bin = toBinString(pcoffset9, 9, 1);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"BRNZP")) {
        if (arg1 == NULL || arg2 != NULL) {
            printf("%s\n", "Invalid number of operands for BRnzp");
            exit(4);
        }
        strcat(mc, "0000111");
        int target = getTarget(arg1);
        int offsetShift = target - (address + 2); /* this should be even */
        int pcoffset9 = offsetShift/2;
        char * bin = toBinString(pcoffset9, 9, 1);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"HALT")) {
        if (arg1 != NULL) {
            printf("%s\n", "Invalid number of operands for HALT");
            exit(4);
        }
        strcat(mc,"1111000000100101");
        return mc;
    }
    if (!strcmp(opcode,"JMP")) {
        if (arg1 == NULL || arg2 != NULL) {
            printf("%s\n", "Invalid number of operands for JMP");
            exit(4);
        }
        strcat(mc, "1100000");
        strcat(mc, translateRegister(arg1));
        strcat(mc, "000000");
        return mc;
    }
    if (!strcmp(opcode,"JSR")) {
        if (arg1 == NULL || arg2 != NULL) {
            printf("%s\n", "Invalid number of operands for JSR");
            exit(4);
        }
        strcat(mc, "01001");
        int target = getTarget(arg1);
        int offsetShift = target - (address + 2); /* this should be even */
        int pcoffset11 = offsetShift/2;
        char * bin = toBinString(pcoffset11, 11, 1);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"JSRR")) {
        if (arg1 == NULL || arg2 != NULL ) {
            printf("%s\n", "Invalid number of operands for JSRR");
            exit(4);
        }
        strcat(mc, "0100000");
        strcat(mc, translateRegister(arg1));
        strcat(mc, "000000");
        return mc;
    }
    if (!strcmp(opcode,"LDB")) {
        if (arg3 == NULL || arg4 != NULL) {
            printf("%s\n", "Invalid number of operands for LDB");
            exit(4);
        }
        strcat(mc, "0010");
        strcat(mc, translateRegister(arg1));
        strcat(mc, translateRegister(arg2));
        int boffset6 = toNum(arg3);
        char * bin = toBinString(boffset6,6, 0);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"LDW")) {
        if (arg3 == NULL || arg4 != NULL) {
            printf("%s\n", "Invalid number of operands for LDW");
            exit(4);
        }
        strcat(mc, "0110");
        strcat(mc, translateRegister(arg1));
        strcat(mc, translateRegister(arg2));
        int offset6 = toNum(arg3);
        char * bin = toBinString(offset6,6, 0);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"LEA")) {
        if (arg2 == NULL || arg3 != NULL) {
            printf("%s\n", "Invalid number of operands for LEA");
            exit(4);
        }
        strcat(mc, "1110");
        strcat(mc, translateRegister(arg1));
        int target = getTarget(arg2);
        int offsetShift = target - (address + 2); /* this should be even */
        int pcoffset9 = offsetShift/2;
        char * bin = toBinString(pcoffset9, 9, 1);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"NOP")) {
        if (arg1 != NULL) {
            printf("%s\n", "Invalid number of operands for NOP");
            exit(4);
        }
        strcat(mc,"0000000000000000");
        return mc;
    }
    if (!strcmp(opcode,"NOT")) {
        if (arg2 == NULL || arg3 != NULL) {
            printf("%s\n", "Invalid number of operands for NOT");
            exit(4);
        }
        strcat(mc, "1001");
        strcat(mc, translateRegister(arg1));
        strcat(mc, translateRegister(arg2));
        strcat(mc, "111111");
        return mc;
    }
    if (!strcmp(opcode,"RET")) {
        if (arg1 != NULL) {
            printf("%s\n", "Invalid number of operands for RET");
            exit(4);
        }
        strcat(mc, "1100000111000000");
        return mc;
    }
    if (!strcmp(opcode,"LSHF")) {
        if (arg3 == NULL || arg4 != NULL) {
            printf("%s\n", "Invalid number of operands for LSHF");
            exit(4);
        }
        strcat(mc, "1101");
        strcat(mc, translateRegister(arg1));
        strcat(mc, translateRegister(arg2));
        int amount4 = toNum(arg3);
        /* do this even though same thing is checked in toBinStringNonNeg, cuz discription is different */
        if (amount4 < 0) {
            printf("Invalid operand: %s: shift amount must be non-nagetive\n", arg3);
            exit(3);
        }
        strcat(mc,"00");
        char * bin = toBinStringNonNeg(amount4,4);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"RSHFL")) {
        if (arg3 == NULL || arg4 != NULL) {
            printf("%s\n", "Invalid number of operands for RSHFL");
            exit(4);
        }
        strcat(mc, "1101");
        strcat(mc, translateRegister(arg1));
        strcat(mc, translateRegister(arg2));
        int amount4 = toNum(arg3);
        if (amount4 < 0) {
           printf("Invalid operand: %s: shift amount must be non-nagetive\n", arg3);
            exit(3);
        }
        strcat(mc,"01");
        char * bin = toBinStringNonNeg(amount4,4);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"RSHFA")) {
        if (arg3 == NULL || arg4 != NULL) {
            printf("%s\n", "Invalid number of operands for RSHFA");
            exit(4);
        }
        strcat(mc, "1101");
        strcat(mc, translateRegister(arg1));
        strcat(mc, translateRegister(arg2));
        int amount4 = toNum(arg3);
        if (amount4 < 0) {
           printf("Invalid operand: %s: shift amount must be non-nagetive\n", arg3);
            exit(3);
        }
        strcat(mc,"11");
        char * bin = toBinStringNonNeg(amount4,4);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"RTI")) {
        if (arg1 != NULL) {
            printf("%s\n", "Invalid number of operands for RTI");
            exit(4);
        }
        strcat(mc, "1000000000000000");
        return mc;
    }
    if (!strcmp(opcode,"STB")) {
        if (arg3 == NULL || arg4 != NULL) {
            printf("%s\n", "Invalid number of operands for STB");
            exit(4);
        }
        strcat(mc, "0011");
        strcat(mc, translateRegister(arg1));
        strcat(mc, translateRegister(arg2));
        int boffset6 = toNum(arg3);
        char * bin = toBinString(boffset6,6,0);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"STW")) {
        if (arg3 == NULL || arg4 != NULL) {
            printf("%s\n", "Invalid number of operands for STW");
            exit(4);
        }
        strcat(mc, "0111");
        strcat(mc, translateRegister(arg1));
        strcat(mc, translateRegister(arg2));
        int offset6 = toNum(arg3);
        char * bin = toBinString(offset6,6,0);
        strcat(mc, bin);
        return mc;
    }
    if (!strcmp(opcode,"TRAP")) {
        if (arg1 == NULL || arg2 != NULL) {
            printf("%s\n", "Invalid number of operands for TRAP");
            exit(4);
        }
        /* must be a hex */
        if (arg1[0] != 'x' && arg1[0] != 'X'){
            printf("Invalid trap vector %s\n", arg1);
            exit(4);
        }
        int trapvect8 = toNum(arg1);
        if (trapvect8 < 0) {
            printf("Invalid operand %s: Trap vector must be non-nagetive\n", arg1);
            exit(3);
        }
        char * bin = toBinStringNonNeg(trapvect8, 8);
        strcat(mc,"11110000");
        strcat(mc,bin);
        return mc;
    }
    if (!strcmp(opcode,"XOR")) {
        if (arg3 == NULL || arg4 != NULL) {
            printf("%s\n", "Invalid number of operands for XOR");
            exit(4);
        }
        strcat(mc, "1001");
        strcat(mc, translateRegister(arg1));
        strcat(mc, translateRegister(arg2));
        if (belongs(arg3,REGISTERS,REGISTER_NUMBER)){
            strcat(mc, "000");
            strcat(mc, translateRegister(arg3));
        } else {
            int imm5 = toNum(arg3);
            strcat(mc, "1");
            char * bin = toBinString(imm5, 5, 0);
            strcat(mc, bin);
        }
        return mc;
    }
    if (!strcmp(opcode,".FILL")) {
        /* .FILL stores a 2's complement number*/
        if (arg1 == NULL || arg2 != NULL) {
            printf("%s\n", "Invalid number of operands for .FILL");
            exit(4);
        }
        int cons = toNum(arg1);
        char * bin;
        if (cons >=0) bin = toBinStringNonNeg(cons, 16);
        else bin = toBinString(cons, 16, 0);
        strcat(mc, bin);
        return mc;
    }

    /* Not a valid opcode*/
    /* Usually the label here was intended as opcode, which turned out to be illegal so recognized as a label */
    /* but if there is an actual label used here & wrong opcode, then label is printed out */
    printf("Invalid opcode %s or %s\n", ins.label, opcode);
    exit(2);

}

char * translateRegister(char * reg) {
    char * bin = (char *) malloc(3+1);
    int i;
    for (i=0; i<REGISTER_NUMBER;i++) {
        if (!strcmp(reg, REGISTERS[i])){
            bin = toBinStringNonNeg(i, 3);
            return bin;
        }
    }

    /* invalid register*/
    printf("Invalid operand, %s\n", reg);
    exit(4);
}

/* convert to 2's complement binary representation */
char * toBinString(int dec, int bits, int isLabel) {
    /* first check if the decimal is too large to represent */
    int upper = power(2,bits-1) - 1;
    int lower = - power(2,bits-1);
    if (dec > upper || dec < lower) {
       if (isLabel) {
          printf("Destination label is too far apart\n");
          exit(4);
       }
        else {
           printf("Cannot represent the constant\n");
           exit(3);
        }
    }
    char * bin = (char *)malloc(bits+1);
    bin[bits] = '\0';
    if (dec >=0) {
        while (dec > 1) {
            bin[--bits] = (dec%2==1)?'1':'0';
            dec /= 2;
        }
        bin[--bits] = dec==1?'1':'0';
        while (bits > 0) {
            bin[--bits] = '0';
        }
    } else {
        dec = -dec -1;
        while (dec > 1) {
            bin[--bits] = dec%2==1?'0':'1';
            dec /= 2;
        }
        bin[--bits] = dec==1?'0':'1';
        while (bits > 0) {
            bin[--bits] = '1';
        }
    }
    return bin;
}

char * toBinStringNonNeg(int dec, int bits) {
    /* first check if the decimal is too large to represent */
    int upper = power(2,bits) - 1;
    int lower = 0;
    if (dec > upper || dec < lower) {
        printf("Cannot represent the constant\n");
        exit(3);
    }
    char * bin = (char *)malloc(bits+1);
    bin[bits] = '\0';

    while (dec > 1) {
        bin[--bits] = (dec%2==1)?'1':'0';
        dec /= 2;
    }
    bin[--bits] = dec==1?'1':'0';
    while (bits > 0) {
        bin[--bits] = '0';
    }
    return bin;
}

int power (int num, int power) {
    if (power == 0) return 1;
    int res = 1;
    while (power > 0) {
        res = res * num;
        power--;
    }
    return res;
}

int getTarget(char * label) {
   int i;
    for (i=0; i<tableSize; i++) {
        if (!strcmp(label, symbolTable[i].label)) {
            return symbolTable[i].address;
        }
    }

    /* if the label not found, must check whether it's a number or not */
    /* if it's a number, different error code*/
    char * p = label;
    int isNum = 1;
    if (*p=='X') {
      p++;
      if (*p == '-') {
         p++;
         int len = strlen(p);
         int i;
         for (i=0; i< len; i++, p++) {
            if (!isxdigit(*p))
               isNum = 0;
         }
      } else {
         int len = strlen(p);
         int i;
         for (i=0; i< len; i++, p++) {
            if (!isxdigit(*p))
               isNum = 0;
         }
      }
   } else if (*p == '#') {
      p++;
      if (*p == '-') {
         p++;
         int len = strlen(p);
         int i;
         for (i=0; i< len; i++, p++) {
            if (!isdigit(*p))
               isNum = 0;
         }
      } else {
         int len = strlen(p);
         int i;
         for (i=0; i< len; i++, p++) {
            if (!isdigit(*p))
               isNum = 0;
         }
      }
   } else isNum = 0;

   if (isNum) {
      printf("Operand must be a label, not a number\n");
      exit(4);
   } else {
      printf("Label %s not defined\n", label);
      exit(1);
   }
}

char * binToHex(char * bin) {
    int len = strlen(bin);
    int hexLen = len / 4 + (len%4 == 0?0:1);
    char * hex = (char *) malloc(2+hexLen+1);
    char * hex_p = hex;
    /* start with 0x */
    *hex_p = '0';
    hex_p++;
    *hex_p = 'x';
    hex_p++;

    int firstHexLen = len % 4;
    if (firstHexLen != 0) {
        char * temp = (char *)malloc(firstHexLen+1);
        char * temp_p = temp;
        while(firstHexLen > 0){
            *temp_p = *bin;
            temp_p++;
            bin++;
            firstHexLen--;
        }
        int hexDigit = strtol(temp, NULL, 2);
        free(temp);
        *hex_p = (hexDigit>=10)?(hexDigit+55):(hexDigit+48);
        hex_p++;
        hexLen--;
    }
    while (hexLen >0) {
        char * temp = (char *)malloc(4+1);
        char * temp_p = temp;
        int i;
        for(i=1; i<=4; i++) {
            *temp_p = *bin;
            temp_p++;
            bin++;
        }
        int hexDigit = strtol(temp, NULL, 2);
        free(temp);
        *hex_p = (hexDigit>=10)?(hexDigit+55):(hexDigit+48);
        hex_p++;
        hexLen--;
    }
    return hex;
}
