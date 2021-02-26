/***************************************************************/
/*                                                             */
/*   ARMv4-32 Instruction Level Simulator                      */
/*                                                             */
/*   ECEN 4243                                                 */
/*   Oklahoma State University                                 */
/*                                                             */
/***************************************************************/

#ifndef _SIM_ISA_H_
#define _SIM_ISA_H_
#define N_CUR ( (CURRENT_STATE.CPSR>>31) & 0x00000001 )
#define Z_CUR ( (CURRENT_STATE.CPSR>>30) & 0x00000001 )
#define C_CUR ( (CURRENT_STATE.CPSR>>29) & 0x00000001 )
#define V_CUR ( (CURRENT_STATE.CPSR>>28) & 0x00000001 )
#define N_NXT ( (NEXT_STATE.CPSR>>31) & 0x00000001 )
#define Z_NXT ( (NEXT_STATE.CPSR>>30) & 0x00000001 )
#define C_NXT ( (NEXT_STATE.CPSR>>29) & 0x00000001 )
#define V_NXT ( (NEXT_STATE.CPSR>>28) & 0x00000001 )

#define N_N 0x80000000
#define Z_N 0x40000000
#define C_N 0x20000000 // Carry
#define V_N 0x10000000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

/*
    Rd - Destination Register
    Rn - First Source Register
    Src2 - Second Source Register
    I - if I = 1 then Src2 is immediate, if I = 0 then Scr2 is a Register
    S - if S = 1 then set condition flags otherwise don't

    Each word consists of 32 bits here is what each bit does:
    31:28 - Condition codes (4 bits)
    27:26 - OPCODE (2 bits)
    25:20 - function to be performed (6 bits)
            function consists of the I bit, cmd, and the S bit
              25 - I
              24:21 - command
              20 - S
    19:16 - Rd (4 bits)
    15:12 - Rn (4 bits)
    11:0 - Src2 (12 bits)
            Src2 contains shamt5, sh, and Rm
              11:7 - shamt5, this is the amount Rm is shifted
              6:5 - sh, this is the type of shift for Rm (i.e. <<, >>, >>>, ROR)
              4 - this bit is always 0
              3:0 - Rm, the second source operand

*/

int ADD (int Rd, int Rn, int Operand2, int I, int S, int CC) {

  int cur = 0;

  //If I = 0 then the processor has to go get Operand2 from memory
  if(I == 0) {
    /*
      These integers apply a mask to different parts of the operand in order to
      look at specific values
    */
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;

    /*This IF checks bit 4 is 1 or 0
        1 means that this is a Register
        0 means that this is a Register-shifted Register
    */
    if (bit4 == 0)
      //switch determines how Rm will be shifted
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] +
  	     (CURRENT_STATE.REGS[Rm] << shamt5);
  	    break;
        case 1: cur = CURRENT_STATE.REGS[Rn] +
  	     (CURRENT_STATE.REGS[Rm] >> shamt5);
  	    break;
        case 2: cur = CURRENT_STATE.REGS[Rn] +
  	     (CURRENT_STATE.REGS[Rm] >> shamt5);
    	  break;
        case 3: cur = CURRENT_STATE.REGS[Rn] +
	       ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
	      break;
      }else
        //switch determines how Rm will be shifted
        switch (sh) {
          case 0: cur = CURRENT_STATE.REGS[Rn] +
  	       (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
  	      break;
          case 1: cur = CURRENT_STATE.REGS[Rn] +
  	       (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
  	      break;
          case 2: cur = CURRENT_STATE.REGS[Rn] +
  	       (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
  	      break;
          case 3: cur = CURRENT_STATE.REGS[Rn] +
  	       ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                 (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
  	      break;
        }
  }

  /*
    If I = 1 then the number being added is there in the command and there
    is no reason to go to memory again
  */
  if (I == 1) {

    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] + (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;

  /*
    If S = 1 then set the condition flags
  */
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if(cur > 0xffffffff)
      NEXT_STATE.CPSR |= V_N;
  }
  return 0;

}
int ADC (int Rd, int Rn, int Operand2, int I, int S, int CC) {

  int cur = 0;

  //If I = 0 then the processor has to go get Operand2 from memory
  if(I == 0) {
    /*
      These integers apply a mask to different parts of the operand in order to
      look at specific values
    */
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;

    /*This IF checks bit 4 is 1 or 0
        1 means that this is a Register
        0 means that this is a Register-shifted Register
    */
    if (bit4 == 0)
      //switch determines how Rm will be shifted
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] +
  	     (CURRENT_STATE.REGS[Rm] << shamt5);
  	    break;
        case 1: cur = CURRENT_STATE.REGS[Rn] +
  	     (CURRENT_STATE.REGS[Rm] >> shamt5);
  	    break;
        case 2: cur = CURRENT_STATE.REGS[Rn] +
  	     (CURRENT_STATE.REGS[Rm] >> shamt5);
    	  break;
        case 3: cur = CURRENT_STATE.REGS[Rn] +
	       ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
	      break;
      }else
        //switch determines how Rm will be shifted
        switch (sh) {
          case 0: cur = CURRENT_STATE.REGS[Rn] +
  	       (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
  	      break;
          case 1: cur = CURRENT_STATE.REGS[Rn] +
  	       (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
  	      break;
          case 2: cur = CURRENT_STATE.REGS[Rn] +
  	       (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
  	      break;
          case 3: cur = CURRENT_STATE.REGS[Rn] +
  	       ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                 (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
  	      break;
        }
  }

  /*
    If I = 1 then the number being added is there in the command and there
    is no reason to go to memory again
  */
  if (I == 1) {

    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] + (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;

  /*
    If S = 1 then set the condition flags
  */
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }
  return 0;

}
int AND (int Rd, int Rn, int Operand2, int I, int S, int CC){

    int cur = 0;

    //If I = 0 then the processor has to go get Operand2 from memory
    if(I == 0) {
      /*
        These integers apply a mask to different parts of the operand in order to
        look at specific values
      */
      int sh = (Operand2 & 0x00000060) >> 5;
      int shamt5 = (Operand2 & 0x00000F80) >> 7;
      int bit4 = (Operand2 & 0x00000010) >> 4;
      int Rm = Operand2 & 0x0000000F;
      int Rs = (Operand2 & 0x00000F00) >> 8;

      /*This IF checks bit 4 is 1 or 0
          1 means that this is a Register
          0 means that this is a Register-shifted Register
      */
      if (bit4 == 0)
        //switch determines how Rm will be shifted
        switch (sh) {
          case 0: cur = CURRENT_STATE.REGS[Rn] &
    	     (CURRENT_STATE.REGS[Rm] << shamt5);
    	    break;
          case 1: cur = CURRENT_STATE.REGS[Rn] &
    	     (CURRENT_STATE.REGS[Rm] >> shamt5);
    	    break;
          case 2: cur = CURRENT_STATE.REGS[Rn] &
    	     (CURRENT_STATE.REGS[Rm] >> shamt5);
      	  break;
          case 3: cur = CURRENT_STATE.REGS[Rn] &
  	       ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                 (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
  	      break;
        }else
          //switch determines how Rm will be shifted
          switch (sh) {
            case 0: cur = CURRENT_STATE.REGS[Rn] &
    	       (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    	      break;
            case 1: cur = CURRENT_STATE.REGS[Rn] &
    	       (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    	      break;
            case 2: cur = CURRENT_STATE.REGS[Rn] &
    	       (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    	      break;
            case 3: cur = CURRENT_STATE.REGS[Rn] &
    	       ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                   (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
    	      break;
          }
    }

    /*
      If I = 1 then the number being added is there in the command and there
      is no reason to go to memory again
    */
    if (I == 1) {

      int rotate = Operand2 >> 8;
      int Imm = Operand2 & 0x000000FF;
      cur = CURRENT_STATE.REGS[Rn] & (Imm>>2*rotate|(Imm<<(32-2*rotate)));
    }
    NEXT_STATE.REGS[Rd] = cur;

    /*
      If S = 1 then set the condition flags
    */
    if (S == 1) {
      if (cur < 0)
        NEXT_STATE.CPSR |= N_N;
      if (cur == 0)
        NEXT_STATE.CPSR |= Z_N;
      if(cur > 0xffffffff)
        NEXT_STATE.CPSR |= V_N;
    }
    return 0;
}
int ASR (char* i_);
int B (char* i_);
int BIC (char* i_);
int BL (char* i_);
int CMN (char* i_);
int CMP (char* i_);
int EOR (int Rd, int Rn, int Operand2, int I, int S, int CC){

      int cur = 0;
      int nand = 0;
      int or = 0;

      //If I = 0 then the processor has to go get Operand2 from memory
      if(I == 0) {
        /*
          These integers apply a mask to different parts of the operand in order to
          look at specific values
        */
        int sh = (Operand2 & 0x00000060) >> 5;
        int shamt5 = (Operand2 & 0x00000F80) >> 7;
        int bit4 = (Operand2 & 0x00000010) >> 4;
        int Rm = Operand2 & 0x0000000F;
        int Rs = (Operand2 & 0x00000F00) >> 8;

        /*This IF checks bit 4 is 1 or 0
            1 means that this is a Register
            0 means that this is a Register-shifted Register
        */
        if (bit4 == 0)
          //switch determines how Rm will be shifted
          switch (sh) {
            case 0: nand = !(CURRENT_STATE.REGS[Rn] &
      	     (CURRENT_STATE.REGS[Rm] << shamt5));
              or = CURRENT_STATE.REGS[Rn] |
              (CURRENT_STATE.REGS[Rm] << shamt5);
              cur = nand & or;
      	    break;
            case 1: nand = !(CURRENT_STATE.REGS[Rn] &
      	     (CURRENT_STATE.REGS[Rm] >> shamt5));
              or = CURRENT_STATE.REGS[Rn] |
              (CURRENT_STATE.REGS[Rm] >> shamt5);
              cur = nand & or;
      	    break;
            //Need to finish cases 2 and 3
            case 2: cur = CURRENT_STATE.REGS[Rn] &
      	     (CURRENT_STATE.REGS[Rm] >> shamt5);
        	  break;
            case 3: cur = CURRENT_STATE.REGS[Rn] &
    	       ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                   (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
    	      break;
          }else
            //switch determines how Rm will be shifted
            switch (sh) {
              case 0: nand = !(CURRENT_STATE.REGS[Rn] &
      	       (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]));
               or = CURRENT_STATE.REGS[Rn] |
       	       (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
               cur = nand & or;
      	      break;
              case 1: nand = !(CURRENT_STATE.REGS[Rn] &
      	       (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]));
               or = !(CURRENT_STATE.REGS[Rn] |
       	       (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]));
               cur= nand & or;
      	      break;
              // Need to finihs cases 2 and 3
              case 2: cur = CURRENT_STATE.REGS[Rn] &
      	       (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
      	      break;
              case 3: cur = CURRENT_STATE.REGS[Rn] &
      	       ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                     (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
      	      break;
            }
      }

      /*
        If I = 1 then the number being added is there in the command and there
        is no reason to go to memory again
      */
      if (I == 1) {

        int rotate = Operand2 >> 8;
        int Imm = Operand2 & 0x000000FF;
        nand = !(CURRENT_STATE.REGS[Rn] & (Imm>>2*rotate|(Imm<<(32-2*rotate))));
        or = CURRENT_STATE.REGS[Rn] | (Imm>>2*rotate|(Imm<<(32-2*rotate)));
        cur = nand & or;
      }
      NEXT_STATE.REGS[Rd] = cur;

      /*
        If S = 1 then set the condition flags
      */
      if (S == 1) {
        if (cur < 0)
          NEXT_STATE.CPSR |= N_N;
        if (cur == 0)
          NEXT_STATE.CPSR |= Z_N;
        if(cur > 0xffffffff)
          NEXT_STATE.CPSR |= V_N;
      }
      return 0;
}
int LDR (char* i_);
int LDRB (char* i_);
int LSL (char* i_);
int LSR (char* i_);
int MLA (char* i_);
int MOV (char* i_);
int MUL (char* i_);
int MVN (char* i_);
int ORR (int Rd, int Rn, int Operand2, int I, int S, int CC){
    int cur = 0;

    //If I = 0 then the processor has to go get Operand2 from memory
    if(I == 0) {
      /*
        These integers apply a mask to different parts of the operand in order to
        look at specific values
      */
      int sh = (Operand2 & 0x00000060) >> 5;
      int shamt5 = (Operand2 & 0x00000F80) >> 7;
      int bit4 = (Operand2 & 0x00000010) >> 4;
      int Rm = Operand2 & 0x0000000F;
      int Rs = (Operand2 & 0x00000F00) >> 8;

      /*This IF checks bit 4 is 1 or 0
          1 means that this is a Register
          0 means that this is a Register-shifted Register
      */
      if (bit4 == 0)
        //switch determines how Rm will be shifted
        switch (sh) {
          case 0: cur = CURRENT_STATE.REGS[Rn] |
    	     (CURRENT_STATE.REGS[Rm] << shamt5);
    	    break;
          case 1: cur = CURRENT_STATE.REGS[Rn] |
    	     (CURRENT_STATE.REGS[Rm] >> shamt5);
    	    break;
          case 2: cur = CURRENT_STATE.REGS[Rn] |
    	     (CURRENT_STATE.REGS[Rm] >> shamt5);
      	  break;
          case 3: cur = CURRENT_STATE.REGS[Rn] |
  	       ((CURRENT_STATE.REGS[Rm] >> shamt5) |
                 (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
  	      break;
        }else
          //switch determines how Rm will be shifted
          switch (sh) {
            case 0: cur = CURRENT_STATE.REGS[Rn] |
    	       (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    	      break;
            case 1: cur = CURRENT_STATE.REGS[Rn] |
    	       (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    	      break;
            case 2: cur = CURRENT_STATE.REGS[Rn] |
    	       (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    	      break;
            case 3: cur = CURRENT_STATE.REGS[Rn] |
    	       ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
                   (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
    	      break;
          }
    }

    /*
      If I = 1 then the number being added is there in the command and there
      is no reason to go to memory again
    */
    if (I == 1) {

      int rotate = Operand2 >> 8;
      int Imm = Operand2 & 0x000000FF;
      cur = CURRENT_STATE.REGS[Rn] | (Imm>>2*rotate|(Imm<<(32-2*rotate)));
    }
    NEXT_STATE.REGS[Rd] = cur;

    /*
      If S = 1 then set the condition flags
    */
    if (S == 1) {
      if (cur < 0)
        NEXT_STATE.CPSR |= N_N;
      if (cur == 0)
        NEXT_STATE.CPSR |= Z_N;
      if(cur > 0xffffffff)
        NEXT_STATE.CPSR |= V_N;
    }
    return 0;
}
int ROR (char* i_);
int SBC (char* i_);
int STR (char* i_);
int STRB (char* i_);
int SUB (int Rd, int Rn, int Operand2, int I, int S, int CC) {

  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0)
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] -
	  (CURRENT_STATE.REGS[Rm] << shamt5);
	  break;
      case 1: cur = CURRENT_STATE.REGS[Rn] -
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
	  break;
      case 2: cur = CURRENT_STATE.REGS[Rn] -
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
    	  break;
      case 3: cur = CURRENT_STATE.REGS[Rn] -
	      ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
	  break;
      }
    else
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] -
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
	  break;
      case 1: cur = CURRENT_STATE.REGS[Rn] -
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	  break;
      case 2: cur = CURRENT_STATE.REGS[Rn] -
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	  break;
      case 3: cur = CURRENT_STATE.REGS[Rn] -
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
	  break;
      }
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] - (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if(cur > 0xffffffff)
      NEXT_STATE.CPSR |= V_N;
  }
  return 0;
}
int TEQ (char* i_);
int TST (char* i_);
int SWI (char* i_){return 0;}

#endif
