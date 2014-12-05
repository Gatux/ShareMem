#include "common_impl.h"

int main(int argc, char **argv)
{   
	/* processus intermediaire pour "nettoyer" */
	/* la liste des arguments qu'on va passer */
	/* a la commande a executer vraiment */
   

	char* array[1024];

	int i;
	int sock;
	int port;
	int DSM_NODE_ID = atoi(argv[1]);
	struct sockaddr_in serv_info;

	char buffer[1024];

	printf("dsmwrap : DSM_NODE_ID = %d\n", DSM_NODE_ID);
	printf("dsmwrap : SWAG_ENV = %s\n", argv[2]);
	printf("dsmwrap : SWAG_PORT = %s\n", argv[3]);
	fflush(stdout);

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */

   	get_addr_info(&serv_info, argv[2], argv[3]);
	sock = creer_socket(SOCK_STREAM, &port);
	do_connect(sock, &serv_info, sizeof(serv_info));
   
	/* Envoie de DSM_NODE_ID */
	memset(buffer, 0, 1024);
	sprintf("%d\n", DSM_NODE_ID);
	write(sock, buffer, strlen(buffer));

   /* Envoi du nom de machine au lanceur */

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */

   /* on execute la bonne commande */
	memset(array, 0, 1024);

	for(i = 4; i < argc; i++)
		array[i-4] = argv[i];

	execvp(array[0], array);
   return 0;
}
