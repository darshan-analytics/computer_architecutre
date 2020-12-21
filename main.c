/*
 *  main.c
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"

// ./apex_sim input_g.asm display 20

int
main(int argc, char const* argv[])
{
  if (argc < 2) {
    fprintf(stderr, "APEX_Help : Usage %s <input_file>\n", argv[0]);
    exit(1);
  }

  APEX_CPU* cpu = APEX_cpu_init(argv[1]);
  if (!cpu) {
    fprintf(stderr, "APEX_Error : Unable to initialize CPU\n");
    exit(1);
  }

  const char *type;
  const char *req_cyc;
  type=argv[2];req_cyc=argv[3];


  APEX_cpu_run(cpu,type,req_cyc);
  APEX_cpu_stop(cpu);
  return 0;
}


//references in readme.txt