#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char  *argv[])
{
	struct addrinfo hints, *res;
	int err, sd, on, opt;
	char *hostname; 
    char *port; 
	memset(&hints, 0, sizeof(hints));

	hints.ai_family   = AF_UNSPEC;		//tipo di comunicazione(IPv4, IPv6, oppure entrambe --questo caso)
    hints.ai_socktype = SOCK_STREAM;	//comunicazione connecion oriented
    hints.ai_flags    = AI_PASSIVE;		//se settata, posso utilizzare indirizzo per fare la bind, se non settata da utilizzare per connect 

    port = argv[1];
    err = getaddrinfo(NULL, port, &hints, &res);
    if(err != 0)
    {
    	fprintf(stderr, "-------error type : %d-------\n",err);
    	exit(1);
    }

    sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    
    if (sd < 0)
    {
    	fprintf(stderr, "-------socket error-------\n");
    	exit(2);
    }

    on = 1;
    opt = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if(opt < 0) //SOL_SOCKET    protocollo per le socket    
    {																 //SO_REUSEADDR  in fase di bind  posso riutilizzare le porte
        fprintf(stderr, "-------setoption error-------\n");
        exit(3);
    }

    if (bind(sd, res->ai_addr, res->ai_addrlen) < 0) {
        fprintf(stderr,"-------error bind-------\n");
        exit(4);
    }

    freeaddrinfo(res);

    // passive socket is listening SOMAXCONN connection 
    listen(sd, SOMAXCONN);

    //fallback version
    for(;;) {
        int ns, pid, nread;
        ns = accept(sd, NULL, NULL);

        if ((pid = fork()) < 0) {
        	fprintf(stderr, "-------error fork------\n");
            exit(5);
        } else if (pid == 0) { /* FIGLIO */
            char mese[64], response[80], anno[4];

            /* Chiudo la socket passiva */
            close(sd);

            /* Inizializzo il buffer a zero e leggo sizeof(request)-1 byte, 
            * così sono sicuro che il contenuto del buffer
            * sarà sempre null-terminated. In questo modo, posso
            * interpretarlo come una stringa C e passarlo
            * direttamente alla funzione strlen. */

			memset(mese, 0, strlen(mese));
			memset(anno, 0, strlen(anno));
	    	if ((nread = read(ns, mese, sizeof(mese))) < 0) {
    	        fprintf(stderr, "-------error read-------\n");
        	  	exit(6);
            }
/*
			if( write(ns, "ACK", strlen("ACK")) < 0 )
			{
			fprintf(stderr, "-------error write-------\n" );
			exit(3);
			}	 
*/			
			while( strcmp(mese, "fine") != 0)
      		{
				fprintf(stderr, "mese : %s\n",mese);
				memset(anno, 0, strlen(anno));
				if ((nread = read(ns, anno, sizeof(anno))) < 0) {
    	        fprintf(stderr, "-------error read-------\n");
        	  	exit(6);
            	}
/*
				if( write(ns, "ACK", strlen("ACK")) < 0 )
				{
					fprintf(stderr, "-------error write-------\n" );
					exit(3);
				}
*/
				fprintf(stderr, "anno : %s\n",anno);
				char directory[256] = "./file/vendite/";
				strcat(directory, anno);
				strcat(directory, "/");
				chdir(directory);

				strcat(mese,".txt");

		        fprintf(stderr, "%s\n",mese);
				fprintf(stderr, "%s\n",directory);
		        close(1);

		        dup(ns);
				
	        	execlp("sort", "sort", "-n" ,mese , (char*)0);
				perror("execlp sort");

				memset(mese, 0, strlen(mese));
				if ((nread = read(ns, mese, sizeof(mese))) < 0) {
    	        fprintf(stderr, "-------error read-------\n");
        	  	exit(6);
            	}
				if( write(ns, "ACK", strlen("ACK")) < 0 )
				{
				fprintf(stderr, "-------error write-------\n" );
				exit(3);
				}
			}
			/* Termino il figlio */
			close(ns);
		    exit(EXIT_SUCCESS);
			
        }

        /* PADRE */
                
        /* Chiudo la socket attiva */
        close(ns);
    }




	return 0;
}
