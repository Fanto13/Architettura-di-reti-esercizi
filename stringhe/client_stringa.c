#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

int main(int argc, char const *argv[])
{
	char mese[64], anno_stringa[5], buffer[2048], ack[3];
	int anno;
	
	struct addrinfo hints, *ptr, *res;
	int err, sd = -1, nread = 0;

	if (argc != 3)
	{
		fprintf(stderr, "sintax : nome hostname porta\n" );
	}

	memset(&hints, 0 , sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	err = getaddrinfo(argv[1], argv[2], &hints, &res);


	if (err != 0) {
        fprintf(stderr, "-------error name : %s-------\n", gai_strerror(err));
        exit(1);
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
    	sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    	if (sd < 0) continue;

    	if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0) break;
    	close(sd);
    }
   
    if(ptr == NULL)
    {
    	fprintf(stderr, "-------error socket-------\n");
    	exit(2);
    }

    freeaddrinfo(res);
	memset(mese, 0, sizeof(mese));
	fprintf(stdout, "Digitare mese : \n");
	scanf("%s", mese);
	while(strcmp(mese, "fine") != 0)
	{
		if( write(sd, mese, sizeof(mese)-1) < 0 )
		{
			fprintf(stderr, "-------error write-------\n" );
			exit(3);
		}
/*
		while ((nread = read(sd, ack, sizeof(buffer))) > 0)
		{
			write(1, ack, nread);
		}
*/
		fprintf(stdout, "Digitare anno : \n");
		scanf("%d", &anno);

		sprintf(anno_stringa, "%d", anno);
		
		
		if( write(sd, anno_stringa, sizeof(anno_stringa) -1) < 0 )
		{
			fprintf(stderr, "-------error write-------\n" );
			exit(3);
		}
/*
		while ((nread = read(sd, ack, sizeof(buffer))) > 0)
		{
			write(1, ack, nread);
		}
*/
		while ((nread = read(sd, buffer, sizeof(buffer))) > 0)
		{
			write(1, buffer, nread);
		}
		write(1,"\n",1);
		memset(mese, 0, sizeof(mese));
		fprintf(stdout, "Digitare mese : \n");
		scanf("%s", mese);

 	}   
    close(sd);

	return 0;
}
