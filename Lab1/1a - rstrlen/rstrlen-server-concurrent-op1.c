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

void handler(int signo) {
    int status;
    /* gestisco tutti i figli terminati */
    while (waitpid(-1, &status, WNOHANG) > 0);
    /*A child that terminates, but has not been waited for becomes a "zombie".
    The kernel maintains a minimal set of information about the zombie process
    (PID, termination status, resource usage information) in order to allow the parent
    to later perform a wait to obtain information about the child.
    As long as a zombie is not removed from the system via a wait,
    it will consume a slot in the kernel process table, and if this table fills,
    it will not be possible to create further processes.
    If a parent process terminates, then its "zombie" children (if any) are adopted by init(8),
    which automatically performs a wait to remove the zombies.*/
}

int main(int argc, char **argv) {
    int sd, errno, on;
    struct addrinfo hints, *res;

    /* sighandler_t signal(int signum, sighandler_t handler);
    signal() sets the disposition of the signal signum to handler,
    which is either SIG_IGN, SIG_DFL, or the address of a
    programmer-defined function (a "signal handler"). */
    signal(SIGCHLD, handler);

    memset(&hints, 0, sizeof(hints));
    /* Usa AF_INET per forzare solo IPv4, AF_INET6 per forzare solo IPv6 */
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    /* porta locale a cui fare la bind passata come argv[1]*/
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
            char request[2048], response[80];

            /* Chiudo la socket passiva */
            close(sd);

            /* Inizializzo il buffer a zero e leggo sizeof(request)-1 byte,
             * così sono sicuro che il contenuto del buffer
             * sarà sempre null-terminated. In questo modo posso
             * interpretarlo come una stringa C e passarlo
             * direttamente alla funzione strlen. */
            memset(request, 0, sizeof(request));
            if ((nread = read(ns, request, sizeof(request) - 1)) < 0) {
                perror("read");
                exit(EXIT_FAILURE);
            }

            /* Preparo il buffer contenente la risposta */
            snprintf(response, sizeof(response), "%d", (int) strlen(request));

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

