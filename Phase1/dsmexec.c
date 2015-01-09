#include "common_impl.h"

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL;
int pipe_exit[2];

/* le nombre de processus effectivement crees */
volatile int num_procs = 0;

void usage(void)
{
	fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
	fflush(stdout);
	exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)
{
	 /* on traite les fils qui se terminent */
	 /* pour eviter les zombies */
	wait(NULL);
	num_procs--;
	write(pipe_exit[1], 0, 1);
}


int main(int argc, char *argv[])
{
	if (argc < 3){
		usage();
	} else {

		pid_t pid;

		int num_procs_creat = 0;

		struct sigaction siga;

		FILE* f;

		char buffer[1024]; // Buffer pour lire un fichier et les pipes
		char buffer2[256]; // Buffer pour l'id
		char buffer3[10]; // Buffer pour le port
		char* array[1024]; // Pour enregistrer les noms de machines
		char* line = NULL;

		int i = 0; // int pour les boucles for
		int r;

		int DSM_NODE_ID;

		// Pour le poll
		struct pollfd* fds;

		int* pipe_out; // Tableau de fd de pipe
		int* pipe_err; // Tableau de fd de pipe
		int fd_out[2];
		int fd_err[2];

		size_t len;
		ssize_t size;

		int port;
		int sock;
		int client;
		socklen_t s_len;

		struct sockaddr_in client_addr_in;

		int* clients;
		int* ports;

		pipe(pipe_exit);

		/* Mise en place d'un traitant pour recuperer les fils zombies*/
		memset(&siga, 0, sizeof(struct sigaction));
		siga.sa_handler = sigchld_handler;
		siga.sa_flags = SA_RESTART;
		sigaction(17, &siga, NULL);
	

		/* lecture du fichier de machines */
		/* 1- on recupere le nombre de processus a lancer */
		/* 2- on recupere les noms des machines : le nom de */
		/* la machine est un des elements d'identification */

		memset(array, 0, 1024*sizeof(char*));
		
		f = fopen(argv[1], "r");

		if(f == NULL)
		{
			printf("Couldn't open %s\n", argv[1]);
			exit(0);
		}

		while( (size = getline(&line, &len, f)) != -1 )
		{
			array[i] = malloc(len*sizeof(char)+1);
			if(line[strlen(line)-1] == '\n')
				memcpy(array[i], line, strlen(line)-1);
			else
				memcpy(array[i], line, strlen(line));
			i++;
		}

		num_procs = i;

		pipe_out = malloc(num_procs*sizeof(int));
		pipe_err = malloc(num_procs*sizeof(int));
		 
		/* creation de la socket d'ecoute */
		/* + ecoute effective */ 
		sock = creer_socket(SOCK_STREAM, &port);
		r = listen(sock, num_procs);
		if(r == -1)
			perror("ERROR with listen in dsmexec");
		
		/* creation des fils */
		for(i = 0; i < num_procs ; i++) {
	
			/* creation du tube pour rediriger stdout */
			pipe(fd_out);

			/* creation du tube pour rediriger stderr */
			pipe(fd_err);

			pid = fork();
			if(pid == -1) {
				ERROR_EXIT("fork");
			}
			else if (pid == 0) {
				// FILS

				/* redirection stdout */	      
				dup2(fd_out[1], 1);

				/* redirection stderr */	      	      
				dup2(fd_err[1], 2);

				close(fd_out[0]);
				close(fd_out[1]);
				close(fd_err[0]);
				close(fd_err[1]);
				
				/* Creation du tableau d'arguments pour le ssh */
				memset(buffer, 0, 1024);
				strcpy(buffer, array[i]); // Buffer prend le nom de la machine
				DSM_NODE_ID = i;
				for(i = 0; i < DSM_NODE_ID; i++) {
					if(array[i] != NULL) {
						free(array[i]);
						array[i] = NULL;
					}
				}

				memset(array, 0 , 1024);


				array[0] = "ssh";
				array[1] = buffer;
				array[2] = "-oStrictHostKeyChecking=no";
				array[3] = "~/ShareMem/Phase1/bin/dsmwrap";
				sprintf(buffer2, "%d", i);
				array[4] = buffer2;
				array[5] = getenv("HOSTNAME");
				sprintf(buffer3, "%d", port);
				array[6] = buffer3;
				for(i = 2; i < argc; i++)
					array[i+5] = argv[i];

				/* jump to new prog : */
				execvp(array[0], array);

			}
			else  if(pid > 0) {
				// PERE

				pipe_out[i] = fd_out[0];

				pipe_err[i] = fd_err[0];

				close(fd_out[1]);
				close(fd_err[1]);

				num_procs_creat++;	      
			}
		}

		num_procs = num_procs_creat;

		fds = malloc((2*num_procs_creat + 1) * sizeof(*fds));

		for(i = 0; i < 2*num_procs_creat; i++) {
			if(i < num_procs_creat) {
				fds[i].fd = pipe_out[i];
				fds[i].events = POLLIN;
			}
			else {
				fds[i].fd = pipe_err[i-num_procs_creat];
				fds[i].events = POLLIN;
			}
		}
		fds[i].fd = pipe_exit[0];
		fds[i].events = POLLIN;

		clients = malloc(num_procs_creat * sizeof(int));
		ports = malloc(num_procs_creat * sizeof(int));
		memset(clients, -1, num_procs_creat);
		memset(ports, -1, num_procs_creat);

		for(i = 0; i < num_procs_creat ; i++){
			//len = sizeof(struct sockaddr);
			//printf("allo? %d\n", i);
			fflush(stdout);
			memset(&client_addr_in, 0, sizeof(struct sockaddr_in));
			s_len = 0;

			/* on accepte les connexions des processus dsm */
			client = accept(sock, (struct sockaddr*) &client_addr_in, &s_len);
			if(client == -1)
				perror("ERROR with accept() in dsmexec");

			r = read(client, buffer, 1024);
			if(r == -1) {
				perror("ERROR with read() in dsmexec");
			}

			r = 0;
			while(buffer[r] != '\n' && buffer[r] != '0' && r < 1024)
				r++;

			/* On recupere le DSM_NODE_ID */
			DSM_NODE_ID = atoi(buffer);

			if(DSM_NODE_ID >= 0 && DSM_NODE_ID < num_procs_creat) {

				clients[DSM_NODE_ID] = client;
				/* On recupere le port d'ecoute */
				ports[DSM_NODE_ID] = atoi(buffer+r+1);
			}

			//printf("dsmexec: DSM_NODE_ID : %d, PORT : %d\n", DSM_NODE_ID, ports[DSM_NODE_ID]);
		}

		// Envoie des données aux proc distants
		for(i = 0; i < num_procs_creat ; i++) {
		  //printf("dsmexec check i: %d\n", i);
			if(clients[i] != -1) {
			  //printf("dsmexec check2 i: %d\n", i);
				memset(buffer, 0, 1024);

				/* envoi du nombre de processus aux processus dsm et son ID*/
				sprintf(buffer, "%d\n%d", i, num_procs_creat);
				do_write(clients[i], buffer, 1024);
				//printf("dsmexec check3 i: %d\n", i);
				/* envoi des infos de connexion aux processus */
				for(r = 0; r < i; r++) {
					memset(buffer, 0, 1024);
					//printf("dsmexec: boucle d'envoie: r: %d,i: %d\n", r,i);
					sprintf(buffer, "%d\n%s\n%d", r, array[r], ports[r]);

					do_write(clients[i], buffer, 1024);
				}
				close(clients[i]);
			}
		}
		free(clients);
		free(ports);
		 
		/* gestion des E/S : on recupere les caracteres */
		/* sur les tubes de redirection de stdout/stderr */

		while(num_procs)
		{
			/* Checks if there is data waiting in stdin */
			poll(fds, 2*num_procs_creat+1, -1);
//printf("num: %d\n", num_procs_creat);
//fflush(stdout);
			if(fds[2*num_procs_creat].revents == POLLIN)
				break;

			for(i=0; i<num_procs_creat; i++)
			{
			    if(fds[i].revents == POLLIN)
				{
			    	memset(buffer, 0, sizeof(char)*1024);
					r = read(pipe_out[i], buffer, sizeof(char)*1024);
					if(r == 0) {
						close(fds[i].fd);
						pipe_out[i] = 0;
						fds[i].fd = 0;
					}
					else {
						printf("[Proc %d : %s : stdout] %s", i, array[i], buffer);
						fflush(stdout);
					}
				}
			}

			for(i=num_procs_creat; i<2*num_procs_creat; i++)
			{
				if(fds[i].revents == POLLIN)
				{
					memset(buffer, 0, sizeof(char)*1024);
					r = read(pipe_err[i-num_procs], buffer, sizeof(char)*1024);
					if(r == 0) {
						close(fds[i].fd);
						pipe_err[i] = 0;
						fds[i].fd = 0;
					}
					else {
						printf("[Proc %d : %s : stderr] %s", i-num_procs, array[i-num_procs], buffer);
						fflush(stdout);
					}
				}
			}
		}
		printf("Sortie du while\n");
		 /* on ferme la socket d'ecoute */
		close(sock);

		free(pipe_out);
		free(pipe_err);
		free(fds);
		for(i = 0; i < num_procs_creat; i++)
			free(array[i]);
	}   
	 exit(EXIT_SUCCESS);  
}

