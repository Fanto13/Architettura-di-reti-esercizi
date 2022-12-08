
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
#include "Invio.h"


#define DIM 4096


//************************PER SPEDIRE**************************************************


//************************PER RICEVERE*****************************************************
Response *package;
//void *buffer;
unsigned length;
uint8_t buffer[DIM];

inviato messaggio;


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
    char operatore;
    float operando;
    char c;

/*********************************FINE DELL'ESERCIZIO********************************************/
    /*********************************GENERAZIONE CLIENT********************************************/
    /* Controllo argomenti */
    if (argc < 2) {
        printf("Uso: client <server> <porta> ...\n");
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
 * EXIT DA 4 IN POI
 *
 */
    int stop = 1;


    printf("Inserire un operatore tra + - * / o z:> ");
    scanf("%c", &messaggio.operatore);

    while ((c = getchar()) != '\n' && c != EOF);


    while (messaggio.operatore != 'z') {


        printf("Inserire operando:> ");
        scanf("%f", &messaggio.operando);

        while ((c = getchar()) != '\n' && c != EOF);
        if (write(sd, &messaggio, sizeof(messaggio)) < 0) {
            perror("INVIO STRUCT");
            exit(4);
        }


        nread = read(sd, buff, sizeof(buff));//RICEVO
        printf("\n\n%d\n\n", nread);
        if (nread < 0) {
            perror("PROTOBUF");
            exit(5);
        }

        package = response__unpack(NULL, nread, buff);//DESERIALIZZO/ESTRAGGO
        if (package == NULL) {
            perror("ERRORE DESERIALIZZAZIONE");
            exit(6);
        }

        printf("STATO OPERAZIONE %s", package->stato);
        printf("Risultato: %f", package->risultato_parziale);

        response__free_unpacked(package, NULL);

        printf("Inserire un operatore tra + - * / o z:> ");
        scanf("%c", &messaggio.operatore);

        while ((c = getchar()) != '\n' && c != EOF);


    }

    if (write(sd, &messaggio, sizeof(messaggio)) < 0) {
        perror("INVIO STRUCT");
        exit(4);
    }


    close(sd);
    return 0;
}

/*******************************************************FINE MODIFICA PER FARE COSE*********************************************************************************************/
