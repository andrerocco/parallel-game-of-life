#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "gol.h"

pthread_mutex_t matrix_mutex;
int row; int collumn;

int main(int argc, char **argv)
{
    int size, steps;
    cell_t **prev, **next, **tmp;
    FILE *f;
    stats_t stats_total = {0, 0, 0, 0};

    if (argc != 3)
    {
        printf("ERRO! Você deve digitar %s <nome do arquivo do tabuleiro> <numero de threads>!\n\n", argv[0]);
        return 0;
    }

    if ((f = fopen(argv[1], "r")) == NULL)
    {
        printf("ERRO! O arquivo de tabuleiro '%s' não existe!\n\n", argv[1]);
        return 0;
    }

    // TODO - conferir o terceiro argumento de threads

    int num_threads = atoi(argv[2]);
    pthread_t threads[num_threads];

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
    pthread_mutex_init(&matrix_mutex, NULL);

    for (int i = 0; i < steps; i++)
    {
        for (int k = 0; k < num_threads; k++) {
            args args_ = {prev, next, size};
            pthread_create(&threads[i], NULL, routine, &args_);
            printf("criou uma thread\n");
        }
        for (int k = 0; k < num_threads; k++) {
            stats_t* stats_thread;
            pthread_join(threads[k], (void**) &stats_thread);
            stats_total.borns += stats_thread -> borns;
            stats_total.survivals += stats_thread -> survivals;
            stats_total.loneliness += stats_thread -> loneliness;
            stats_total.overcrowding += stats_thread -> overcrowding;
            //free(stats_thread);
        }

        row = 0;
        collumn = 0;

        printf("\n");
        print_board(next, size);
        printf("\n");

#ifdef DEBUG
        printf("Step %d ----------\n", i + 1);
        print_board(next, size);
        print_stats(stats_step);
#endif
        tmp = next;
        next = prev;
        prev = tmp;
    }

    pthread_mutex_destroy(&matrix_mutex);

#ifdef RESULT
    printf("Final:\n");
    print_board(prev, size);
    print_stats(stats_total);
#endif

    free_board(prev, size);
    free_board(next, size);
    printf("fim");
}
