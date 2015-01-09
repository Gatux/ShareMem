#include "common.h"

void do_connect(int sockfd, const struct sockaddr_in *addr, socklen_t addrlen) {
	int i = 1;
	int r = -1;

	while(r != 0 && i) {
		r = connect(sockfd, (struct sockaddr*)addr, addrlen);
		if(r == 0)
			break;
		i++;
	}
	if(r != 0)
		perror("Error with connect() in do_connect()");
}

int get_addr_info(struct sockaddr_in* serv_info, char* host, char* port)
{
	//struct hostent* h = gethostbyname(host);
	struct addrinfo* res;
	struct addrinfo hint;
	struct sockaddr_in *addr;

	memset(&hint, 0, sizeof(hint));

	/* getaddrinfo will fill the res structure with information about the
	   server based on its hostname
	   it returns 0 on success */
	if( getaddrinfo(host, port, &hint, &res) )
    {
    	perror("getaddrinfo");
    	exit(errno);
    }

    memset(serv_info, 0, sizeof(*serv_info));
    	serv_info->sin_family = AF_INET;
    	serv_info->sin_port =  htons(atoi(port));

    /* These information are then stored into the addr structure*/
    addr = (struct sockaddr_in *)res->ai_addr;

    /* The server's IP address is stored into the serv_info structure */
    serv_info->sin_addr = addr->sin_addr;

    /* res is freed */
    freeaddrinfo(res);

    return 0;
}

 int creer_socket(int prop, int *port_num) {
 	int yes = 1;
 	int fd = socket(AF_INET, prop, 0);
 	struct sockaddr_in serv_addr;
 	socklen_t len;
 	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
 	serv_addr.sin_family = AF_INET;
 	serv_addr.sin_port = 0;
 	serv_addr.sin_addr.s_addr = INADDR_ANY;

 	if(fd == -1) {
 		perror("ERROR with socket() in do_socket()");
 	}

 	if(bind(fd, (struct sockaddr*)(&serv_addr), sizeof(struct sockaddr_in)) == -1)
 		perror("error with bind() in creer_socket()");

 	len = sizeof(struct sockaddr*);
 	if(getsockname(fd, (struct sockaddr*)(&serv_addr), &len) == -1)
 		perror("error with getsockname() in creer_socket()");

 	*port_num = ntohs(serv_addr.sin_port);

	// 	 Setting the socket option so the (address, port) tuple can be used again
	// 	   right after the connection is closed
 	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
 		perror("ERROR setting socket options");
 	return fd;
 }


/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */

 int change_buffer(char* buffer, int taille) {
	 int i;
	 int n = 1;
	 if(buffer != NULL)
		 for(i = 0; i < taille; i++)
			 if(buffer[i] == '\n') {
				 buffer[i] = 0;
				 n++;
			 }
	 return n;
 }

void do_write(int fg, char* buffer, int size) {
	 int r = size;
	 int i;
	 if(buffer != NULL) {
		 do {
			 i = write(fg, buffer+(size-r), r);
			 if(i == -1) {
				 perror("ERROR with write() in do_write");
				 return;
			 }
			 r -= i;
		 } while(r > 0);
	 }
 }
int do_read(int fd, char* buffer, int size) {
	int r = read(fd, buffer, size);
		if(r == -1)
			perror("ERROR with read()");
		return r;
}
