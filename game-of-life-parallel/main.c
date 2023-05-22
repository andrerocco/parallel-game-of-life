#include <stdio.h>
#include <stdlib.h>  // atoi
#include <pthread.h>
#include "gol.h"


int main(int argc, char **argv)
{
    int size, steps;
    cell_t **prev, **next, **tmp;
    FILE *f;
    stats_t stats_total = {0, 0, 0, 0};
    stats_t stats_step = {0, 0, 0, 0};  // Utilizado apenas para fins de printing

    /* >>> Validação e recebimento de argumentos <<< */
    if (argc != 3) {
        printf("ERRO! Você deve digitar %s <nome do arquivo do tabuleiro> <número de threads>!\n\n", argv[0]);
        return 0;
    }
    if ((f = fopen(argv[1], "r")) == NULL) {
        printf("ERRO! O arquivo de tabuleiro '%s' não existe!\n\n", argv[1]);
        return 0;
    }
    if (atoi(argv[2]) < 1) {
        printf("ERRO! O número de threads deve ser maior que 0!\n\n");
        return 0;
    }

    fscanf(f, "%d %d", &size, &steps);

    int full_size = size * size;
    int n_threads = atoi(argv[2]);

    if (n_threads > full_size)
        n_threads = full_size;
    /* <<< Validação e recebimento de argumentos >>> */

    /* >>> Inicialização do tabuleiro <<< */
    prev = allocate_board(size);
    next = allocate_board(size);
    read_file(f, prev, size);
    fclose(f);

    #ifdef DEBUG
    printf("Initial:\n");
    print_board(prev, size);
    print_stats(stats_step);
    #endif
    /* <<< Inicialização do tabuleiro >>> */

    /* >>> Divisão dos chunks <<< */
    arguments_t arguments[n_threads];

    int current_index = 0;
    int chunk_size = full_size / n_threads;
    int remainder = full_size % n_threads;

    #ifdef DEBUG
    printf("\n\n");
    printf("Total size: %d\n", full_size);
    printf("Number of threads: %d\n", n_threads);
    printf("Chunk size: %d\n", chunk_size);
    printf("Remainder: %d\n", remainder);
    #endif

    for (int i = 0; i < n_threads; i++) {
        arguments[i].start_index = current_index;
        current_index += chunk_size;

        /* >>> Alocação do chunk <<< */
        if (i < remainder) {
            current_index++;
        }
        arguments[i].end_index = current_index-1;
        
        #ifdef DEBUG
        printf("Thread %d: %d-%d\n", i+1, arguments[i].start_index, arguments[i].end_index);
        #endif
        /* <<< Alocação do chunk >>> */

        /* >>> Alocação do outros argumentos <<< */        
        // É passado o endereço da matriz para que uma mudança na matriz seja refletida na matriz
        // de cada thread e mudanças na matriz de cada thread sejam refletidas na matriz original
        arguments[i].board = &prev;
        arguments[i].newboard = &next;
        arguments[i].board_size = size;
        arguments[i].chunk_stats = (stats_t){0, 0, 0, 0};
        /* <<< Alocação do outros argumentos >>> */
    }
    /* <<< Divisão dos chunks >>> */
    
    #ifdef DEBUG
    printf("\n\n");
    #endif


    pthread_t threads[n_threads];

    for (int i = 0; i < steps; i++)
    {
        for (int i = 0; i < n_threads; i++)
            pthread_create(&threads[i], NULL, play, &arguments[i]);
        for (int i = 0; i < n_threads; i++)
            pthread_join(threads[i], NULL);

        for (int i = 0; i < n_threads; i++) {
            /* printf(
                "Thread %d: borns: %d, survivals: %d, loneliness: %d, overcrowding: %d\n", 
                i+1,
                arguments[i].chunk_stats.borns,
                arguments[i].chunk_stats.survivals,
                arguments[i].chunk_stats.loneliness,
                arguments[i].chunk_stats.overcrowding
            ); */

            stats_step.borns += arguments[i].chunk_stats.borns;
            stats_step.survivals += arguments[i].chunk_stats.survivals;
            stats_step.loneliness += arguments[i].chunk_stats.loneliness;
            stats_step.overcrowding += arguments[i].chunk_stats.overcrowding;
            arguments[i].chunk_stats = (stats_t){0, 0, 0, 0};
        }
        
        stats_total.borns += stats_step.borns;
        stats_total.survivals += stats_step.survivals;
        stats_total.loneliness += stats_step.loneliness;
        stats_total.overcrowding += stats_step.overcrowding;

        #ifdef DEBUG
        printf("Step %d ----------\n", i + 1);
        // print_board(prev, size);
        print_board(next, size);
        print_stats(stats_step);
        #endif
        
        tmp = next;
        next = prev;
        prev = tmp;

        stats_step = (stats_t){0, 0, 0, 0};
    }


    #ifdef RESULT
    printf("Final:\n");
    print_board(prev, size);
    print_stats(stats_total);
    #endif

    free_board(prev, size);
    free_board(next, size);
}
