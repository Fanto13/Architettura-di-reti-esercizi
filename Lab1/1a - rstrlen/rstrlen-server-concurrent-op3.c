#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

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

void handler(int signo) {
    int status;
    /* gestisco tutti i figli terminati */
    while (waitpid(-1, &status, WNOHANG) > 0);
    /*A child that terminates, but has not been waited for becomes a "zombie". The kernel maintains a minimal set of information about the zombie process (PID, termination status, resource usage information) in order to allow the parent to later perform a wait to obtain information about the child. As long as a zombie is not removed from the system via a wait, it will consume a slot in the kernel process table, and if this table fills, it will not be possible to create further processes. If a parent process terminates, then its "zombie" children (if any) are adopted by init(8), which automatically performs a wait to remove the zombies.*/
}

int main(int argc, char **argv) {
    int sd, errno, on;
    struct addrinfo hints, *res;

    signal(SIGCHLD, handler);

    memset(&hints, 0, sizeof(hints));
    /* Usa AF_INET per forzare solo IPv4, AF_INET6 per forzare solo IPv6 */
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((errno = getaddrinfo(NULL, argv[1], &hints, &res)) != 0) {
        fprintf(stderr, "Errore setup indirizzo bind: %s\n", gai_strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        perror("Errore in socket");
        exit(EXIT_FAILURE);
    }

    on = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(sd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Errore in bind");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    /* trasforma in socket passiva d'ascolto */
    listen(sd, SOMAXCONN);

    for (;;) {
        int ns, pid, nread;

        if ((ns = accept(sd, NULL, NULL)) < 0) {
            /* Ignoro gli errori di tipo interruzione da segnale. */
            /* The <errno.h> header file defines the integer variable errno, which
            is set by system calls and some library functions in the event of an
            error to indicate what went wrong.  Its value is significant only
            when the return value of the call indicated an error (i.e., -1 from
            most system calls; -1 or NULL from most library functions); a
            function that succeeds is allowed to change errno. */
            /* EINTR Interrupted function call */
            if (errno == EINTR)
                continue;
            /* Gestisco tutte le altre tipologie di errore. */
            perror("accept");
            exit(7);
        }

        if ((pid = fork()) < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { /* FIGLIO */
            char request[4096], response[80], buff[1];

            /* Chiudo la socket passiva */
            close(sd);

            /* Leggo un singolo byte con la dimensione del messaggio successivo */
            if ((nread = read(ns, buff, 1)) < 0) {
                perror("read");
                exit(EXIT_FAILURE);
            }

            /* Leggo esattamente dim byte. */
            memset(request, 0, sizeof(request));
            if (ricevi(ns, request, buff[0]) < 0) {
                perror("read");
                exit(EXIT_FAILURE);
            }

            /* Preparo il buffer contenente la risposta */
            snprintf(response, sizeof(response), "%d", (int) strlen(request));

            /* Preparo il buffer contenente la dimensione della risposta */
            buff[0] = strlen(response);

            /* Invio la dimensione della risposta */
            write(ns, buff, 1);

            /* Invio la risposta */
            write(ns, response, strlen(response));

            /* Chiudo la socket attiva */
            close(ns);

            /* Termino il figlio */
            exit(EXIT_SUCCESS);
        }

        /* PADRE */

        /* Chiudo la socket attiva */
        close(ns);
    }

    return 0;
}

