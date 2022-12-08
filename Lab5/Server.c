//
// Created by giacomo on 02/12/18.
//

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include "message.pb-c.h"

#define N 80
Request *package;
void *buffer;
unsigned length;

/* Gestore del segnale SIGCHLD */
void handler(int s) {
    int status;
    wait(&status);
}


int main(int argc, char **argv) {
    /******************************************SEMPRE***************************************************************/
    struct addrinfo hints, *res;//Servono sempre
    int err, sd, ns, pid;//Servono sempre
    int on;
    /********************************************FINE SEMPRE*************************************************************/
    /**********************************************DELL'ESERCIZIO***********************************************************/
    /*
     *
     * DICHIARAZIONE VARIABILI UTILI
     *
     *
     */
    char buff[256];
    int nread;
    char regione[255];
    char impianti[255];
    int pid2;
    int piped[2];

    /**********************************************FINE DELL'ESERCIZIO**********************************************************/
    /***********************************************GENERAZIONE SERVER*************************************************************/
    /* Controllo argomenti */
    if (argc < 2) {
        printf("Uso: ./server <porta> \n");
        exit(1);
    }

    signal(SIGCHLD, handler);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) != 0) {
        fprintf(stderr, "Errore setup indirizzo bind: %s\n", gai_strerror(err));
        exit(2);
    }

    if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        perror("Errore in socket");
        exit(3);
    }

    on = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt");
        exit(4);
    }

    if (bind(sd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Errore in bind");
        exit(5);
    }

    freeaddrinfo(res);

    if (listen(sd, SOMAXCONN) < 0) {
        perror("listen");
        exit(6);
    }

    /*******************************************FINE GENERAZIONE SERVER**************************************************************/


    /* Attendo i client... */
    for (;;) {
        printf("Server in ascolto...\n");

        if ((ns = accept(sd, NULL, NULL)) < 0) {
            /* Ignoro gli errori di tipo interruzione da segnale. */
            if (errno == EINTR)
                continue;
            /* Gestisco tutte le altre tipologie di errore. */
            perror("accept");
            exit(7);
        }

        /* Generazione di un figlio */
        if ((pid = fork()) < 0) {
            perror("fork");
            exit(8);
        } else if (pid == 0) {
            /* figlio */
            close(sd);
/*******************************************************MODIFICARE PER FARE COSE*********************************************************************************************/
/*
 *
 * FIGLIO E CODICE DA ESEGUIRE
 *
 *
 */


            nread = read(ns, buff, sizeof(buff));
            if (nread < 0) {
                perror("PROTOBUF");
                exit(3);
            }
            package = request__unpack(NULL, nread, buff);
            if (package == NULL) {
                perror("ERRORE DESERIALIZZAZIONE");
                exit(4);
            }
            memset(regione, 0, sizeof(regione));
            strcat(regione, package->nomeregione);
            strcat(regione, ".txt");
            sprintf(impianti, "%d", package->numero_impianti);

            request__free_unpacked(package, NULL);

            if (pipe(piped) < 0) {
                perror("ERRORE PIPE");
                exit(10);
            }


            if ((pid2 = fork()) < 0) {
                perror("Seconda fork");
                exit(11);
            } else if (pid2 == 0) {
                //NIPOTE

                close(piped[0]);
                close(ns);
                //STDOUT
                close(1);
                dup(piped[1]);
                //STDERROR
                close(2);
                dup(piped[1]);
                close(piped[1]);

                execlp("sort", "sort", "-n", "-r", regione, (char *) 0);
                perror("SORT ERROR");
                exit(123);


            }


            close(piped[1]);
//SCRIVO SU STREAM
            close(0);
            dup(piped[0]);
            close(piped[0]);
            //STDOUT
            close(1);
            dup(ns);
            //STDERROR
            close(2);
            dup(ns);
            close(ns);

            execlp("head", "head", "-n", impianti, (char *) 0);
            perror("EXEC HEAD");
            exit(111);



/*******************************************************FINE MODIFICA PER FARE COSE*********************************************************************************************/



        } else {
            /* padre */
            close(ns);
        }
    }
    return 0;
}
