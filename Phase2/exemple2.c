#include "dsm.h"

int main(int argc, char **argv)
{
  char *pointer; 
  char *current;
  int value;

  printf("Programme exemple2\n");

  pointer = dsm_init(argc,argv);
  current = pointer;

  printf("[%i] Coucou, mon adresse de base est : %p\n", DSM_NODE_ID, pointer);
  fflush(stdout);

  if (DSM_NODE_ID == 0)
  {
    sprintf(current, "voici le code secret: 1921");
    printf("[%i] Chaine contenue dans la page 0 : %s\n", DSM_NODE_ID, current);
    fflush(stdout);
  } 
  else if (DSM_NODE_ID == 1)
  {
    printf("[%i] Chaine contenue dans la page 0 : %s\n", DSM_NODE_ID, current);
    fflush(stdout);
  }
  printf("[%d] Fin du programme\n", DSM_NODE_ID);
  fflush(stdout);

  dsm_finalize();

  printf("[%d] Fin de dsm_finalize\n", DSM_NODE_ID);
  fflush(stdout);
  return 0;
}
