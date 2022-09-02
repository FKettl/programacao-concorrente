#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

//                          (principal)
//                               |
//              +----------------+--------------+
//              |                               |
//           filho_1                         filho_2
//              |                               |
//    +---------+-----------+          +--------+--------+
//    |         |           |          |        |        |
// neto_1_1  neto_1_2  neto_1_3     neto_2_1 neto_2_2 neto_2_3

// ~~~ printfs  ~~~
//      principal (ao finalizar): "Processo principal %d finalizado\n"
// filhos e netos (ao finalizar): "Processo %d finalizado\n"
//    filhos e netos (ao inciar): "Processo %d, filho de %d\n"

// Obs:
// - netos devem esperar 5 segundos antes de imprmir a mensagem de finalizado (e terminar)
// - pais devem esperar pelos seu descendentes diretos antes de terminar

int main(int argc, char** argv) {

    // ....

    /*************************************************
     * Dicas:                                        *
     * 1. Leia as intruções antes do main().         *
     * 2. Faça os prints exatamente como solicitado. *
     * 3. Espere o término dos filhos                *
     *************************************************/
    int pid;
    int pid2;

    for (int i = 0; i < 2; i++) {
        pid = fork();
        if(!pid) {
            printf("Processo %d, filho de %d\n", getpid(), getppid());
            fflush(stdout);
            /* Garante que o filho criado não executa as próximas iterações do for. */
            break;
        }   
    }

    /* Garante que apenas a primeira geração de filhos entre no segundo for. */
    if (!pid) {
        for (int i = 0; i < 3; i++) {
            pid2 = fork();
            if(!pid2) {
                printf("Processo %d, filho de %d\n", getpid(), getppid());
                fflush(stdout);
                sleep(5);
                /* Garante que o filho criado não executa as próximas iterações do for. */
                break;
            }
        }
    }
    while(wait(NULL) >= 0);
    if (pid) {
        printf("Processo principal %d finalizado\n", getpid());    
        fflush(stdout);
    } else {
        printf("Processo %d finalizado\n", getpid());   
    }
    return 0;
}
