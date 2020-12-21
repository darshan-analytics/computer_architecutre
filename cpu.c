  /*
   *  cpu.c
   *  Contains APEX cpu pipeline implementation
   *
   *  Author :
   *  Gaurav Kothari (gkothar1@binghamton.edu)
   *  State University of New York, Binghamton
   */
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>

  #include "cpu.h"

  /* Set this flag to 1 to enable debug messages */
  #define ENABLE_DEBUG_MESSAGES 1

  /*
   * This function creates and initializes APEX cpu.
   *
   * Note : You are free to edit this function according to your
   * 				implementation
   */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

    /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

    /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
      "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
      cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
       cpu->code_memory[i].opcode,
       cpu->code_memory[i].rd,
       cpu->code_memory[i].rs1,
       cpu->code_memory[i].rs2,
       cpu->code_memory[i].imm);
    }
  }

    /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

  /*
   * This function de-allocates APEX cpu.
   *
   * Note : You are free to edit this function according to your
   * 				implementation
   */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

  /* Converts the PC(4000 series) into
   * array index for code memory
   *
   * Note : You are not supposed to edit this function
   *
   */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0)
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (strcmp(stage->opcode, "STR") == 0)
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode,stage->rd, stage->rs1, stage->rs2);
  }

   if (strcmp(stage->opcode, "LDR") == 0)
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode,stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "LOAD") == 0) 
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) 
  {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }

  if (strcmp(stage->opcode, "ADD") == 0) 
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }

  if (strcmp(stage->opcode, "ADDL") == 0) 
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1,stage->imm);
  }

  if (strcmp(stage->opcode, "SUB") == 0) 
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }

  if (strcmp(stage->opcode, "AND") == 0) 
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }

  if (strcmp(stage->opcode, "OR") == 0) 
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }

  if (strcmp(stage->opcode, "XOR") == 0) 
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }

  if (strcmp(stage->opcode, "MUL") == 0) 
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }

  if (strcmp(stage->opcode, "BZ") == 0) 
  {
    printf("%s,#%d", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "BNZ") == 0) 
  {
    printf("%s,#%d", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "JUMP") == 0) 
  {
    printf("%s,R%d,#%d", stage->opcode,stage->rs1,stage->imm);
  }

  if (strcmp(stage->opcode, "HALT") == 0)
  {
    printf("HALT");
  }

  if(strcmp(stage->opcode, "Due to Halt")==0){
    printf("%s", stage->opcode);
  }

}

  /* Debug function which dumps the cpu stage
   * content
   *
   * Note : You are not supposed to edit this function
   *
   */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

  /*
   *  Fetch Stage of APEX Pipeline
   *
   *  Note : You are free to edit this function according to your
   * 				 implementation
   */
int
fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];
  if (!stage->busy && !stage->stalled) {  
      /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

      /* Index into code memory using this pc and copy all instruction fields into
       * fetch latch
       */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

    if(!cpu->stage[DRF].stalled)
    {
      /* Update PC for next instruction */
      cpu->pc += 4;

      /* Copy data from fetch latch to decode latch*/
      cpu->stage[DRF] = cpu->stage[F];
    }


    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Fetch", stage);
    }
  }
  else{print_stage_content("Fetch", stage);}
  
  
  return 0;}

  /*
   *  Decode Stage of APEX Pipeline
   *
   *  Note : You are free to edit this function according to your
   * 				 implementation
   */
int
decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];

  if(stage->stalled) {
    stage->stalled = 0;
  }
  if(stage->forward_enabler==1){
  cpu->regs[stage->forward_regindex]=stage->forward_buffer;
  stage->forward_enabler=0;
 // printf("\n\nR%d----%d\n\n",stage->forward_regindex,stage->forward_buffer);
}

  if (!stage->busy && !stage->stalled) 
  {
      
      /* Read data from register file for store */
    if (strcmp(stage->opcode, "HALT") == 0) 
    {
      cpu->stage[F].stalled=1; 
        cpu->stage[DRF].stalled=0;
        cpu->stage[F].pc =0;
        strcpy(cpu->stage[F].opcode, "HALT");

    }
    
    if (strcmp(stage->opcode, "STORE") == 0) 
    {
      if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])  //Valid bits of regs checked for dependency
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value= cpu->regs[stage->rs1];
        stage->rs2_value= cpu->regs[stage->rs2];
      }
      else
      {
        cpu->stage[F].stalled=1; 
        cpu->stage[DRF].stalled=1;
      }
    }

    if (strcmp(stage->opcode, "STR") == 0) 
    {
      if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2]&& cpu->regs_valid[stage->rd])  //Valid bits of regs checked for dependency
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value= cpu->regs[stage->rs1];
        stage->rs2_value= cpu->regs[stage->rs2];
        stage->buffer=cpu->regs[stage->rd];
      }
      else
      {
        cpu->stage[F].stalled=1; 
        cpu->stage[DRF].stalled=1;
      }
    }


      /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
        
        cpu->regs_valid[stage->rd]=0;    
      }
   

      if (strcmp(stage->opcode, "LOAD") == 0) 
      {


        
        if(cpu->regs_valid[stage->rs1])       
        {
          cpu->stage[F].stalled=0; 
          cpu->stage[DRF].stalled=0;
        stage->rs1_value= cpu->regs[stage->rs1];  // 0 is invalid for dependency
        cpu->regs_valid[stage->rd]=0; 
      }
      else
      {
        cpu->stage[F].stalled=1; 
        cpu->stage[DRF].stalled=1;
      }

    }

    if (strcmp(stage->opcode, "LDR") == 0) 
      {


        
        if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])       
        {
          cpu->stage[F].stalled=0; 
          cpu->stage[DRF].stalled=0;
        stage->rs1_value= cpu->regs[stage->rs1];  // 0 is invalid for dependency
        stage->rs2_value= cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd]=0; 
      }
      else
      {
        cpu->stage[F].stalled=1; 
        cpu->stage[DRF].stalled=1;
      }

    }

    if (strcmp(stage->opcode, "JUMP") == 0) 
    {
      
    stage->rs1_value= cpu->regs[stage->rs1];
    }

    if (strcmp(stage->opcode, "ADD") == 0) 
    {stage->math_ins++;

      
      if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])  
      {
        cpu->stage[F].stalled=0; 
        cpu->stage[DRF].stalled=0;
        stage->rs1_value= cpu->regs[stage->rs1];//printf("pPPPASSSSmake\n");
        stage->rs2_value= cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd]=0;
        
      }
else
    {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
    }

    }

     if (strcmp(stage->opcode, "ADDL") == 0) 
    {stage->math_ins++;

      
      if(cpu->regs_valid[stage->rs1])  
      {
        cpu->stage[F].stalled=0; 
        cpu->stage[DRF].stalled=0;
        stage->rs1_value= cpu->regs[stage->rs1];//printf("pPPPASSSSmake\n");
       
        cpu->regs_valid[stage->rd]=0;
        
      }
else
    {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
    }

    }

    if (strcmp(stage->opcode, "SUB") == 0) 
    {stage->math_ins++;
      
      if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])  
      {
        cpu->stage[F].stalled=0; 
        cpu->stage[DRF].stalled=0;
        stage->rs1_value= cpu->regs[stage->rs1];
        stage->rs2_value= cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd]=0;
        
      }
      else
      {
        cpu->stage[F].stalled=1; 
        cpu->stage[DRF].stalled=1;
      }
    }

    if (strcmp(stage->opcode, "AND") == 0) 
      {
        if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])  
        {
          cpu->stage[F].stalled=0; 
          cpu->stage[DRF].stalled=0;
          stage->rs1_value= cpu->regs[stage->rs1];
          stage->rs2_value= cpu->regs[stage->rs2];
          cpu->regs_valid[stage->rd]=0;
        }
        else
        {
          cpu->stage[F].stalled=1; 
          cpu->stage[DRF].stalled=1;
        }  
      }

      if (strcmp(stage->opcode, "OR") == 0) 
      {
        
        if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])  
        {
          cpu->stage[F].stalled=0; 
          cpu->stage[DRF].stalled=0;
          stage->rs1_value= cpu->regs[stage->rs1];
          stage->rs2_value= cpu->regs[stage->rs2];
          cpu->regs_valid[stage->rd]=0;
        }
        else
        {
          cpu->stage[F].stalled=1; 
          cpu->stage[DRF].stalled=1;
        } 
      }
      if (strcmp(stage->opcode, "XOR") == 0) 
      {
        
      if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])  // For Dependency
      {
        cpu->stage[F].stalled=0; 
        cpu->stage[DRF].stalled=0;
        stage->rs1_value= cpu->regs[stage->rs1];
        stage->rs2_value= cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd]=0;  
      }
      else
      {
        cpu->stage[F].stalled=1; 
        cpu->stage[DRF].stalled=1;
      }

    }
    if (strcmp(stage->opcode, "MUL") == 0) 
    {
      stage->math_ins++;
      if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])  // For Dependency
      {
        cpu->stage[F].stalled=0; 
        cpu->stage[DRF].stalled=0;
        stage->rs1_value= cpu->regs[stage->rs1];
        stage->rs2_value= cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd]=0; 
        
      }
      else
      {
        cpu->stage[F].stalled=1; 
        cpu->stage[DRF].stalled=1;
      }


    }
    if( strcmp(stage->opcode, "BNZ") == 0) 
      {
      
      if((cpu->stage[WB].math_ins == 1) || (cpu->stage[MEM1].math_ins == 1)||(cpu->stage[MEM2].math_ins == 1)) 
      {
        stage->stalled = 1;
      } else {
        stage->stalled = 0;
      }
    }

    if(strcmp(stage->opcode, "BZ") == 0) 
      {
      
      if((cpu->stage[WB].math_ins == 1) || (cpu->stage[MEM1].math_ins == 1)||(cpu->stage[MEM2].math_ins == 2)) 
      {
        stage->stalled = 1;
      } else {
        stage->stalled = 0;
      }
    }


      /* Copy data from decode latch to execute latch*/
    cpu->stage[EX1] = cpu->stage[DRF];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode/RF", stage);
    }
  }

   else
   {  
   if(ENABLE_DEBUG_MESSAGES)
   {
      printf("Decode/RF        : EMPTY\n");
    }
  } 

  return 0;
}

  /*
   *  Execute Stage of APEX Pipeline
   *
   *  Note : You are free to edit this function according to your
   * 				 implementation
   */
int
execute1(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX1];
  if (!stage->busy && !stage->stalled) {

    if((cpu->stage[DRF].rs1==stage->rd)||(cpu->stage[DRF].rs2==stage->rd)){
      (cpu->stage[DRF].forward_enabler)=1;

    }



      /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      stage->mem_address = (stage->rs2_value) + (stage->imm);


    }

    if (strcmp(stage->opcode, "STR") == 0) 
    {
      stage->mem_address=(stage->rs2_value)+(stage->rs1_value);
    }

      /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
     stage->buffer = stage->imm;
   }

   if (strcmp(stage->opcode, "LOAD") == 0) 
   {
    stage->mem_address = (stage->imm)+(stage->rs1_value);

  }


   if (strcmp(stage->opcode, "LDR") == 0) 
   {
    stage->mem_address = (stage->rs2_value)+(stage->rs1_value);

  }

  if (strcmp(stage->opcode, "ADD") == 0) 
  {
     stage->buffer = (stage->rs1_value) + (stage->rs2_value);
     if(stage->buffer==0)
   cpu->zero=1;
 else
   cpu->zero=0;
  
 }

if (strcmp(stage->opcode, "ADDL") == 0) 
  {
     stage->buffer = (stage->rs1_value) + (stage->imm);
     if(stage->buffer==0)
   cpu->zero=1;
 else
   cpu->zero=0;
  
 }

 if (strcmp(stage->opcode, "SUB") == 0) 
 {
  stage->buffer = (stage->rs1_value) - (stage->rs2_value);
  if(stage->buffer==0)
   cpu->zero=1;
 else
   cpu->zero=0;
    
}

if (strcmp(stage->opcode, "AND") == 0) 
{
  stage->buffer = stage->rs2_value&stage->rs1_value;
  
}

 

if (strcmp(stage->opcode, "OR") == 0) 
{
  stage->buffer = stage->rs2_value | stage->rs1_value;
 
}

if (strcmp(stage->opcode, "XOR") == 0) 
{
  stage->buffer = stage->rs2_value ^ stage->rs1_value;
  
}

if (strcmp(stage->opcode, "MUL") == 0) 
{
  stage->buffer = stage->rs1_value * stage->rs2_value;
  if(stage->buffer==0)
   cpu->zero=1;
 else
   cpu->zero=0;
  


}


if(strcmp(stage->opcode, "HALT") == 0)
     {
      
      cpu->stage[DRF].pc = 0;
       strcpy(cpu->stage[DRF].opcode, "HALT");
       cpu->stage[DRF].stalled = 1;
      cpu->stage[F].stalled = 1;
       strcpy(cpu->stage[F].opcode, "HALT");
      cpu->stage[F].pc = 0;
      
    }

    

    if (strcmp(stage->opcode, "BZ") == 0)  //*bnz
   {  
    
    if(cpu->zero==1)
    {
      
      stage->mem_address = stage->pc + stage->imm;
      
      cpu->zero=0;
  } else {
    stage->mem_address = 0;
  }
  }

 if (strcmp(stage->opcode, "BNZ") == 0)
   {  
    
    if((!cpu->zero)==1)
    {
      
      stage->mem_address = stage->pc + stage->imm;
    
      cpu->zero=0;
  } else {
    stage->mem_address = 0;
  }
  }



      /* Copy data from Execute latch to Memory latch*/
cpu->stage[EX2] = cpu->stage[EX1];

if (ENABLE_DEBUG_MESSAGES) {
  print_stage_content("Execute1", stage);
}
}

 else
  {
   cpu->stage[EX2] = cpu->stage[EX1];
  if(ENABLE_DEBUG_MESSAGES)
   {
      printf("Execute        : EMPTY\n");
    } 
  } 
  return 0;
}

int
execute2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX2];
  if(strcmp(stage->opcode, "LOAD") && strcmp(stage->opcode,"LDR") != 0) {
      cpu->regs_valid[stage->rd] = 1;
      {
          cpu->regs[stage->rd] = stage->buffer;
      }
  }
  if (!stage->busy && !stage->stalled) {
  
    

    if(strcmp(stage->opcode, "HALT") == 0)
     {
      
      cpu->stage[DRF].pc = 0;
       strcpy(cpu->stage[DRF].opcode, "HALT");
       cpu->stage[DRF].stalled = 1;
      cpu->stage[F].stalled = 1;
       strcpy(cpu->stage[F].opcode, "HALT");
      cpu->stage[F].pc = 0;
      cpu->stage[EX1].stalled = 1;
       strcpy(cpu->stage[EX1].opcode, "HALT");
      cpu->stage[EX1].pc = 0;
      
    }

    if (strcmp(stage->opcode, "JUMP") == 0) 
  { 
    cpu->pc = stage->rs1_value + stage->imm;
  }
    if (strcmp(stage->opcode, "BZ") == 0) 
    {
      if(stage->mem_address != 0) {
        cpu->pc =stage->mem_address;


      strcpy(cpu->stage[F].opcode, "flush");        
        
          
        cpu->stage[DRF].pc = cpu->stage[F].pc= 0;
         strcpy(cpu->stage[DRF].opcode, "flushflush");
         //strcpy(cpu->stage[EX2].opcode, "BZ");
         strcpy(cpu->stage[EX1].opcode, "flush");
         cpu->stage[EX1].pc = 0;
        if(stage->imm < 0) {
          cpu->ins_completed = (cpu->ins_completed + (stage->imm/4))-1;
        }

        else{
          cpu->ins_completed = (cpu->ins_completed - (stage->imm/4));
        }
        
      }
    }

      if (strcmp(stage->opcode, "BNZ") == 0) 
    {
      if(stage->mem_address != 0) {
        cpu->pc =stage->mem_address;


        strcpy(cpu->stage[F].opcode, "flush");
         strcpy(cpu->stage[DRF].opcode, "flush");
         strcpy(cpu->stage[EX1].opcode, "flush");
         //strcpy(cpu->stage[EX2].opcode, "");
          cpu->stage[DRF].pc =cpu->stage[EX1].pc=cpu->stage[F].pc   = 0;
        if(stage->imm < 0) {
          cpu->ins_completed = (cpu->ins_completed + (stage->imm/4))-1;
        }

        else{
          cpu->ins_completed = (cpu->ins_completed - (stage->imm/4));
        }
        
      }
    }


    if(cpu->stage[DRF].forward_enabler==1){
    cpu->regs_valid[stage->rd]=1;
    cpu->stage[DRF].forward_regindex=stage->rd;
    cpu->stage[DRF].forward_buffer = stage->buffer;
  //printf("\n\t----!!!!!rd ex2 %d----rd buff %d----",stage->rd,stage->buffer);

}



 




    cpu->stage[MEM1] = cpu->stage[EX2];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute2", stage);
    }
  }

  else{cpu->stage[MEM1] = cpu->stage[EX2];

    if (ENABLE_DEBUG_MESSAGES) {
      printf("Execute2        : EMPTY\n");
    }}
  return 0;
}

  /*
   *  Memory Stage of APEX Pipeline
   *
   *  Note : You are free to edit this function according to your
   * 				 implementation
   */
int
memory1(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM1];
  if (!stage->busy && !stage->stalled) {

  if((cpu->stage[DRF].rs1==stage->rd)||(cpu->stage[DRF].rs2==stage->rd)){
      (cpu->stage[DRF].forward_enabler)=1;
    }


      /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {

      cpu->data_memory[stage->mem_address] = stage->rs1_value;
        //printf("----------ADDRESSSSS-------%d---------\n",);

    }

       /* Store */
    if (strcmp(stage->opcode, "STR") == 0) {

      cpu->data_memory[stage->mem_address] = stage->buffer;
        //printf("----------ADDRESSSSS-------%d---------\n",);

    }


    if (strcmp(stage->opcode, "LOAD") == 0) 
    {
      stage->buffer= cpu->data_memory[stage->mem_address];
    }

    if (strcmp(stage->opcode, "LDR") == 0) 
    {
      stage->buffer= cpu->data_memory[stage->mem_address];
    }


    if(strcmp(stage->opcode, "HALT") == 0) 
  {   //printf("HALTTTT___");
      cpu->stage[EX1].pc = cpu->stage[EX2].pc =cpu->stage[DRF].pc = cpu->stage[F].pc =0;
      strcpy(cpu->stage[EX1].opcode, "HALT");
      strcpy(cpu->stage[EX2].opcode, "HALT");
      strcpy(cpu->stage[DRF].opcode, "HALT");
      cpu->stage[EX1].stalled=cpu->stage[EX2].stalled = cpu->stage[DRF].stalled = cpu->stage[F].stalled = 1;
       strcpy(cpu->stage[F].opcode, "HALT");
     
      
      
    }
        


   
      /* Copy data from decode latch to execute latch*/
    cpu->stage[MEM2] = cpu->stage[MEM1];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory1", stage);
    }
  }
   else
   {
   cpu->stage[MEM2] = cpu->stage[MEM1];
    if(ENABLE_DEBUG_MESSAGES)
    {
       printf("Memory1         : EMPTY\n");
    } 
  }
  
  return 0;
}

int
memory2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM2];
    if(strcmp(stage->opcode, "LOAD") && strcmp(stage->opcode,"LDR") == 0) {
        cpu->regs_valid[stage->rd] = 1;
        {
            cpu->regs[stage->rd] = stage->buffer;
        }
    }
  if (!stage->busy && !stage->stalled) {

 if(strcmp(stage->opcode, "HALT") == 0) 
  {
      cpu->stage[EX1].pc = cpu->stage[EX2].pc =cpu->stage[DRF].pc = cpu->stage[F].pc =0;
      strcpy(cpu->stage[EX1].opcode, "HALT");
      strcpy(cpu->stage[EX2].opcode, "HALT");
      strcpy(cpu->stage[DRF].opcode, "HALT");
      cpu->stage[MEM1].stalled = cpu->stage[EX1].stalled=cpu->stage[EX2].stalled = cpu->stage[DRF].stalled = cpu->stage[F].stalled = 1;
       strcpy(cpu->stage[F].opcode, "HALT");strcpy(cpu->stage[MEM1].opcode, "HALT");
     
      
      
    }

     if(cpu->stage[DRF].forward_enabler==1){

    cpu->regs_valid[stage->rd]=1;
    cpu->stage[DRF].forward_regindex=stage->rd;
    cpu->stage[DRF].forward_buffer = stage->buffer;
 // printf("\n\t--pcDRF-%s---currentPC--%s----!!!!!rd MEM2 %d----rd buff %d----",cpu->stage[DRF].opcode,stage->opcode,stage->rd,stage->buffer);
}



 

      /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM2];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory2", stage);
    }
  }

  else{cpu->stage[WB] = cpu->stage[MEM2];

    if (ENABLE_DEBUG_MESSAGES) {
      printf("Memory2         : EMPTY\n");
    }}
  return 0;
}

  /*
   *  Writeback Stage of APEX Pipeline
   *
   *  Note : You are free to edit this function according to your
   * 				 implementation
   */
int
writeback(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[WB];
  if (!stage->busy && !stage->stalled) {
    


      /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0) {

     cpu->regs[stage->rd] = stage->buffer;
     //cpu->regs_valid[stage->rd]=1;
     cpu->stage[DRF].stalled=0;
     cpu->stage[F].stalled=0;  
    // printf("\ncpu->regs[stage->rd] %d\n",cpu->regs[0]);
   }



   if (strcmp(stage->opcode, "LOAD") == 0) 
   {
    cpu->regs[stage->rd] = stage->buffer;
    //cpu->regs_valid[stage->rd]=1;
    cpu->stage[DRF].stalled=0;
    cpu->stage[F].stalled=0; 

  }

   if (strcmp(stage->opcode, "LDR") == 0) 
   {
    cpu->regs[stage->rd] = stage->buffer;
    //cpu->regs_valid[stage->rd]=1;
    cpu->stage[DRF].stalled=0;
    cpu->stage[F].stalled=0; 

  }

  if (strcmp(stage->opcode, "ADD") == 0) 
  { 
    cpu->regs[stage->rd] = stage->buffer;
    //printf("\ntseter --%d\n",stage->buffer);
    //cpu->regs_valid[stage->rd]=1;
    cpu->stage[DRF].stalled=0;
    cpu->stage[F].stalled=0; 
     if(stage->buffer==0)
     cpu->zero=1;
   else
     cpu->zero=0;   
  }

  if (strcmp(stage->opcode, "ADDL") == 0) 
  { 
    cpu->regs[stage->rd] = stage->buffer;
    //printf("\ntseter --%d\n",stage->buffer);
    //cpu->regs_valid[stage->rd]=1;
    cpu->stage[DRF].stalled=0;
    cpu->stage[F].stalled=0; 
     if(stage->buffer==0)
     cpu->zero=1;
   else
     cpu->zero=0;   
  }

  if (strcmp(stage->opcode, "SUB") == 0) 
  {
   cpu->regs[stage->rd] = stage->buffer;
   //cpu->regs_valid[stage->rd]=1;
   cpu->stage[DRF].stalled=0;
   cpu->stage[F].stalled=0; 
    if(stage->buffer==0)
     cpu->zero=1;
   else
     cpu->zero=0;   
 }

 if (strcmp(stage->opcode, "AND") == 0) 
 {

  //cpu->regs_valid[stage->rd]=1;
  cpu->stage[DRF].stalled=0;
  cpu->stage[F].stalled=0; 
}

if (strcmp(stage->opcode, "OR") == 0) 
{

  //cpu->regs_valid[stage->rd]=1;
  cpu->stage[DRF].stalled=0;
  cpu->stage[F].stalled=0; 
}

if (strcmp(stage->opcode, "XOR") == 0) 
{
  //cpu->regs_valid[stage->rd]=1;
  cpu->stage[DRF].stalled=0;
  cpu->stage[F].stalled=0; 
}

if (strcmp(stage->opcode, "MUL") == 0) 
{
  cpu->regs[stage->rd] = stage->buffer;
  //cpu->regs_valid[stage->rd]=1;
  cpu->stage[DRF].stalled=0;
  cpu->stage[F].stalled=0; 
   if(stage->buffer==0)
     cpu->zero=1;
   else
     cpu->zero=0;   
}

if(strcmp(stage->opcode, "HALT") == 0) {
        cpu->ins_completed = cpu->code_memory_size - 1;
        cpu->stage[F].pc =cpu->stage[MEM1].pc =cpu->stage[MEM2].pc =cpu->stage[EX1].pc=cpu->stage[EX2].pc=cpu->stage[DRF].pc =  0;
        strcpy(cpu->stage[EX1].opcode, "HALT");
        strcpy(cpu->stage[EX2].opcode, "HALT");        
        
        strcpy(cpu->stage[DRF].opcode, "HALT");
        cpu->stage[EX2].stalled =cpu->stage[EX1].stalled = 1;
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
         strcpy(cpu->stage[F].opcode, "HALT");
         cpu->ins_completed = cpu->code_memory_size - 1;
        strcpy(cpu->stage[MEM2].opcode, "HALT");
        strcpy(cpu->stage[MEM1].opcode, "HALT");
        cpu->stage[MEM2].stalled =cpu->stage[MEM1].stalled = 1;
        }




cpu->ins_completed++;

if (ENABLE_DEBUG_MESSAGES) {
  print_stage_content("Writeback", stage);
}
}
else{
  printf("Writeback      : EMPTY\n");
}

return 0;
}

  /*
   *  APEX CPU simulation loop
   *
   *  Note : You are free to edit this function according to your
   * 				 implementation
   */
  


  int
  APEX_cpu_run(APEX_CPU *cpu, const char* type, const char* req_cyc)
  {
     int need_cyc = atoi(req_cyc);
  





    while (1) {
     
      //int need_cyc=req_cyc;

      /* All the instructions committed, so exit */
      if ((cpu->ins_completed == cpu->code_memory_size)) {
        printf("(apex) >> Simulation Complete");
        break;
      }

      if (ENABLE_DEBUG_MESSAGES) {
        printf("--------------------------------\n");
        printf("Clock Cycle #: %d\n", cpu->clock+1);
        printf("--------------------------------\n");
      }



      writeback(cpu);
      memory2(cpu);
      memory1(cpu);
      execute2(cpu);
      execute1(cpu);
      decode(cpu);
      fetch(cpu);
      cpu->clock++;
      //if((cpu->clock == need_cyc)){
      //  break;
      
    }

  printf("\n");
    printf("\n----+++Register Value+++----\n");
    for(int i=0;i<16;i++)
      {printf("\n");
    printf("Register[%d] >> Value=%d >> status=%s \n",i,cpu->regs[i],(cpu->regs_valid[i])?"Valid" : "Invalid");

  }

  
  printf("----+++DATA MEMORY+++----\n");

  for(int i=0;i<101;i++)
  {
    printf(" DATA_MEM[%d] :- Value=%d \n",i,cpu->data_memory[i]);
  }


  return 0;
}