#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int ricevi(int sd, char *buf, int n) {
    int i, j;
    i = recv(sd, buf, n, 0);
    if (i != n && i != 0) {
        if (i == -1) {
            fprintf(stderr, "errore in lettura\n");
            exit(1);
        }
        while (i < n) {
            j = recv(sd, &buf[i], n - i, 0);
            if (j == -1) {
                fprintf(stderr, "errore in lettura\n");
                exit(1);
            }
            i += j;
            if (j == 0) break;
        }
    } /* si assume che tutti i byte arrivino… se si verifica il fine file si esce */
    return i;
}

int main(int argc, char **argv) {
    char buff[2048];
    char response[80];
    int sd, err, nread;
    struct addrinfo hints, *ptr, *res;

    if (argc != 4) {
        fprintf(stderr, "Sintassi: rstrlen hostname porta stringa\n");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    err = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (err != 0) {
        fprintf(stderr, "Errore risoluzione nome: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    /* connessione con fallback */
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next) {
        sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        /* se la socket fallisce, passo all'indirizzo successivo */
        if (sd < 0) continue;

        /* se la connect va a buon fine, esco dal ciclo */
        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
            break;

        /* altrimenti, chiudo la socket e passo all'indirizzo
         * successivo */
        close(sd);
    }

    /* controllo che effettivamente il client sia connesso */
    if (ptr == NULL) {
        fprintf(stderr, "Errore di connessione!\n");
        exit(EXIT_FAILURE);
    }

    /* a questo punto, posso liberare la memoria allocata da getaddrinfo */
    freeaddrinfo(res);

    /* Preparo il buffer contenente la richiesta */
    snprintf(buff, sizeof(buff), "%s", argv[3]);

    /* mando l'intero buffer */
    if (write(sd, buff, sizeof(buff)) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    /* Leggo l'intero buffer di risposta */
    memset(response, 0, sizeof(response));
    if (ricevi(sd, response, sizeof(response)) < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", response);
    close(sd);

    return 0;
}

