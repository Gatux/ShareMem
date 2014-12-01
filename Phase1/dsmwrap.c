#include "common_impl.h"

int main(int argc, char **argv)
{   
	/* processus intermediaire pour "nettoyer" */
	/* la liste des arguments qu'on va passer */
	/* a la commande a executer vraiment */
   
	char* array[1024];

	int i;

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */   
   
   /* Envoi du nom de machine au lanceur */

   /* Envoi du pid au lanceur */

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */

   /* on execute la bonne commande */
	memset(array, 0, 1024);

	for(i = 1; i < argc; i++)
		array[i-1] = argv[i];

	execvp(array[0], array);
   return 0;
}
