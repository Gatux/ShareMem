#include "common_impl.h"

int main(int argc, char **argv)
{   
	/* processus intermediaire pour "nettoyer" */
	/* la liste des arguments qu'on va passer */
	/* a la commande a executer vraiment */
   
	char* array[1024];
	char buffer[1024];

	int i;

	int sock_dsmexec;
	int sock_l;
	int port_dsm;
	int port_l;

	int DSM_NODE_ID = atoi(argv[1]);

	struct sockaddr_in serv_info;

	printf("dsmwrap : DSM_NODE_ID = %d\n", DSM_NODE_ID);
	printf("dsmwrap : SWAG_ENV = %s\n", argv[2]);
	printf("dsmwrap : SWAG_PORT = %s\n", argv[3]);
	fflush(stdout);

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */

	get_addr_info(&serv_info, argv[2], argv[3]);
	sock_dsmexec = creer_socket(SOCK_STREAM, &port_dsm);
	do_connect(sock_dsmexec, &serv_info, sizeof(serv_info));

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */
	sock_l = creer_socket(SOCK_STREAM, &port_l);

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm et envoie de DSM_NODE_ID */
	memset(buffer, 0, 1024);
	sprintf(buffer, "%d\n%d", DSM_NODE_ID, port_l);
	do_write(sock_dsmexec, buffer, strlen(buffer));

	/* on execute la bonne commande */
	memset(array, 0, 1024);
	memset(buffer, 0, 1024);

	sprintf(buffer, "%d", sock_dsmexec);
	setenv("SOCK_DSMEXEC", buffer, 1);

	memset(buffer, 0, 1024);

	sprintf(buffer, "%d", sock_l);
	setenv("SOCK_L", buffer, 1);

	for(i = 4; i < argc; i++)
			array[i-4] = argv[i];

	execvp(array[0], array);
	return 0;
}
