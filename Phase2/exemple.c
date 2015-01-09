#include "dsm.h"

int main(int argc, char **argv)
{
  char *pointer; 
  char *current;
  int value;

  printf("Programme exemple\n");

  pointer = dsm_init(argc,argv);
  current = pointer;

  printf("[%i] Coucou, mon adresse de base est : %p\n", DSM_NODE_ID, pointer);
  fflush(stdout);

  // Teste d'un segfault standard :
  //int* ptr = 0x1;
  //*ptr = 0;

  if (DSM_NODE_ID == 0)
  {
    current += 4*sizeof(int);
    value = *((int *)current);
    printf("[%i] valeur de l'entier : %i\n", DSM_NODE_ID, value);
    fflush(stdout);
  } 
  else if (DSM_NODE_ID == 1)
  {
    //current += PAGE_SIZE;
    current += 16*sizeof(int);
    value = *((int *)current);
    printf("[%i] valeur de l'entier : %i\n", DSM_NODE_ID, value);
    fflush(stdout);
  }
  dsm_finalize();
  return 0;
}
