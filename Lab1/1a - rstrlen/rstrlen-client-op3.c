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
    char buff[2048], dim[1];
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

    /* mando un singolo byte con la dimensione del messaggio
    (per ipotesi dimensione del messaggio inferiore a 2^8-1)
    In generale, con interi a più byte, problemi di
    1) dimensione in byte (16 bit, 32 bit, 64 bit),
    2) ordine dei byte (endianness),
    3) rappresentazione (ad esempio, complemento a 1 o a 2) */
    dim[0] = strlen(argv[3]);
    if (write(sd, dim, 1) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    /* non mando il terminatore - dovrò fare attenzione lato server */
    if (write(sd, argv[3], strlen(argv[3])) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    /* A valle delle due write quanti segmenti TCP sono stati inviati?
    Probabilmente uno solo. */

    /* Leggo un byte con la dimensione della risposta */
    if (read(sd, dim, 1) < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    /* Leggo "dim" byte, esattamente la dimensione della risposta */
    if (ricevi(sd, buff, dim[0]) < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    write(1, buff, dim[0]);
    write(1, "\n", 1);
    close(sd);

    return 0;
}

