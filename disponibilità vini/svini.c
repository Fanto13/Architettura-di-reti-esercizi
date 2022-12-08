#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* SERVER ES 4.9 vini  */

void handler(int signo)
{
        int status;
        /* gestisco tutti i figli */
        while (waitpid(-1, &status, WNOHANG) > 0)
                ;
}

int main(int argc, char **argv)
{

        int sd, err, pid, pid2;
        struct addrinfo hints, *res;
        int piped[2];

        memset(&hints, 0, sizeof(hints));
        /* usa AF_INET per forzare IPv4, AF_INET6 per IPv6 */
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        /* controllo input */
        if (argc < 3)
        {
                printf("Uso: ./nomeserver porta\n");
                exit(1);
        }

        /* CONNESSIONE */

        /* risoluzione dei nomi */
        if ((err = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0)
        {
                fprintf(stderr, "Errore setup indirizzo bind: %s\n", gai_strerror(err));
                exit(EXIT_FAILURE);
        }

        /* connessione socket */
        if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        {
                perror("errore in bind");
                exit(EXIT_FAILURE);
        }

        /* binding */
        if (bind(sd, res->ai_addr, res->ai_addrlen) < 0)
        {
                perror("errore in bind");
                exit(EXIT_FAILURE);
        }

        freeaddrinfo(res);

        /* trasforma in socket passiva d'ascolto */
        if (listen(sd, SOMAXCONN) < 0)
        {
                perror("listen");
                exit(EXIT_FAILURE);
        }

        /* trasforma in socket passiva d'ascolto */
        //listen(sd, SOMAXCONN);

        /* CODICE SERVER */

        for (;;)
        {
                printf("Server in ascolto...\n");
                /* accept */
                int ns;
                if ((ns = accept(sd, NULL, NULL)) < 0)
                {
                        perror("accept");
                        exit(EXIT_FAILURE);
                }
                /* fork */
                if ((pid = fork()) < 0)
                {
                        perror("fork");
                        exit(EXIT_FAILURE);

                        /* figlio */
                }
                else if (pid == 0)
                {
                        /* chiusura socket passiva */
                        close(sd);

                        char nome[80], annata[80];
                        const char *ack = "ACK";
                        //chiudo socket passiva

                        /* inizializzo i buffer di lettura a 0 e leggo */
                        memset(nome, 0, sizeof(nome));

                        /* ricezione nome dal client */
                        if (read(ns, nome, sizeof(nome) - 1) < 0)
                        {
                                perror("read");
                                exit(EXIT_FAILURE);
                        }

                        /* invio ACK al client per avvenuta lettura */
                        if (write(ns, ack, strlen(ack)) < 0)
                        {
                                perror("write ack");
                                exit(EXIT_FAILURE);
                        }
                        memset(annata, 0, sizeof(annata));
                        /* ricezione annata dal client */
                        if (read(ns, annata, sizeof(annata) - 1) < 0)
                        {
                                perror("read");
                                exit(EXIT_FAILURE);
                        }

                        /*move su /var/local/magazzino.txt */
                        /*chdir("/var/local/");*/

                        if (pipe(piped) < 0)
                        {
                                perror("pipe");
                                exit(EXIT_FAILURE);
                        }

                        if ((pid2 = fork()) < 0)
                        {
                                perror("seconda fork");
                                exit(EXIT_FAILURE);
                        }
                        else if (pid2 == 0)
                        {
                                /* nipote 1 cerco il nome del vino*/
                                /* Ridireziono stdin */

                                close(piped[0]);
                                close(ns);
                                /* Ridireziono stdout */
                                close(1);
                                dup(piped[1]);
                                close(piped[1]);

                                execlp("grep", "grep", nome, "magazzino.txt", NULL);
                                perror("exec prima grep");
                                exit(EXIT_FAILURE);
                        }
                        else
                        {
                                if ((pid = fork()) >= 0)
                                {
                                        close(piped[1]);
                                        /* nipote 2 cerca l'annata del vino*/
                                        /* Ridireziono stdin */
                                        close(0);
                                        dup(piped[0]);
                                        close(piped[0]);
                                        /* Ridireziono stdout */
                                        close(1);
                                        dup(ns);
                                        close(ns);
                                        execlp("grep", "grep", annata, NULL);
                                        perror("exec");
                                        exit(EXIT_FAILURE);
                                }

                        } /*figlio*/
                          /*padre*/

                        close(ns);
                }
                return 0;
        }
