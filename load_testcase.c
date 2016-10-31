/*----------------------------------------------------------------------*
 *	Example mips_asm program loader. This loads the mips_asm binary	*
 *	named "testcase1.mb" into an array in memory. It reads		*
 *	the 64-byte header, then loads the code into the mem array.	*
 *									*
 *	DLR 4/18/16							*
 *
 *
 *	Modified original program loader to decode ANY MIPS binary file
 *  and print out extra information regarding the type of instruction
 *  (R, I, J), the OpCode, the function code (if applicable), and
 *  other small details relevant to specific instructions.
 *
 *  Added function: decodeInstructions()
 *  Added bitmasks for decoding
 *
 *  NOTE: Changed calling convention: Must include MIPS binary filename
 *  as second commandline arguement
 *
 *  Matthew Bryan, Tristan Broughton, Sam Rastovich 10/28/16
 *----------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mips_asm_header.h"

typedef unsigned int MIPS, *MIPS_PTR;

MB_HDR mb_hdr;		/* Header area */
MIPS mem[1024];		/* Room for 4K bytes */

							//Shift amounts to isolate value:
#define OPMASK 0xFC000000	//Right shift 26
#define RSMASK 0x03E00000   //21
#define RTMASK 0x001F0000   //16
#define RDMASK 0x0000F800   //11
#define SHMASK 0x000007C0   //6
#define FNMASK 0x0000003F
#define IMMASK 0x0000FFFF
#define JIMASK 0x03FFFFFF
#define SYMASK 0xFFFFFFF0

void decodeInstructions(MIPS *mem, int i) {
	unsigned int current, opCode, sourceReg, targReg, destReg, shiftAmt, function;
	signed int immediate;

	current = mem[i / 4];

	//check instruction opcode
	opCode = (current & OPMASK) >> 26;
	if ((current & SYMASK) == 0) {
		printf(" R Syscall");
	}
	else if (opCode == 0) {	//Corresponds to any Register instruction
		printf(" R");
		sourceReg = (current & RSMASK) >> 21;
		targReg = (current & RTMASK) >> 16;
		destReg = (current & RDMASK) >> 11;
		shiftAmt = (current & SHMASK) >> 6;
		function = (current & FNMASK);

		printf(" OpCode: 0, Function Code: %02X,", function);

		switch (function) {	//checks function code

			case 0x00 :
				printf(" Shift Left Logical %d position(s)", shiftAmt);
				break;

			case 0x02 :
				printf(" Shift Right Logical %d position(s)", shiftAmt);
				break;

			case 0x03 :
				printf(" Shift Right Arithmetic %d position(s)", shiftAmt);
				break;

			case 0x04 :
				printf(" Shift Left Logical Variable, Register R%d to be shifted by value in R%d", 
				 targReg, sourceReg);
				break;

			case 0x06 :
				printf(" Shift Right Logical Variable, Register R%d to be shifted by value in R%d", 
				 targReg, sourceReg);
				break;

			case 0x07 :
				printf(" Shift Right Arithmetic Variable, Register R%d to be shifted by value in R%d", 
				 targReg, sourceReg);
				break;

			case 0x20 :
				printf(" Add");
				break;

			case 0x21 :
				printf(" Add Unsigned");
				break;

			case 0x22 :
				printf(" Subtract");
				break;

			case 0x23 :
				printf(" Subtract Unsigned");
				break;

			case 0x24 :
				printf(" And");
				break;

			case 0x27 :
				printf(" Nor");
				break;

			case 0x25 :
				printf(" Or");
				break;

			case 0x26 :
				printf(" Xor");
				break;

			case 0x2A :
				printf(" Set less than");
				break;

			case 0x2B :
				printf(" Set less than unsigned");
				break;

			case 0x08 :
				printf(" Jump Register");
				break;

			case 0x09 :
				printf(" Jump and link Register");
				break;

			default :
				printf(" Invalid Instruction");
				break;
		}
	}
	else if (opCode == 0x02 || opCode == 0x03) {	//Jump instruction
		printf(" J");
		printf(" OpCode: %02X,", opCode);
		immediate = (current & JIMASK);

		printf(" Effective Address: %08X", immediate);
	}
	else if (opCode == 0x04 || opCode == 0x05) {	//Branch Instruction
		printf(" I");
		printf(" OpCode: %02X,", opCode);
		sourceReg = (current & RSMASK) >> 21;
		targReg = (current & RTMASK) >> 16;
		immediate = (current & IMMASK);
		printf(" Branch - Effective Address: %08X, Registers being compared: R%d and R%d", (immediate << 2) + 4, sourceReg, targReg);
	}
	else {	//Immediate Instruction otherwise
		printf(" I");
		printf(" OpCode: %02X,", opCode);
		sourceReg = (current & RSMASK) >> 21;
		targReg = (current & RTMASK) >> 16;
		immediate = (current & IMMASK);

		switch (opCode) {

			case 0x08 :
				printf(" Add Immediate,");
				if (immediate & 0x00008000) {
					immediate |= 0xFFFF0000;
				}
				printf(" RS = R%d, RT = R%d, Immediate = %08X", sourceReg, targReg, immediate);
				break;

			case 0x09 :
				printf(" Add Immediate Unsigned,");
				if (immediate & 0x00008000) {
					immediate |= 0xFFFF0000;
				}
				printf(" RS = R%d, RT = R%d, Immediate = %08X", sourceReg, targReg, immediate);
				break;

			case 0x0C :
				printf(" And Immediate,");
				printf(" RS = R%d, RT = R%d, Immediate = %08X", sourceReg, targReg, immediate);
				break;

			case 0x0D :
				printf(" Or Immediate,");
				printf(" RS = R%d, RT = R%d, Immediate = %08X", sourceReg, targReg, immediate);
				break;

			case 0x0E :
				printf(" Xor Immediate,");
				printf(" RS = R%d, RT = R%d, Immediate = %08X", sourceReg, targReg, immediate);
				break;

			case 0x0A :
				printf(" Set less than Immediate,");
				printf(" RS = R%d, RT = R%d, Immediate = %08X", sourceReg, targReg, immediate);
				break;

			case 0x0B :
				printf(" Set less than Immediate Unsigned,");
				printf(" RS = R%d, RT = R%d, Immediate = %08X", sourceReg, targReg, immediate);
				break;

			default :
				printf(" Invalid Instruction");
				break;
		}

		//printf(" RS: %08X, RT: %08X, IM: %08X", sourceReg, targReg, immediate);
	}
	//printf(" %08X", opCode);

	printf("\n");
}

main(int argc, char **argv)
  {
  FILE *fd;
  int n;
  int memp;
  int i;
/* format the MIPS Binary header */

  fd = fopen(argv[1], "rb");
  if (fd == NULL) { printf("\nCouldn't load test case - quitting.\n"); exit(99); }

  memp = 0;		/* This is the memory pointer, a byte offset */

/* read the header and verify it. */
  fread((void *) &mb_hdr, sizeof(mb_hdr), 1, fd);
  if (! strcmp(mb_hdr.signature, "~MB")==0)
    { printf("\nThis isn't really a mips_asm binary file - quitting.\n"); exit(98); }
  
  printf("\n%s Loaded ok, program size=%d bytes.\n\n", argv[1], mb_hdr.size);

/* read the binary code a word at a time */
  
  do {
    n = fread((void *) &mem[memp/4], 4, 1, fd); /* note div/4 to make word index */
    if (n) 
      memp += 4;	/* Increment byte pointer by size of instr */
    else
      break;       
    } while (memp < sizeof(mem)) ;

  fclose(fd);


/* ok, now dump out the instructions loaded: */

  for (i = 0; i<memp; i+=4)	/* i contains byte offset addresses */
     {
     printf("Instruction@%08X : %08X", i, mem[i/4]);
     decodeInstructions(mem, i);
     printf("\n");
     }
  printf("\n");

  exit(0);
  }