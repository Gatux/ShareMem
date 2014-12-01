#include "common_impl.h"

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL;

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

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
}


int main(int argc, char *argv[])
{
	if (argc < 3){
		usage();
	} else {

		pid_t pid;

		int num_procs = 0;

		struct sigaction siga;

		FILE* f;

		char buffer[1024]; // Buffer pour lire un fichier et les pipes
		char* array[1024]; // Pour enregistrer les noms de machines
		char* line = NULL;

		int i = 0; // int pour les boucles for

		int DSM_NODE_ID;

		// Pour le poll
		struct pollfd* fds;

		int* pipe_out; // Tableau de fd de pipe
		int* pipe_err; // Tableau de fd de pipe
		int fd_out[2];
		int fd_err[2];

		size_t len;
		ssize_t size;

		/* Mise en place d'un traitant pour recuperer les fils zombies*/
		memset(&siga, 0, sizeof(struct sigaction));
		siga.sa_handler = sigchld_handler;
		//sigaction(10, &siga, NULL);
	

		/* lecture du fichier de machines */
		/* 1- on recupere le nombre de processus a lancer */
		/* 2- on recupere les noms des machines : le nom de */
		/* la machine est un des elements d'identification */

		/* TO FREE */

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
			strcpy(array[i], line);
			i++;
		}

		num_procs = i;

		pipe_out = malloc(num_procs*sizeof(int));
		pipe_err = malloc(num_procs*sizeof(int));
		 
		/* creation de la socket d'ecoute */
		/* + ecoute effective */ 
		
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
				close(fd_out[0]);
				
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

				array[0] = buffer;
				array[1] = "-oStrictHostKeyChecking=no";
				array[2] = "~/ShareMem/Phase1/truc";

				/* jump to new prog : */
				execvp("ssh",array);

			}
			else  if(pid > 0) {
				// PERE

				pipe_out[i] = fd_out[0];

				pipe_err[i] = fd_err[0];

				close(fd_out[1]);
				close(fd_out[1]);

				num_procs_creat++;	      
			}
		 }


		fds = malloc(2*num_procs_creat * sizeof(*fds));

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

		 for(i = 0; i < num_procs ; i++){
	
	/* on accepte les connexions des processus dsm */
	
	/*  On recupere le nom de la machine distante */
	/* 1- d'abord la taille de la chaine */
	/* 2- puis la chaine elle-meme */
	
	/* On recupere le pid du processus distant  */
	
	/* On recupere le numero de port de la socket */
	/* d'ecoute des processus distants */
		 }
		 
		 /* envoi du nombre de processus aux processus dsm*/
		 
		 /* envoi des rangs aux processus dsm */
		 
		 /* envoi des infos de connexion aux processus */
		 
		 /* gestion des E/S : on recupere les caracteres */
		 /* sur les tubes de redirection de stdout/stderr */     
		 /* while(1)
				 {
						je recupere les infos sur les tubes de redirection
						jusqu'à ce qu'ils soient inactifs (ie fermes par les
						processus dsm ecrivains de l'autre cote ...)
			 
				 };
			*/

		while(1)
		{
		    /* Checks if there is data waiting in stdin */
		    poll(fds, 2*num_procs_creat, -1);

			for(i=0; i<num_procs_creat; i++)
			{
			    if(fds[i].revents == POLLIN)
				{
			    	memset(buffer, 0, sizeof(char)*1024);
					read(pipe_out[i], buffer, sizeof(char)*1024);
					printf("[Proc %d : toto : stdout] %s", i, buffer);
					fflush(stdout);
				}
			}

			for(i=num_procs_creat; i<2*num_procs_creat; i++)
			{
				if(fds[i].revents == POLLIN)
				{
					memset(buffer, 0, sizeof(char)*1024);
					read(pipe_err[i-num_procs_creat], buffer, sizeof(char)*1024);
					printf("[Proc %d : toto : stderr] %s", i, buffer);
					fflush(stdout);
				}
			}
		}

		 /* on attend les processus fils */
		 
		 /* on ferme les descripteurs proprement */
		 
		 /* on ferme la socket d'ecoute */
	}   
	 exit(EXIT_SUCCESS);  
}

