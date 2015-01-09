#include "dsm.h"

int DSM_NODE_NUM; /* nombre de processus dsm */
int DSM_NODE_ID;  /* rang (= numero) du processus */
int* fd_procs_dist;

/* indique l'adresse de debut de la page de numero numpage */
static char *num2address( int numpage )
{ 
   char *pointer = (char *)(BASE_ADDR+(numpage*(PAGE_SIZE)));
   
   if( pointer >= (char *)TOP_ADDR ){
	  fprintf(stderr,"[%i] Invalid address !\n", DSM_NODE_ID);
	  return NULL;
   }
   else return pointer;
}

/* indique l'adresse de debut de la page de numero numpage */
static int address2num( void* addr )
{
   long int page = (long int)(addr - BASE_ADDR) / PAGE_SIZE;

   if( page < 0 || page >= PAGE_NUMBER ){
	  fprintf(stderr,"[%i] Invalid address !\n", DSM_NODE_ID);
	  return -1;
   }
   else return (int)page;
}

/* fonctions pouvant etre utiles */
static void dsm_change_info( int numpage, dsm_page_state_t state, dsm_page_owner_t owner)
{
	if ((numpage >= 0) && (numpage < PAGE_NUMBER)) {	
		if (state != NO_CHANGE )
			table_page[numpage].status = state;
		if (owner >= 0 )
			table_page[numpage].owner = owner;
	return;
	}
	else {
		fprintf(stderr,"[%i] Invalid page number !\n", DSM_NODE_ID);
		return;
	}
}

static dsm_page_owner_t get_owner( int numpage)
{
   return table_page[numpage].owner;
}

static dsm_page_state_t get_status( int numpage)
{
   return table_page[numpage].status;
}

/* Allocation d'une nouvelle page */
static void dsm_alloc_page( int numpage )
{
   char *page_addr = num2address( numpage );
   mmap(page_addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   return ;
}

/* Changement de la protection d'une page */
static void dsm_protect_page( int numpage , int prot)
{
   char *page_addr = num2address( numpage );
   mprotect(page_addr, PAGE_SIZE, prot);
   return;
}

static void dsm_free_page( int numpage )
{
   char *page_addr = num2address( numpage );
   munmap(page_addr, PAGE_SIZE);
   return;
}

static int dsm_send_page(dsm_page_owner_t new_owner, int numpage)
{
	// Envoie de la page
	do_write(fd_procs_dist[new_owner], num2address(numpage), PAGE_SIZE);
	dsm_change_info(numpage, WRITE, new_owner);
	dsm_free_page(numpage);
}

static int dsm_get_page(int numpage)
{
	//printf("[%d] dsm_get_page1: numpage: %d\n",DSM_NODE_ID, numpage);
	//fflush(stdout);

	char buffer[1024];
	dsm_page_owner_t owner;
	int i, e;
	long int addr, r;

	// il faut trouver celui qui possède la page
	owner = get_owner(numpage);

	//printf("[%d] dsm_get_page2: owner: %d\n",DSM_NODE_ID, owner);
	//fflush(stdout);

	// si en effet la page ne nous appartient pas
	if( owner != DSM_NODE_ID) {

		// on lui dit qu'on veut la page numpage
		memset(buffer, 0, 1024);
		sprintf(buffer, "askPage\n%d", numpage);
		do_write(fd_procs_dist[numpage], buffer, strlen(buffer) + 1);

		//printf("[%d] dsm_get_page3: do_write OK, buffer: %s\n",DSM_NODE_ID, buffer);
		//fflush(stdout);

		// on la récupère
		dsm_change_info(numpage, WRITE, DSM_NODE_ID);
		dsm_alloc_page(numpage);
		memset(buffer, 0, 1024);
		r = PAGE_SIZE;
		addr = (long int)num2address(numpage);

		do {
			//printf("[%d] dsm_get_page: pre_do_read r: %d\n",DSM_NODE_ID, r);
			//fflush(stdout);

			e = do_read(fd_procs_dist[owner], (char*) (addr + (PAGE_SIZE - r)), PAGE_SIZE);

			if(e == -1)
				fprintf(stderr, "Error : read = -1\n");
			else r -= (long int) e;

			//printf("[%d] dsm_get_page: post_do_read : %d\n",DSM_NODE_ID, r);
			//fflush(stdout);
		} while(r > 0);

		printf("[%d] Page %d récupérée !\n",DSM_NODE_ID, numpage);
		fflush(stdout);

		// 5: on notifie tout le monde (sauf lui et moi) de ce changement
		for(i = 0; i < DSM_NODE_NUM; i++) {
			if(i != DSM_NODE_ID && i != owner) {
				memset(buffer, 0, 1024);
				sprintf(buffer, "%d\n%d", numpage, DSM_NODE_ID);
				do_write(fd_procs_dist[i], buffer, strlen(buffer) + 1);
			}
		}
	}
}

static void *dsm_comm_daemon( void *arg)
{
	struct pollfd fds[DSM_NODE_NUM];
	int i;
	char buffer[1024];
	int numpage;
	dsm_page_owner_t owner;

	memset(buffer, 0, 1024);

	// Préparation du tableau de structure pour le poll
	for(i = 0; i < DSM_NODE_NUM; i++) {
		if(i == DSM_NODE_ID) {
			fds[i].fd = 0;
			fds[i].events = POLLIN;
		}
		else {
			fds[i].fd = fd_procs_dist[i];
			fds[i].events = POLLIN;
		}
	}

	printf("[%i] Waiting for incoming reqs \n", DSM_NODE_ID);
	fflush(stdout);
	
	while(1) {
		poll(fds, DSM_NODE_NUM, -1);
		for(i = 0; i < DSM_NODE_NUM; i++) {
			//printf("[%d] dsm_comm_daemon i: %d\n",DSM_NODE_ID, i);
			//fflush(stdout);

			if(fds[i].revents == POLLIN) {
				// On regarde si c'est une demande de page ou une update des propriétaires de pages 
				do_read(fd_procs_dist[i], buffer, 1024);

				change_buffer(buffer, strlen(buffer));

				if(strncmp("askPage", buffer, 7) == 0) { // cas d'une demande d'une page
					numpage = atoi(buffer+strlen(buffer)+1);
					if(numpage >= 0 && numpage < PAGE_NUMBER) {

						if(get_owner(numpage) != DSM_NODE_ID)
							dsm_get_page(numpage);

						dsm_send_page(i, numpage);

						printf("[%d] Page %d envoyée\n",DSM_NODE_ID, numpage);
						fflush(stdout);
					}
				}
				else { // cas d'une mise à jour du propriétaire d'une page
					numpage = atoi(buffer);
					owner = atoi(buffer+strlen(buffer)+1);
					if(numpage >= 0 && numpage < PAGE_NUMBER && owner >= 0 && owner < DSM_NODE_NUM)
						dsm_change_info(numpage, WRITE, owner);
					else
						fprintf(stderr, "Error: mise à jour du propriétaire: numpage=%d, owner=%d, DSM_NODE_ID=%d\n", numpage, owner, DSM_NODE_ID);
				}
			}
		}
	}
	return NULL;
}

static void dsm_handler( void* addr )
{  
	int numpage;

	printf("[%i] FAULTY  ACCESS !!! \n",DSM_NODE_ID);
	fflush(stdout);

	// On cherche le numero de la page lié à l'adresse
	numpage = address2num(addr);

	if(numpage != -1)
			dsm_get_page(numpage);
}

/* traitant de signal adequat */
static void segv_handler(int sig, siginfo_t *info, void *context)
{
   /* A completer */
   /* adresse qui a provoque une erreur */
   void  *addr = info->si_addr;   
  /* Si ceci ne fonctionne pas, utiliser a la place :*/
  /*
   #ifdef __x86_64__
   void *addr = (void *)(context->uc_mcontext.gregs[REG_CR2]);
   #elif __i386__
   void *addr = (void *)(context->uc_mcontext.cr2);
   #else
   void  addr = info->si_addr;
   #endif
   */
   /*
   pour plus tard (question ++):
   dsm_access_t access  = (((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR] & 2) ? WRITE_ACCESS : READ_ACCESS;   
  */   
   /* adresse de la page dont fait partie l'adresse qui a provoque la faute */
   void  *page_addr  = (void *)(((unsigned long) addr) & ~(PAGE_SIZE-1));

   if ((addr >= (void *)BASE_ADDR) && (addr < (void *)TOP_ADDR))
	 {
	dsm_handler(addr);
	 }
   else
	 {
	/* SIGSEGV normal : ne rien faire*/
	   fprintf(stderr, "Normal segfault\n");
	   abort();
	 }
}

/* Seules ces deux dernieres fonctions sont visibles et utilisables */
/* dans les programmes utilisateurs de la DSM                       */
char *dsm_init(int argc, char **argv)
{   
	struct sigaction act;

	int index;
	int sock_dsmexec;
	int sock_l;
	int r;
	int i;
	int id;
	int fd;

	struct sockaddr_in serv_info;
	int port;

	socklen_t s_len;
	struct sockaddr_in client_addr_in;

	char buffer[1024];

	// On récupère les numéro de fd des socket que dsmwrap a créé
	sock_dsmexec = atoi(getenv("SOCK_DSMEXEC"));
	sock_l = atoi(getenv("SOCK_L"));

	// On lit la socket, dsmexec doit nous envoyer des choses
	memset(buffer, 0, 1024);
	r = read(sock_dsmexec, buffer, 1024);
	if(r == -1)
		perror("ERROR with read() in dsm_init");

	// Les informations sont envoyée avec comme séparateur '\n', cette fonction les tranforme en '\0'
	change_buffer(buffer, 1024);

	/* reception de mon numero de processus dsm envoye */
	/* par le lanceur de programmes (DSM_NODE_ID)*/
	DSM_NODE_ID = atoi(buffer);

	/* reception du nombre de processus dsm envoye */
	/* par le lanceur de programmes (DSM_NODE_NUM)*/
	DSM_NODE_NUM = atoi(buffer+strlen(buffer)+1);

	i = listen(sock_l, DSM_NODE_NUM);
	if(i == -1)
		perror("ERROR with listen in dsm_init()");
   
	fd_procs_dist = calloc(DSM_NODE_NUM, sizeof(int));
	
	for(i = 0; i < DSM_NODE_ID; i++) {

		/* reception des informations de connexion des autres */
		/* processus envoyees par le lanceur : */
		/* nom de machine, numero de port, etc. */
		memset(buffer, 0, 1024);
		r = read(sock_dsmexec, buffer, 1024);
		if(r == -1)
			perror("ERROR with read() in dsm_init");

		change_buffer(buffer, 1024);

		// id
		id = atoi(buffer);

		if(id >= 0 && id < DSM_NODE_NUM) {
			/* initialisation des connexions */
			/* avec les autres processus : connect/accept */

			if(id < DSM_NODE_ID) {
				//							machine name 				port
				get_addr_info(&serv_info, buffer+strlen(buffer)+1, buffer + strlen(buffer) +1 + strlen(buffer+strlen(buffer)+1) + 1);
				fd_procs_dist[id] = creer_socket(SOCK_STREAM, &port);
				//printf("[%d] do_connect to > %d <\n", DSM_NODE_ID, id); fflush(stdout);
				
				// On se connecte au processus distant
				do_connect(fd_procs_dist[id], &serv_info, sizeof(serv_info));
				
				//printf("[%d] do_connect to > %d < ___OK___\n", DSM_NODE_ID, id); fflush(stdout);
				memset(buffer, 0, 1024);
				sprintf(buffer, "%d", DSM_NODE_ID);

				// On lui envoie notre DSM_NODE_ID
				do_write(fd_procs_dist[id], buffer, 1024);

				//printf("do_write done dsm_id: %d, id: %d\n", DSM_NODE_ID, id);
				fflush(stdout);
			}
		}
	}

//printf("pré accept: dsm_id: %d\n", DSM_NODE_ID); fflush(stdout);
	for(i = DSM_NODE_ID +1; i < DSM_NODE_NUM; i++) {
		memset(buffer, 0, 1024);
		memset(&client_addr_in, 0, sizeof(struct sockaddr_in));
		s_len = 0;

		//printf("[%d] accept number > %d <\n", DSM_NODE_ID, i);
		//fflush(stdout);

		// On accept les connection des processus qui ont un id > au notre
		fd = accept(sock_l, (struct sockaddr*) &client_addr_in, &s_len);
		//printf("[%d] accept number > %d < __OK__ , FD = %d, now do_read\n", DSM_NODE_ID, i, fd); fflush(stdout);

		// Celui ci nous donne son ID
		do_read(fd, buffer, 1024);
		//printf("[%d] do_read number > %d < __OK__ BUFFER: %s\n", DSM_NODE_ID, i, buffer); fflush(stdout);

		id = atoi(buffer);
		// On enregistre le fd
		if(id > DSM_NODE_ID && id < DSM_NODE_NUM)
			fd_procs_dist[id] = fd;
	}
	printf("[%d] DSM_INIT OK\n", DSM_NODE_ID); fflush(stdout);
   /* Allocation des pages en tourniquet */
   for(index = 0; index < PAGE_NUMBER; index ++) {
	 if ((index % DSM_NODE_NUM) == DSM_NODE_ID)
	   dsm_alloc_page(index);
	 dsm_change_info( index, WRITE, index % DSM_NODE_NUM);
   }
   
   /* mise en place du traitant de SIGSEGV */
   act.sa_flags = SA_SIGINFO; 
   act.sa_sigaction = segv_handler;
   sigaction(SIGSEGV, &act, NULL);
   
   /* creation du thread de communication */
   /* ce thread va attendre et traiter les requetes */
   /* des autres processus */
   pthread_create(&comm_daemon, NULL, dsm_comm_daemon, NULL);
   
   /* Adresse de début de la zone de mémoire partagée */
   return ((char *)BASE_ADDR);
}

void dsm_finalize( void )
{
	sleep(3); // Temporisation
	int i;

	/* terminer correctement le thread de communication */
   	/* pour le moment, on peut faire : */
   	pthread_cancel(comm_daemon);

	/* fermer proprement les connexions avec les autres processus */
	for(i = 0; i < DSM_NODE_NUM; i++) {
		close(fd_procs_dist[i]);
	}
	free(fd_procs_dist);
	
   
   
  return;
}

