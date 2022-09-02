#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>

// Lê o conteúdo do arquivo filename e retorna um vetor E o tamanho dele
// Se filename for da forma "gen:%d", gera um vetor aleatório com %d elementos
//
// +-------> retorno da função, ponteiro para vetor malloc()ado e preenchido
// | 
// |         tamanho do vetor (usado <-----+
// |         como 2o retorno)              |
// v                                       v
double* load_vector(const char* filename, int* out_size);

// Avalia se o prod_escalar é o produto escalar dos vetores a e b. Assume-se
// que ambos a e b sejam vetores de tamanho size.
void avaliar(double* a, double* b, int size, double prod_escalar);
void* thread(void* arg);

typedef struct vetores{
    int start, stop;
    double* a;
    double* b;
    double* result;
} vetores;

int main(int argc, char* argv[]) {
    srand(time(NULL));

    //Temos argumentos suficientes?
    if(argc < 4) {
        printf("Uso: %s n_threads a_file b_file\n"
               "    n_threads    número de threads a serem usadas na computação\n"
               "    *_file       caminho de arquivo ou uma expressão com a forma gen:N,\n"
               "                 representando um vetor aleatório de tamanho N\n", 
               argv[0]);
        return 1;
    }
  
    //Quantas threads?
    int n_threads = atoi(argv[1]);
    if (!n_threads) {
        printf("Número de threads deve ser > 0\n");
        return 1;
    }
    //Lê números de arquivos para vetores alocados com malloc
    int a_size = 0, b_size = 0;
    double* a = load_vector(argv[2], &a_size);
    if (!a) {
        //load_vector não conseguiu abrir o arquivo
        printf("Erro ao ler arquivo %s\n", argv[2]);
        return 1;
    }
    double* b = load_vector(argv[3], &b_size);
    if (!b) {
        printf("Erro ao ler arquivo %s\n", argv[3]);
        return 1;
    }
    
    //Garante que entradas são compatíveis
    if (a_size != b_size) {
        printf("Vetores a e b tem tamanhos diferentes! (%d != %d)\n", a_size, b_size);
        return 1;
    }

    //Calcula produto escalar. Paralelize essa parte
    n_threads = ((n_threads > a_size) ? a_size : n_threads);
    vetores vet[n_threads];
    pthread_t threads[n_threads];

    // descobre quantas iteracoes minimas uma thread precisa
    // o resto sempre vai ser menor que o n_threads logo todas precisam do minimo e algumas precisam de minimo+1 
    // por exemplo 5 threads e vetores com size 11, 11/5 é 2 com resto 1
    // por isso todas threads precisam de 2 e uma delas precisa de 2+1
    // a distribuição de um a um das iterações ficaria 3 2 2 2 2 (iterações para cada thread)
    int iteracoes = floor(a_size/n_threads);
    int resto = a_size - (iteracoes*n_threads);

    //variavel V tem dupla função
    //ela marca pra próxima iteração quantas vezes ja foi somado o resto
    //ao mesmo tempo que soma quantas iterações extras ja foram para poder somar como desvio
    //ex: 4 threads e 10 de size
    // [0,3), [3, 6), [6, 8), [8, 10) 
    // o minimo é 2 para cada thread, porém a primeira e a segunda receberam um a mais
    // portanto é necessário ''desviar'' todos os próximos em 2 
    int v = 0;
    double result = 0;
    double* presult = &result;

    for (int i = 0; i < n_threads; i++) {
        vet[i].start = i*iteracoes + v;
        if (resto > 0) {
            v ++;
        } else {
            v = v;
        }
        vet[i].stop = ((i+1)*iteracoes) + v;
        vet[i].a = a;
        vet[i].b = b;
        vet[i].result = presult;
        resto--;
        pthread_create(&threads[i], NULL, thread, (void *)&vet[i]);
    }    

    for (int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    //    +---------------------------------+
    // ** | IMPORTANTE: avalia o resultado! | **
    //    +---------------------------------+
    avaliar(a, b, a_size, result);

    //Libera memória
    free(a);
    free(b);

    return 0;
}

void* thread(void* vetor) {
    vetores *vet = (vetores*) vetor;
    int inicio = vet->start;
    int fim = vet->stop;
    for (int i = inicio; i < fim; i++) {
        *(vet->result) += (vet->a)[i] * (vet->b)[i];
    }
    pthread_exit(NULL);
}