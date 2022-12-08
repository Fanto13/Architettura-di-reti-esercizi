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

#define DIM 4096

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
    int nread;
    char buf[DIM];
    int i = 1;
/*********************************FINE DELL'ESERCIZIO********************************************/
    /*********************************GENERAZIONE CLIENT********************************************/
    /* Controllo argomenti */
    if (argc < 2) {
        printf("Uso: rps <server> <options>...\n");
        exit(1);
    }

    /* Costruzione dell'indirizzo */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    /* Risoluzione dell'host */
    host_remoto = argv[1];
    servizio_remoto = "50000";
    if ((err = getaddrinfo(host_remoto, servizio_remoto, &hints, &res)) != 0) {
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


    /* Scambio dati */

    /* Invio argomenti comando ps */
    if (argc < 3) {
        strcpy(buf, "");
    } else {
        strcpy(buf, argv[2]);
    }

    /* Cosa vuol dire inviare 0 byte?
    man write https://linux.die.net/man/3/write
    [...] If nbyte is zero and the file is not a regular file, the results are unspecified. [...]
    Perché funziona anche con 0 byte? Cosa riceve il server?
    Comuqnue meglio adottare soluzioni alternative,
    ad esempio non effettuare la write se non ci sono option
    oppure inviare sempre strlen(buf+1), così da inviare sempre almeno il terminatore di stringa. */
    if (write(sd, buf, strlen(buf)) < 0) {
        perror("write params");
        exit(6);
    }

    /* Facciamo lo shutdown della scrittura sulla socket */
    shutdown(sd, SHUT_WR);

    /* Ricezione risultato */
    printf("Server %s:\n", argv[1]);
    /* È consigliabile effettuare il flushing del buffer di printf
     * prima di iniziare a scrivere sullo standard output con write */
    fflush(stdout);

    while ((nread = read(sd, buf, DIM)) > 0) {
        if (write(1, buf, nread) < 0) {
            perror("write su stdout");
            exit(9);
        }
    }
    printf("fine comando ps\n");

    /* Controllo errori di lettura */
    if (nread < 0) {
        perror("read dei risultati");
        exit(10);
    }

    close(sd);
    return 0;
}