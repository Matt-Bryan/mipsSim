/*----------------------------------------------------------------------*
 *	Example mips_asm program loader. This loads the mips_asm binary	*
 *	named "testcase1.mb" into an array in memory. It reads		*
 *	the 64-byte header, then loads the code into the mem array.	*
 *									*
 *	DLR 4/18/16							*
 *
 *
 *  This file will be modified to act as a fully-fledged MIPS simulator.
 *----------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mips_asm_header.h"

#define OPMASK 0xFC000000	//Right shift 26
#define RSMASK 0x03E00000   //21
#define RTMASK 0x001F0000   //16
#define RDMASK 0x0000F800   //11
#define SHMASK 0x000007C0   //6
#define FNMASK 0x0000003F
#define IMMASK 0x0000FFFF
#define JIMASK 0x03FFFFFF
#define SYMASK 0xFFFFFFF0

typedef unsigned int MIPS, *MIPS_PTR;

MB_HDR mb_hdr;		/* Header area */
static MIPS mem[1024];		/* Room for 4K bytes */
static unsigned int reg[32], PC, *nextInstruction, memRef;
static float clockCount;


int loadMemory(char *filename)	//This function acts as step 1 of lab 5, and loads the MIPS binary file into an array
  {
  FILE *fd;
  int n;
  int memp;
/* format the MIPS Binary header */

  fd = fopen(filename, "rb");
  if (fd == NULL) { printf("\nCouldn't load test case - quitting.\n"); exit(99); }

  memp = 0;		/* This is the memory pointer, a byte offset */

/* read the header and verify it. */
  fread((void *) &mb_hdr, sizeof(mb_hdr), 1, fd);
  if (! strcmp(mb_hdr.signature, "~MB")==0)
    { printf("\nThis isn't really a mips_asm binary file - quitting.\n"); exit(98); }
  
  printf("\n%s Loaded ok, program size=%d bytes.\n\n", filename, mb_hdr.size);

/* read the binary code a word at a time */
  
  do {
    n = fread((void *) &mem[memp/4], 4, 1, fd); /* note div/4 to make word index */
    if (n) 
      memp += 4;	/* Increment byte pointer by size of instr */
    else
      break;       
    } while (memp < sizeof(mem)) ;

  fclose(fd);

  return memp;
  }

/* Takes the location in memory of the next instruction as an arguement.
*  
*  Modifies the nextInstruction buffer with details about the next Instruction
*  
*  If it detects a syscall halt, it only puts -1 in first spot of buffer  -MB*/
void decode(int memLoc) {
	unsigned int instr = mem[memLoc/4], opCode;
	int count;

	for (count = 0; count < 6; count++) {	/* Clear out nextInstruction buffer */
		nextInstruction[count] = 0;
	}
	opCode = (instr & OPMASK) >> 26;
	nextInstruction[0] = opCode;

	if ((instr & SYMASK) == 0 && reg[2] == 0) {
		//Syscall HALT
		nextInstruction[0] = -1;
	}
	else if (opCode == 0) {
		//Register
		nextInstruction[1] = (instr & RSMASK) >> 21;
		nextInstruction[2] = (instr & RTMASK) >> 16;
		nextInstruction[3] = (instr & RDMASK) >> 11;
		nextInstruction[4] = (instr & SHMASK) >> 6;
		nextInstruction[5] = (instr & FNMASK);
	}
	else if (opCode == 0x02 || opCode == 0x03) {
		//Jump
		nextInstruction[1] = (instr & JIMASK);
	}
	else {
		//Immediate
		nextInstruction[1] = (instr & RSMASK) >> 21;
		nextInstruction[2] = (instr & RTMASK) >> 16;
		nextInstruction[3] = (instr & IMMASK);
	}
}

/* Sub routine of execute to handle register-type instructions */
void exReg()
{
   unsigned int rs = nextInstruction[1];
   unsigned int rt = nextInstruction[2];
   unsigned int rd = nextInstruction[3]; 
   unsigned int shamt = nextInstruction[4];  
   unsigned int func = nextInstruction[5];
   int signedVal1 = 0;
   int signedVal2 = 0;

   switch(func) 
   {
      case 0x00 : 	// sll
         reg[rd] = (reg[rt] << shamt);
         PC += 4; 
         break;
      case 0x02 : 	// srl
         reg[rd] = (reg[rt] >> shamt);
         PC += 4;
         break;
      case 0x03 : 	// sra ***NEED TO TEST
         signedVal1 = (int) reg[rt];
         reg[rd] = (signedVal1 >> shamt);
         PC += 4;
         break;
      case 0x04 : 	// sllv
         reg[rd] = (reg[rt] << reg[rs]);
         PC += 4;
         break;
      case 0x06 : 	// srlv
         reg[rd] = (reg[rt] >> reg[rs]);
         PC += 4;
         break;
      case 0x07 : 	// srav
         signedVal1 = (int) reg[rt];
         reg[rd] = (signedVal1 >> reg[rs]);
         PC += 4;
         break;
      case 0x08 : 	// jr
         PC = reg[rs];
         break;
      case 0x09 : 	// jalr
         reg[31] = PC + 4;
         PC = reg[rs];
         break;
      case 0x20 : 	// add
         signedVal1 = (int) reg[rs];
         signedVal2 = (int) reg[rt];
         reg[rd] = signedVal1 + signedVal2;
         PC += 4;
         break;
      case 0x21 : 	// addu
         reg[rd] = reg[rs] + reg[rt];
         PC += 4;
         break;
      case 0x22 : 	// sub
         signedVal1 = (int) reg[rs];
         signedVal2 = (int) reg[rt];
         reg[rd] = signedVal1 - signedVal2;
         PC += 4;
         break;
      case 0x23 : 	// subu
         reg[rd] = (reg[rs] - reg[rt]);
         PC += 4;
         break;
      case 0x24 : 	// and
         reg[rd] = (reg[rs] & reg[rt]);
         PC += 4;
         break;
      case 0x25 : 	// or
         reg[rd] = (reg[rs] | reg[rt]);
         PC += 4;
         break;
      case 0x26 : 	// Xor
         reg[rd] = (reg[rs] ^ reg[rt]);
         PC += 4;
         break;
      case 0x27 : 	// Nor
         reg[rd] = ~(reg[rs] | reg[rt]);
         PC += 4;
         break;
      case 0x2A : 	// slt
         signedVal1 = (int) reg[rs];
         signedVal2 = (int) reg[rt];
         reg[rd] = (signedVal1 < signedVal2);
         PC += 4;
         break;
      case 0x2B : 	// sltu
         reg[rd] = (reg[rs] < reg[rt]);
         PC += 4;
         break;
   }
   clockCount += 4;
}

void execute() {
<<<<<<< HEAD
	 int count = 0;

	 for (count; count < 6; count++) {
	 	printf("nextInstruction[%d]: %08X\n", count, nextInstruction[count]);
	 }
	 printf("\n");
=======



	// int count = 0;

	// for (count; count < 6; count++) {
	// 	printf("nextInstruction[%d]: %08X\n", count, nextInstruction[count]);
	// }
	// printf("\n");
>>>>>>> af910850915729dd3722c54ab3485b0f9fabac1b
}

/* Displays the number of instructions simulated,
*  the number of memory references, and the CPI, 
*  up until this point.
*
*  Takes the corresponding details from main as
*  arguements.    -MB */
void displayResult(int numInstr, float clockCount, int memRef) {
	int count = 0;

	for (count; count < 32; count++) {
		printf("R%d = %08X\n", count, reg[count]);
	}
	printf("Number of instructions simulated: %d\nMemory References: %d\nCPI: %.2f\n\n", numInstr, memRef, clockCount / numInstr);
}

int main(int argc, char **argv) {
	int curLoc = 0, memp, input = 0, numInstr = 0;

	nextInstruction = calloc(6, sizeof(int));
	clockCount = 0.0;
	memRef = 0;

	memp = loadMemory(argv[1]);

	do {
		printf("Enter 0 for Run, 1 for Single-Step, or -1 to exit\n");

		if (scanf("%02d", &input) >= 0 && input != -1) {

			if (input == 1) {
				//Single-step

				decode(curLoc);
				execute();
				curLoc += 4;
				numInstr++;
				displayResult(numInstr, clockCount, memRef);
			}
			else {
				//Running

				for (curLoc; curLoc < memp; curLoc += 4) {
					decode(curLoc);
					execute();
					numInstr++;
				}
				displayResult(numInstr, clockCount, memRef);
			}
		}
	} while (input > 0);

	// for (i = 0; i < memp; i += 4) {
 //       printf("Instruction@%08X : %08X\n", i, mem[i/4]);
 //    }

	free(nextInstruction);
	return 0;
}
