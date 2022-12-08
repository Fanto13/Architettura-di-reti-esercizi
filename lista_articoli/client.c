#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define DIM 4096

int main(int argc, char** argv){
        int err;
        struct addrinfo hints;
        struct addrinfo *res,*ptr;
        char *host_remoto;
        char *servizio_remoto;
        int sd, nread;
        char buf[DIM], email[DIM], password[DIM], categoria[DIM];
        int i=1;

        /* Controllo argomenti */
        if (argc < 3){
                printf("Uso: lista_articoli <server> <porta>\n");
                exit(EXIT_FAILURE);
        }
        //lettura dati dall'utente 
        printf("inserisci indirizzo email:\n");
        scanf("%s,", email);
        printf("inserisci password:\n");
        scanf("%s,", password);
        
       
       
        /* Costruzione dell'indirizzo */
        memset(&hints, 0, sizeof(hints));
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        /* Risoluzione dell'host */
        host_remoto = argv[1];
        servizio_remoto = argv[2];

        if ((err = getaddrinfo(host_remoto, servizio_remoto, &hints, &res)) != 0) {
                fprintf(stderr, "Errore risoluzione nome: %s\n", gai_strerror(err));
                exit(EXIT_FAILURE);
        }

        /* Connessione con fallback */
        for (ptr = res; ptr != NULL; ptr = ptr->ai_next){
                /*se socket fallisce salto direttamente alla prossima iterazione*/
                if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0){
                        fprintf(stderr,"creazione socket fallita\n");
                        continue;
                }
                /*se connect funziona esco dal ciclo*/
                if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0){
                        printf("connect riuscita al tentativo %d\n",i);
                        break;
                }
                i++;
                close(sd);
        }

        /* Verifica sul risultato restituito da getaddrinfo */
        if (ptr == NULL) {
                fprintf(stderr, "Errore risoluzione nome: nessun indirizzo corrispondente trovato\n");
                exit(EXIT_FAILURE);
        }

        /* Liberiamo la memoria allocata da getaddrinfo() */
        freeaddrinfo(res);

        /* Scambio dati col server */

        /* Mando l'indirizzo email al server */
        if(write(sd, email, strlen(email))<0){
                perror("write email");
                exit(EXIT_FAILURE);
        }
        //leggo l'ack
        memset(buf, 0, sizeof(buf));

        if(read(sd, buf, DIM)<0){
                perror("read ack");
                exit(EXIT_FAILURE);
        }
         //leggo l'ack
        memset(buf, 0, sizeof(buf));
        //il server poi invierà il suo ack 
        if(strcmp(buf, "ack\n")!=0){
                close(sd);
                printf("errore dell'ack\n");
                exit(EXIT_FAILURE);
        }

        if(write(sd, password, strlen(password))<0){
                perror("write password");
                exit(EXIT_FAILURE);
        }

          if(read(sd, buf, DIM)<0){
                perror("read ack");
                exit(EXIT_FAILURE);
        }
        
        //il server poi invierà il suo ack 
        if(strcmp(buf, "ack\n")!=0){
                close(sd);
                printf("errore dell'ack\n");
                exit(EXIT_FAILURE);
        }
        /* Ricezione risultato */

        /* È consigliabile effettuare il flushing del buffer di printf
         * prima di iniziare a scrivere sullo standard output con write */
        fflush(stdout);

        memset(buf, 0, sizeof(buf));
        while ((nread = read(sd, buf, DIM)) > 0) {//scrive l'output reindirizzato dal server
                if (write(1, buf, nread) < 0) {
                        perror("write su stdout");
                        exit(EXIT_FAILURE);
                }
        }
        printf("%d",nread);

        /* Controllo errori di lettura */
        if (nread < 0) {
                perror("read del risultato");
                exit(EXIT_FAILURE);
        }

        /* chiudo la socket */
        close(sd);

        return 0;
}
