#include <stdio.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
        char buff[2048];
        char fine[] = "fine";
        char nome[80], annata[80];
        int sd, err, nread;
        struct addrinfo hints, *ptr, *res;

        if (argc < 3)
        {
                printf("uso: verifica_disponibilità_vini server porta");
                exit(1);
        }

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        err = getaddrinfo(argv[1], argv[2], &hints, &res);
        if (err != 0)
        {
                fprintf(stderr, "Errore risoluzione nome: %s\n", gai_strerror(err));
                exit(EXIT_FAILURE);
        }

        /* il client rimane attivo finchè non passo in input la stringa fine */
        while (&free)
        { /* &free restituisce true, che non mi faceva usare compilation time */

                /* connessione con fallback */
                for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
                {
                        sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
                        /* se la socket fallisce, passo all'indirizzo successivo */
                        if (sd < 0)
                        {
                                continue;
                        }

                        /* se la connect va a buon fine, esco dal ciclo */
                        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
                        {
                                break;
                        }

                        /* altrimenti, chiudo la socket e passo all'indirizzo
						 * successivo */
                        close(sd);
                }

                /* controllo che effettivamente il client sia connesso */
                if (ptr == NULL)
                {
                        fprintf(stderr, "Errore di connessione!\n");
                        exit(EXIT_FAILURE);
                }

                /* controllo utente */
                printf("\nInserimento parametri di rierca. 'fine' per terminare");

                printf("\nNome vino: ");
                scanf("%s", nome);
                if (strcmp(nome, fine) == 0)
                        break;

                printf("Annata: ");
                scanf("%s", annata);
                printf("\n");
                if (strcmp(annata, fine) == 0)
                        break;

                /* COMUNICAZIONE SERVER */

                /* invio nome al server */
                if (write(sd, nome, strlen(nome)) < 0)
                {
                        perror("write");
                        exit(EXIT_FAILURE);
                }

                /* ricevo ACK */
                if (read(sd, buff, sizeof(buff)) < 0)
                {
                        perror("read");
                        exit(EXIT_FAILURE);
                }

                /* invio nome al server */
                if (write(sd, annata, strlen(nome)) < 0)
                {
                        perror("write");
                        exit(EXIT_FAILURE);
                }

                /* ricevo output grep */
                while ((nread = read(sd, buff, sizeof(buff))) > 0)
                {
                        if (write(1, buff, nread) < 0)
                        {
                                perror("write");
                                exit(EXIT_FAILURE);
                        }
                }

        } /* while esterno */

        /* freeaddrinfo lo faccio fuori dal ciclo perché la chiusura della connessione avviene lato server
		 * ma una volta terminata una richiesta ne voglio mandare altre quindi riuso l'indirizzo ottenuto
		 * dalla connessione con fallback e lo libero una volta terminata l'esecuzione */
        freeaddrinfo(res);

} /*main*/

