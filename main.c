/*FORMOSI DOMENICO 0001124248*/
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/select.h>
#include "parser.h"
#define MAX_SIZE 256
#define max(a, b) ((a) > (b) ? (a) : (b))
#define is_alfabetico(a) (((a) >= 'A' && (a) <= 'Z') || ((a) >= 'a' && (a) <= 'z') ? (1) : (0))
#define is_vocale(a) ((a) == 'a' || (a) == 'e' || (a) == 'i' || (a) == 'o' || (a) == 'u' || (a) == 'A' || (a) == 'E' || (a) == 'I' || (a) == 'O' || (a) == 'U' ? (1) : (0))
#define is_numerico(a) ((a)>='0' && (a)<='9' ? (1) : (0))
#define BUFF_SIZE 1024

void gestore(int signo)
{
    int stato;
    printf("esecuzione gestore di SIGCHLD\n");
    wait(&stato);
}
/********************************************************/

int main(int argc, char **argv)

{
    int listenfd, connfd, udpfd, fd, fd_temp, nready, maxfdp1;
    const int on = 1;
    fd_set rset;
    int len, nread, nwrite, num, port;
    struct sockaddr_in cliaddr, servaddr;
    DIR *dir;
    struct dirent *dd;
    char direttorio[FILENAME_MAX], path[FILENAME_MAX], buff[BUFF_SIZE], filename[FILENAME_MAX], c;

    /* CONTROLLO ARGOMENTI ---------------------------------- */
    if (argc != 2)
    {
        printf("Error: %s port\n", argv[0]);
        exit(1);
    }

    nread = 0;
    while (argv[1][nread] != '\0')
    {
        if (!is_numerico(argv[1][nread]))
        {
            printf("Primo argomento non intero\n");
            exit(2);
        }
        nread++;
    }
    port = atoi(argv[1]);
    if (port < 1024 || port > 65535)
    {
        printf("Porta scorretta...");
        exit(2);
    }

    /* INIZIALIZZAZIONE INDIRIZZO SERVER E BIND ---------------------------- */
    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    printf("Server avviato\n");

    /* CREAZIONE SOCKET TCP ------------------------------------------------ */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        perror("apertura socket TCP ");
        exit(1);
    }
    printf("Creata la socket TCP d'ascolto, fd=%d\n", listenfd);

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("set opzioni socket TCP");
        exit(2);
    }
    printf("Set opzioni socket TCP ok\n");

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind socket TCP");
        exit(3);
    }
    printf("Bind socket TCP ok\n");

    if (listen(listenfd, 5) < 0)
    {
        perror("listen");
        exit(4);
    }
    printf("Listen ok\n");

    /* CREAZIONE SOCKET UDP ------------------------------------------------ */
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpfd < 0)
    {
        perror("apertura socket UDP");
        exit(5);
    }
    printf("Creata la socket UDP, fd=%d\n", udpfd);

    if (setsockopt(udpfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("set opzioni socket UDP");
        exit(6);
    }
    printf("Set opzioni socket UDP ok\n");

    if (bind(udpfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind socket UDP");
        exit(7);
    }
    printf("Bind socket UDP ok\n");

    /* AGGANCIO GESTORE PER EVITARE FIGLI ZOMBIE -------------------------------- */
    signal(SIGCHLD, gestore);

    /* PULIZIA E SETTAGGIO MASCHERA DEI FILE DESCRIPTOR ------------------------- */
    FD_ZERO(&rset);
    maxfdp1 = max(listenfd, udpfd) + 1;

    /* CICLO DI RICEZIONE EVENTI DALLA SELECT ----------------------------------- */
    for (;;)
    {
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);

        printf("In attesa di un evento..\n");
        if ((nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0)
        {
            if (errno == EINTR)
                continue;
            else
            {
                perror("select");
                exit(8);
            }
        }

        /* GESTIONE RICHIESTE TCP ------------------------------------- */
        if (FD_ISSET(listenfd, &rset))
        {
            printf("Ricevuta richiesta TCP\n");
            len = sizeof(struct sockaddr_in);
            if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0)
            {
                if (errno == EINTR)
                    continue;
                else
                {
                    perror("accept");
                    exit(9);
                }
            }

            if (fork() == 0)
            { /* processo figlio che serve la richiesta di operazione */
                close(listenfd);
                printf("Dentro il figlio, pid=%i\n", getpid());

                HttpRequest request;
                request.request_state = STATE_READ_EMPTY;

                while ((nread=read(connfd,path,BUFF_SIZE))>0)
                {
                    printf("rcvd msg\n");
                    request.read_byte = nread;
                    parse_request(path,&request);
                    printf("%i\n",request.request_state);

                    if (request.request_state == STATE_READ_HEADERS)
                    {
                        printf("Receveid request line: %i %s %i\n",request.method,request.resource,request.protocol);
                    }
                }

                //free(request.body);
                //free(request.resource);


                /*la connessione assegnata al figlio viene chiusa*/
                printf("Figlio %i: termino\n", getpid());
                shutdown(connfd, 0);
                shutdown(connfd, 1);
                close(connfd);
                exit(0);
            } // figlio-fork
            close(connfd);
        } /* FINE GESTIONE RICHIESTE TCP ------------------------------------------ */


    } /* ciclo for della select */
    /* NEVER ARRIVES HERE */
    exit(0);
}
