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

#define N 80

/* Gestore del segnale SIGCHLD */
void handler(int s) {
    int status;
    wait(&status);
}


int main(int argc, char **argv) {
    /******************************************SEMPRE***************************************************************/
    struct addrinfo hints, *res;//Servono sempre
    int err, sd, ns, pid;//Servono sempre
    /********************************************FINE SEMPRE*************************************************************/
    /**********************************************DELL'ESERCIZIO***********************************************************/
    int file;
    char Nome_FILE = {"conto_corrente.txt"};
    int pipefd[2];//Creazione pipe
    int pid2;//Creazione NIPOTE


    char *ack = "ack\n";

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

    go = 1;
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
            memset(CategoriaSpesa, 0, sizeof(CategoriaSpesa));
            if (read(ns, CategoriaSpesa, N - 1) < 0) {
                perror("read nomeFile");
                exit(9);
            }

            if (pipe(pipefd) == -1) {
                perror("PIPE");
                exit(EXIT_FAILURE);
            }


            if ((pid2 = fork()) < 0) {
                perror("FORK");
                exit(9);

            } else if (pid2 == 0) {
                /*nipote*/
                /*NON NECESSARIO; SERVE  A VERIFICARE ESISTENZA FILE*/
                if ((file = open(Nome_FILE, O_RDONLY)) == -1) {
                    close(ns);
                    exit(10);
                } else { close(file); }
/*******************/

                //Il nipote non comunica con il client quindi non gli serve il descrittore della connessione perciò chiude la sua copia
                close(ns);
                close(pipefd[0]);//Chiudo l'estremità della pipe che non uso

                //Redirezione pipefd[1] al posto di stdout

                close(1);//Chiudo stdout!
                dup(pipefd[1]);//duplico estremità pipe in uso che sostituisce stdout
                close(pipefd[1]);//Chiudo estremo pipe perchè verrà usato come stdout

                execlp("grep", "grep", CategoriaSpesa, Nome_FILE, (char *) 0);
                perror("execlp grep");

                exit(12);


            }
            /*figlio*/
            /*Chiudo l'estremità della pipe che non uso*/
            close(pipefd[1]);

            /* Redirezione stdin*/
            close(0);
            dup(pipefd[0]);
            close(pipefd[0]);

            execlp("sort", "sort", "-r", "-n", (char *) 0);
            perror("execlp sort");

            exit(13);


/*******************************************************FINE MODIFICA PER FARE COSE*********************************************************************************************/



        } else {
            /* padre */
            close(ns);
        }
    }
    return 0;
}
