/*
 * The Game of Life
 *
 * RULES:
 *  1. A cell is born, if it has exactly three neighbours.
 *  2. A cell dies of loneliness, if it has less than two neighbours.
 *  3. A cell dies of overcrowding, if it has more than three neighbours.
 *  4. A cell survives to the next generation, if it does not die of lonelines or overcrowding.
 *
 * In this version, a 2D array of ints is used.  A 1 cell is on, a 0 cell is off.
 * The game plays a number of steps (given by the input), printing to the screen each time.
 * A 'x' printed means on, space means off.
 *
 */

#include <stdlib.h>
#include <pthread.h>
#include "gol.h"

/* Statistics */
stats_t statistics;


cell_t **allocate_board(int size)
{
    cell_t **board = (cell_t **)malloc(sizeof(cell_t *) * size);
    int i;
    for (i = 0; i < size; i++)
        board[i] = (cell_t *)malloc(sizeof(cell_t) * size);
    
    statistics.borns = 0;
    statistics.survivals = 0;
    statistics.loneliness = 0;
    statistics.overcrowding = 0;

    return board;
}

void free_board(cell_t **board, int size)
{
    int i;
    for (i = 0; i < size; i++)
        free(board[i]);
    free(board);
}

int adjacent_to(cell_t **board, int size, int i, int j)
{
    int k, l, count = 0;

    int sk = (i > 0) ? i - 1 : i;
    int ek = (i + 1 < size) ? i + 1 : i;
    int sl = (j > 0) ? j - 1 : j;
    int el = (j + 1 < size) ? j + 1 : j;

    for (k = sk; k <= ek; k++)
        for (l = sl; l <= el; l++)
            count += board[k][l];
    count -= board[i][j];

    return count;
}

void* play(void* arg)
{
    arguments_t *args = (arguments_t *)arg;

    cell_t ***board = args->board;  // Acessa o membro board da estrutura
    cell_t ***newboard = args->newboard;  // Acessa o membro newboard da estrutura
    int board_size = args->board_size;  // Acessa o membro board_size da estrutura

    int start_index = args->start_index;  // Acessa o membro start_index da estrutura
    int end_index = args->end_index;  // Acessa o membro end_index da estrutura

    stats_t* stats = &(args->chunk_stats);  // Acessa o membro stats da estrutura    

    
    /* >>> Loop de calculo das células do chunk <<< */
    // i deve ser <= a end_index, pois end_index é o último índice do chunk
    for (int i = start_index; i <= end_index; i++) {
        int x = i / board_size;
        int y = i % board_size;

        int a = adjacent_to(*board, board_size, x, y);

        if((*board)[x][y]) {  // if cell is alive
            if (a < 2) {  // death: loneliness
                (*newboard)[x][y] = 0;
                stats->loneliness++;
            }
            else {  // survival
                if (a == 2 || a == 3) {
                    (*newboard)[x][y] = (*board)[x][y];
                    stats->survivals++;
                }
                else {  // death: overcrowding
                    if (a > 3) {
                        (*newboard)[x][y] = 0;
                        stats->overcrowding++;
                    }
                }
            }
        }
        else {  // if cell is dead 
            if (a == 3) {  // new born
                (*newboard)[x][y] = 1;
                stats->borns++;
            }
            else  // stay unchanged
                (*newboard)[x][y] = (*board)[x][y];
        }
    }


    /* arguments_t *args = (arguments_t *)arg;

    cell_t **board = args->board;  // Acesse o membro board da estrutura
    cell_t **newboard = args->newboard;  // Acesse o membro newboard da estrutura
    int size = args->size;  // Acesse o membro size da estrutura
    stats_t *stats = args->stats;  // Acesse o membro stats da estrutura

    // Linha e coluna precisam ser protegidas para que duas threads não operem na mesma célula
    int i, j, a;  // Linha, coluna e vizinhos

    while (linha_atual < size) {
        pthread_mutex_lock(&matrix_mutex);
        i = linha_atual;
        j = coluna_atual;
        coluna_atual++;  // Incrementa globalmente para que a próxima thread não opere na mesma célula

        if (coluna_atual >= size) {
            coluna_atual = 0;
            linha_atual++;
        }
        pthread_mutex_unlock(&matrix_mutex);

        if (i >= size) break;

        // for each cell, apply the rules of Life
        a = adjacent_to(board, size, i, j);

        if(board[i][j]) {  // if cell is alive
            if (a < 2) {  // death: loneliness
                newboard[i][j] = 0;
                pthread_mutex_lock(&stats_mutex);
                stats->loneliness++;
                pthread_mutex_unlock(&stats_mutex);
            }
            else {  // survival
                if (a == 2 || a == 3) {
                    newboard[i][j] = board[i][j];
                    pthread_mutex_lock(&stats_mutex);
                    stats->survivals++;
                    pthread_mutex_unlock(&stats_mutex);
                }
                else {  // death: overcrowding
                    if (a > 3) {
                        newboard[i][j] = 0;
                        pthread_mutex_lock(&stats_mutex);
                        stats->overcrowding++;
                        pthread_mutex_unlock(&stats_mutex);
                    }
                }
            }
        }
        else {  // if cell is dead 
            if (a == 3) {  // new born
                newboard[i][j] = 1;
                pthread_mutex_lock(&stats_mutex);
                stats->borns++;
                pthread_mutex_unlock(&stats_mutex);
            }
            else  // stay unchanged
                newboard[i][j] = board[i][j];
        }
    } */

    return NULL;
}

void print_board(cell_t **board, int size)
{
    int i, j;
    /* for each row */
    for (j = 0; j < size; j++)
    {
        /* print each column position... */
        for (i = 0; i < size; i++)
            printf("%c", board[i][j] ? 'x' : ' ');
        /* followed by a carriage return */
        printf("\n");
    }
}

void print_stats(stats_t stats)
{
    /* print final statistics */
    printf("Statistics:\n\tBorns..............: %u\n\tSurvivals..........: %u\n\tLoneliness deaths..: %u\n\tOvercrowding deaths: %u\n\n",
        stats.borns, stats.survivals, stats.loneliness, stats.overcrowding);
}

void read_file(FILE *f, cell_t **board, int size)
{
    char *s = (char *) malloc(size + 10);

    /* read the first new line (it will be ignored) */
    fgets(s, size + 10, f);

    /* read the life board */
    for (int j = 0; j < size; j++)
    {
        /* get a string */
        fgets(s, size + 10, f);

        /* copy the string to the life board */
        for (int i = 0; i < size; i++)
            board[i][j] = (s[i] == 'x');
    }

    free(s);
}