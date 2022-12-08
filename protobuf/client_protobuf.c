#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include "messages.pb-c.h"
int main(int argc, char  *argv[])
{
	char buffer[2048];
	char dim[1];
	struct addrinfo hints, *ptr, *res;
	int err, sd = -1, nread = 0;

	if (argc != 3)
	{
		fprintf(stderr, "sintax : nome hostname porta \n" );
	}

	memset(&hints, 0 , sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	err = getaddrinfo(argv[1], argv[2], &hints, &res);

    Parametri cmsg = PARAMETRI__INIT;
    void * cmsg_buffer;
    unsigned len;

    /*
    ServerMessage *smsg;
    uint8_t smsg_buffer[2048];
    */

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
    char mese[64];
	int anno;
    
    int usr, psw;
    char email[64];
	memset(mese, 0, strlen(mese));
    printf("INSERIRE mese : ");
    scanf("%s", mese);
    

	while(strcmp(mese, "fine") != 0)
	{
		cmsg.mese = mese;
		printf("INSERIRE anno : ");
    	scanf("%d", &anno);
		cmsg.anno = anno;
	  
		//serializza il messaggio

		len = parametri__get_packed_size(&cmsg);
		cmsg_buffer = malloc(len);
		parametri__pack(&cmsg, cmsg_buffer);

		dim[0] = len;
		if (write(sd, dim,1) < 0){
		    fprintf(stderr, "-------error write-------\n");
		    exit(8);
		}


		if (write(sd, cmsg_buffer, len) < 0){
		    fprintf(stderr, "-------error write-------\n");
		    exit(8);
		}

		free(cmsg_buffer);

		memset(buffer, 0, sizeof(buffer));
		nread = read(sd, buffer, sizeof(buffer));
		//printf("%d\n",nread );
		if(nread < 0)
		{
		    fprintf(stderr, "-------error read-------\n");
		    exit(10);
		}
		fprintf(stdout, "%s\n",buffer );
		memset(mese, 0, strlen(mese));
		printf("INSERIRE mese : ");
    	scanf("%s", mese);
	}
    //smsg = server_message__unpack(NULL, nread, smsg_buffer);
    //fprintf(stdout, "%d\n", smsg->integer);
    //fprintf(stdout, "%d\n", smsg->decimal);

    //server_message__free_unpacked(smsg, NULL);

    close(sd);


	return 0;
}
