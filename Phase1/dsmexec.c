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
		struct sigaction siga;
		int num_procs = 0;
		int i = 0, ret;
		FILE* f;
		char* array[1024];
		int** pipe_out;
		int** pipe_err;
		fd_set readfs;
		char* line = NULL;
		size_t len;
		ssize_t size;
		char buffer[1024];

		FD_ZERO(&readfs);

		/* Mise en place d'un traitant pour recuperer les fils zombies*/      
		siga.sa_handler = sigchld_handler;
	

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

		pipe_out = malloc(num_procs*sizeof(int*));
		pipe_err = malloc(num_procs*sizeof(int*));
		 
		/* creation de la socket d'ecoute */
		/* + ecoute effective */ 
		
		/* creation des fils */
		for(i = 0; i < num_procs ; i++) {
	
			pipe_out[i] = malloc(2*sizeof(int));
			pipe_err[i] = malloc(2*sizeof(int));

			/* creation du tube pour rediriger stdout */
			pipe(pipe_out[i]);

			/* creation du tube pour rediriger stderr */
			pipe(pipe_err[i]);
			
			pid = fork();
			if(pid == -1) ERROR_EXIT("fork");
			
			if (pid == 0) { /* fils */	 
				/* redirection stdout */	      
				dup2(pipe_out[i][1], 1);

				/* redirection stderr */	      	      
				dup2(pipe_err[i][1], 2);
				 
				close(pipe_out[i][0]);
				close(pipe_err[i][0]);
				
				/* Creation du tableau d'arguments pour le ssh */ 
				
				while(1)
					write(1, "Coucou\n", 8);
				exit(0);

				/* jump to new prog : */
				/* execvp("ssh",newargv); */

			} else  if(pid > 0) { /* pere */		      
				/* fermeture des extremites des tubes non utiles */
				close(pipe_out[i][1]);
				close(pipe_err[i][1]);
		
				FD_SET(pipe_out[i][0], &readfs);
				FD_SET(pipe_err[i][0], &readfs);

				num_procs_creat++;	      
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
			if( (ret = select(pipe_err[num_procs_creat-1][0]+1, &readfs, NULL, NULL, NULL)) < 0 )
			{

			}
			else if(ret)
			{
				for(i=0; i<num_procs; i++)
				{
					if(FD_ISSET(pipe_out[i][0], &readfs))
					{
						memset(buffer, 0, sizeof(char)*1024);
						recv(pipe_out[i][0], buffer, sizeof(char)*1024, 0);
						printf("[Proc %d : toto : stdout] %s !\n", i, buffer);
					}
					//FD_ZERO(&readfs);
					//FD_SET(pipe_out[i][0], &readfs);
				}

				for(i=0; i<num_procs; i++)
				{
					if(FD_ISSET(pipe_err[i][0], &readfs))
					{
						memset(buffer, 0, sizeof(char)*1024);
						recv(pipe_err[i][0], buffer, sizeof(char)*1024, 0);
						printf("[Proc %d : toto : stderr] %s !\n", i, buffer);
					}
					//FD_ZERO(&readfs);
					//FD_ISSET(pipe_err[i][0], &readfs);
				}
			}
		}

		 /* on attend les processus fils */
		 
		 /* on ferme les descripteurs proprement */
		 
		 /* on ferme la socket d'ecoute */
	}   
	 exit(EXIT_SUCCESS);  
}

