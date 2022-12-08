
//
// Created by giacomo on 02/12/18.
//


#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "message.pb-c.h"

#define DIM 4096


//************************PER SPEDIRE**************************************************
Request msg = REQUEST__INIT;
void *buffer;
unsigned length;
//************************PER RICEVERE*****************************************************
Response *package;
void *buffer;
unsigned length;

int main(int argc, char **argv) {
    /*********************************SEMPRE********************************************/
    int err;
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *ptr;
    char *host_remoto;
    char *servizio_remoto;
    int sd;
    /*********************************FINE SEMPRE********************************************/
    /*********************************DELL'ESERCIZIO********************************************/
    /*
     * VARIBILI UTILI
     */
    char buff[DIM];
    int nread;
    char nome_file[DIM];
    int end = 0;
    int found;
/*********************************FINE DELL'ESERCIZIO********************************************/
    /*********************************GENERAZIONE CLIENT********************************************/
    /* Controllo argomenti */
    if (argc < 3) {
        printf("Uso: client <server> <porta> <nome_file>...\n");
        exit(1);
    }
    /****************************************************************************************/

    /* Costruzione dell'indirizzo */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    /* Risoluzione dell'host */
    host_remoto = argv[1];

    if ((err = getaddrinfo(host_remoto, argv[2], &hints, &res)) != 0) {
        fprintf(stderr, "Errore risoluzione nome: %s\n", gai_strerror(err));
        exit(2);
    }
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next) {
        /*se socket fallisce salto direttamente alla prossima iterazione */
        if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0) {
            fprintf(stderr, "creazione socket fallita\n");
            continue;
        }
        /*se connect funziona esco dal ciclo */
        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0) {
            printf("connect riuscita\n");
            break;
        }

        close(sd);
    }
    /* Verifica sul risultato restituito da getaddrinfo */
    if (ptr == NULL) {
        fprintf(stderr, "Errore risoluzione nome: nessun indirizzo corrispondente trovato\n");
        exit(3);
    }

    /* Liberiamo la memoria allocata da getaddrinfo() */
    freeaddrinfo(res);
/*********************************FINE GENERAZIONE CLIENT********************************************/
/*
 *
 * CODE CHE IL CLIENT DEVE FARE
 *
 */
    strcpy(nome_file, argv[3]);

    while (!end) {

        msg.nome_file = nome_file;


        length = request__get_packed_size(&msg);
        buffer = malloc(length);
        request__pack(&msg, buffer);


        if (write(sd, buffer, length) < 0) {
            perror("WRITE ERROR");
            exit(6);
        }

        free(buffer);


        fflush(stdout);

        //*************************************************************************
        nread = read(sd, buff, sizeof(buff));
        if (nread < 0) {
            perror("PROTOBUF");
            exit(3);
        }
        package = response__unpack(NULL, nread, buff);
        if (package == NULL) {
            perror("ERRORE DESERIALIZZAZIONE");
            exit(4);
        }

        found = package->filedim;
        if (found < 0) {
            printf("NON TROVATO\n Inserire il nome di un altro file: ");
            scanf("%s", nome_file);


        } else {
            printf("\n Filedim: %d\n", found);
            end = 1;
        }


        response__free_unpacked(package, NULL);
    }

    close(sd);
    return 0;
}

/*******************************************************FINE MODIFICA PER FARE COSE*********************************************************************************************/
