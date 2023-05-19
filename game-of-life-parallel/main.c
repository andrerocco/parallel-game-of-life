#include <stdio.h>
#include <stdlib.h>  // atoi
#include <pthread.h>
#include "gol.h"

// Variáveis globais
int linha_atual = 0;
int coluna_atual = 0;
pthread_mutex_t matrix_mutex;
pthread_mutex_t stats_mutex;


int main(int argc, char **argv)
{
    int size, steps;
    cell_t **prev, **next, **tmp;
    FILE *f;
    stats_t stats_total = {0, 0, 0, 0};
    stats_t stats_step = {0, 0, 0, 0};

    if (argc != 3)
    {
        printf("ERRO! Você deve digitar %s <nome do arquivo do tabuleiro> <número de threads>!\n\n", argv[0]);
        return 0;
    }

    if ((f = fopen(argv[1], "r")) == NULL)
    {
        printf("ERRO! O arquivo de tabuleiro '%s' não existe!\n\n", argv[1]);
        return 0;
    }
    if (atoi(argv[2]) < 1)
    {
        printf("ERRO! O número de threads deve ser maior que 0!\n\n");
        return 0;
    }

    int n_threads = atoi(argv[2]);

    fscanf(f, "%d %d", &size, &steps);

    prev = allocate_board(size);
    next = allocate_board(size);

    read_file(f, prev, size);

    fclose(f);

#ifdef DEBUG
    printf("Initial:\n");
    print_board(prev, size);
    print_stats(stats_step);
#endif

    pthread_t threads[n_threads];

    pthread_mutex_init(&matrix_mutex, NULL);
    pthread_mutex_init(&stats_mutex, NULL);

    // After the first iteration of the loop, the board gets
    // reset to the initial matrix
    for (int i = 0; i < steps; i++)
    {
        stats_step = (stats_t){0, 0, 0, 0};
        arguments_t arguments = {prev, next, size, &stats_step};

        for (int i = 0; i < n_threads; i++)
            pthread_create(&threads[i], NULL, play, &arguments);
        for (int i = 0; i < n_threads; i++)
            pthread_join(threads[i], NULL);
        
        coluna_atual = 0;
        linha_atual = 0;
        
        stats_total.borns += stats_step.borns;
        stats_total.survivals += stats_step.survivals;
        stats_total.loneliness += stats_step.loneliness;
        stats_total.overcrowding += stats_step.overcrowding;

#ifdef DEBUG
        printf("Step %d ----------\n", i + 1);
        print_board(prev, size);
        print_board(next, size);
        print_stats(stats_step);
#endif
        tmp = next;
        next = prev;
        prev = tmp;
    }

    pthread_mutex_destroy(&matrix_mutex);
    pthread_mutex_destroy(&stats_mutex);

#ifdef RESULT
    printf("Final:\n");
    print_board(prev, size);
    print_stats(stats_total);
#endif

    free_board(prev, size);
    free_board(next, size);
}
