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


static void *dsm_comm_daemon( void *arg)
{  
   while(1)
     {
	/* a modifier */
	printf("[%i] Waiting for incoming reqs \n", DSM_NODE_ID);
	sleep(2);
     }
   return NULL;
}

static int dsm_send(int dest,void *buf,size_t size)
{
   /* a completer */
}

static int dsm_recv(int from,void *buf,size_t size)
{
   /* a completer */
}

static void dsm_handler( void )
{  
   /* A modifier */
   printf("[%i] FAULTY  ACCESS !!! \n",DSM_NODE_ID);
   abort();
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
	dsm_handler();
     }
   else
     {
	/* SIGSEGV normal : ne rien faire*/
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

	char** machine_names;
	int* ports;

	char buffer[1024];

	sock_dsmexec = atoi(getenv("SOCK_DSMEXEC"));
	sock_l = atoi(getenv("SOCK_L"));

	memset(buffer, 0, 1024);
	r = read(sock_dsmexec, buffer, 1024);
	if(r == -1)
		perror("ERROR with read() in dsm_init");

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
	machine_names = calloc(DSM_NODE_NUM, sizeof(char*));
	ports 		 = calloc(DSM_NODE_NUM, sizeof(int));

	for(i = 0; i < DSM_NODE_NUM; i++) {
   
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
			// nom de machine
			machine_names[id] = calloc(strlen(buffer)+1, sizeof(char));
			strcpy(machine_names[id], buffer+strlen(buffer)+1);

			// port
			ports[id] = atoi(buffer + strlen(buffer) +1 + strlen(buffer+strlen(buffer)+1) + 1);

			/* initialisation des connexions */
			/* avec les autres processus : connect/accept */

			if(id < DSM_NODE_ID) {
				get_addr_info(&serv_info, machine_names[id], buffer + strlen(buffer) +1 + strlen(buffer+strlen(buffer)+1) + 1);
				fd_procs_dist[id] = creer_socket(SOCK_STREAM, &port);
				do_connect(fd_procs_dist[id], &serv_info, sizeof(serv_info));
				memset(buffer, 0, 1024);
				sprintf(buffer, "%d", id);
				do_write(fd_procs_dist[id], buffer, 1024);
			}
		}
	}

	for(i = DSM_NODE_ID +1; i < DSM_NODE_NUM; i++) {
		memset(buffer, 0, 1024);
		memset(&client_addr_in, 0, sizeof(struct sockaddr_in));
		s_len = 0;

		fd = accept(sock_l, (struct sockaddr*) &client_addr_in, &s_len);

		do_read(fd, buffer, 1024);

		id = atoi(buffer);
		if(id > DSM_NODE_ID && id < DSM_NODE_NUM)
			fd_procs_dist[id] = fd;
	}

   /* Allocation des pages en tourniquet */
   for(index = 0; index < PAGE_NUMBER; index ++){	
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
	free(fd_procs_dist);
   /* fermer proprement les connexions avec les autres processus */

   /* terminer correctement le thread de communication */
   /* pour le moment, on peut faire : */
   pthread_cancel(comm_daemon);
   
  return;
}

