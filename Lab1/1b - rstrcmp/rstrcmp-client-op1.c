#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    char buff[2048];
    int sd, err, nread;
    struct addrinfo hints, *ptr, *res;

    if (argc != 5) {
        fprintf(stderr, "Sintassi: rstrcmp hostname porta stringa1 stringa2\n");
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

    /* Invio la prima stringa, con terminatore. */
    if (write(sd, argv[3], strlen(argv[3]) + 1) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    /* Invio la seconda stringa, con terminatore. */
    if (write(sd, argv[4], strlen(argv[4]) + 1) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    /* Leggo il risultato dal Server e lo stampo a video. */
    while ((nread = read(sd, buff, sizeof(buff))) > 0) {
        write(1, buff, nread);
    }

    write(1, "\n", 1);
    close(sd);

    return 0;
}

