
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

Request msg = REQUEST__INIT;
void *buffer;
unsigned length;
//----------------------------------------------
Response *response_message;
uint8_t buf[]


int main(int argc, char **argv) {
    /*********************************SEMPRE********************************************/
    int err;
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *ptr;
    char *host_remoto;
    char *servizio_remoto;
    int sd;
    int i = 0;
    /*********************************FINE SEMPRE********************************************/
    /*********************************DELL'ESERCIZIO********************************************/
    message package;
    response response_message;
    int nread;
    /*
     * VARIBILI UTILI
     */
/*********************************FINE DELL'ESERCIZIO********************************************/
    /*********************************GENERAZIONE CLIENT********************************************/
    /* Controllo argomenti */
    if (argc < 6) {
        printf("Uso: rsqrt <hostname> <port> <username> <password> <value>\n");
        exit(1);
    }

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
            printf("connect riuscita al tentativo %d\n", i);
            break;
        }
        i++;
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


    //PROTOBUF WRITE

    msg.username = argv[3];
    msg.password = argv[4];
    msg.value = atof(argv[5]);


    length = request__get_packed_size(&msg);
    buffer = malloc(length);
    request__pack(&msg, buffer);


    if (write(sd, buffer, length) < 0) {
        perror("WRITE ERROR");
        exit(6);
    }

    free(buffer);
    //END PROTOBUF WRITE

    /* Facciamo lo shutdown della scrittura sulla socket */
    // shutdown(sd, SHUT_WR);

    if (read(sd, &response_message, sizeof(response)) < 0) {
        perror("READ ERROR");
        exit(7);
    }

    printf("sqrt(%f)=%d.%d\n", package.value, response_message.integer, response_message.decimal);

    printf("Parte intera: %d\n Parte frazionaria: %d\n", response_message.integer, response_message.decimal);


    /*

    {
        if (write(1, buf, nread) < 0) {
            perror("write su stdout");
            exit(9);
        }
    }*/

    close(sd);
    return 0;
}

/*******************************************************FINE MODIFICA PER FARE COSE*********************************************************************************************/
