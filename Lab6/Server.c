//
// Created by giacomo on 02/12/18.
//

//*****************************************************************
#include <sys/stat.h>
//*****************************************************************
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
Response msg = RESPONSE__INIT;


struct stat st;

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
    int dowhile = 1;
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
//*************************************************************
    char nome_file[256];
//*************************************************************
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

            while (dowhile) {

                nread = read(ns, buff, sizeof(buff));//RICEVO
                if (nread < 0) {
                    perror("PROTOBUF");
                    exit(3);
                }
                package = request__unpack(NULL, nread, buff);//DESERIALIZZO/ESTRAGGO
                if (package == NULL) {
                    perror("ERRORE DESERIALIZZAZIONE");
                    exit(4);
                }

                memset(&nome_file, 0, sizeof(nome_file));
                strcpy(nome_file, package->nome_file);

                request__free_unpacked(package, NULL);

                if (stat(nome_file, &st)!=0)
                {
                    msg.filedim = -1;
                }
                else
                {
                    msg.filedim = st.st_size;
                    dowhile = 0;
                }

                length = response__get_packed_size(&msg);
                buffer = malloc(length);
                response__pack(&msg, buffer);

                if (write(ns, buffer, length) < 0) {
                    perror("WRITE ERROR");
                    exit(6);
                }

                free(buffer);
                fflush(stdout);
            }

            printf("FINE CICLO WHILE");
            exit(0);//IMPORTANTE!!!!!!!

/*******************************************************FINE MODIFICA PER FARE COSE*********************************************************************************************/



        } else {
            /* padre */
            close(ns);
        }
    }
    return 0;
}
