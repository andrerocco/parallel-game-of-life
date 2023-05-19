#include <stdio.h>
#include <stdlib.h>  // atoi
#include <pthread.h>
#include "gol.h"

// Variáveis globais
int linha_atual = 0;
int coluna_atual = 0;


int main(int argc, char **argv)
{
    int size, steps;
    cell_t **prev, **next, **tmp;
    FILE *f;
    stats_t stats_total = {0, 0, 0, 0};
    stats_t* thread_stats;
    interval_t* intervals;

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
    thread_stats = malloc(sizeof(stats_t) * n_threads);
    intervals = malloc(sizeof(interval_t) * n_threads);
    

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

    // After the first iteration of the loop, the board gets
    // reset to the initial matrix
    for (int i = 0; i < steps; i++)
    {
        int parcel = size / n_threads;
        int leftovers = size % n_threads;
        printf("parcel: %d, leftovers: %d\n", parcel, leftovers);
        arguments_t arguments = {prev, next, size, NULL, NULL};
        for (int i = 0; i < n_threads; i++)
        {
            /* Calculando intervalo*/
            if (i)
            {
                intervals[i].start = intervals[i-1].end;
                intervals[i].end = intervals[i].start + parcel;
            }
            else {
                intervals[i].start = 0;
                intervals[i].end = parcel;
            }
            if (leftovers)
                {
                    intervals[i].start += i;
                    intervals[i].end++;
                    leftovers--;
                }
            /**/
            printf("start: %d, end: %d\n", intervals[i].start, intervals[i].end);

            thread_stats[i] = (stats_t){0, 0, 0, 0};
            arguments.stats = &thread_stats[i];
            arguments.interval = &intervals[i];
            
            pthread_create(&threads[i], NULL, play, &arguments);
        }
        for (int i = 0; i < n_threads; i++)
        {
            pthread_join(threads[i], NULL);
            stats_total.borns += thread_stats[i].borns;
            stats_total.survivals += thread_stats[i].survivals;
            stats_total.loneliness += thread_stats[i].loneliness;
            stats_total.overcrowding += thread_stats[i].overcrowding;
        }
        

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

    free(thread_stats);
    free(intervals);

#ifdef RESULT
    printf("Final:\n");
    print_board(prev, size);
    print_stats(stats_total);
#endif

    free_board(prev, size);
    free_board(next, size);
}