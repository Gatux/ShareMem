#include "common_impl.h"

// int creer_soket(int domain, int type, int protocol) {
// 	int yes = 1;
// 	int fd = socket(domain, type, protocol);
// 	if(fd == -1) {
// 		error("ERROR with socket() in do_socket()");
// 	}

// 	 Setting the socket option so the (address, port) tuple can be used again 
// 	   right after the connection is closed 
// 	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
// 		error("ERROR setting socket options");
// 	return fd;
// }


int creer_socket(int prop, int *port_num) 
{
   int fd = 0;
   
   /* fonction de creation et d'attachement */
   /* d'une nouvelle socket */
   /* renvoie le numero de descripteur */
   /* et modifie le parametre port_num */
   
   return fd;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */