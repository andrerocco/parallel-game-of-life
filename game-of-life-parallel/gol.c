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
/* Matrix Mutex */
extern pthread_mutex_t matrix_mutex;
/* Row */
int row;
/* Collumn */
int collumn;

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

void* routine(void* arg) {
    args* args_ptr = (args*) arg;
    int a;
    int my_row; int my_collumn;
    int* stats_ = malloc(sizeof(stats_t)*4);
    while (row < args_ptr -> size) {

        pthread_mutex_lock(&matrix_mutex);
        my_row = row;
        my_collumn = collumn;

        collumn++;
        if (collumn >= args_ptr -> size)
        {
            collumn = 0;
            row++;
        }
        pthread_mutex_unlock(&matrix_mutex);

        printf("i: %d, j: %d\n", my_row, my_collumn);

        if (my_row >= args_ptr -> size) break;

        a = adjacent_to(args_ptr -> board, args_ptr -> size, my_row, my_collumn);

            /* if cell is alive */
            if(args_ptr -> board[my_row][my_collumn]) 
            {
                /* death: loneliness */
                if(a < 2) {
                    args_ptr -> newboard[my_row][my_collumn] = 0;
                    stats_[2]++;
                }
                else
                {
                    /* survival */
                    if(a == 2 || a == 3)
                    {
                        args_ptr -> newboard[my_row][my_collumn] = args_ptr -> board[my_row][my_collumn];
                        stats_[3]++;
                    }
                    else
                    {
                        /* death: overcrowding */
                        if(a > 3)
                        {
                            args_ptr -> newboard[my_row][my_collumn] = 0;
                            stats_[1]++;
                        }
                    }
                }
             } 
             else /* if cell is dead */
            {
                if(a == 3) /* new born */
                {
                    args_ptr -> newboard[my_row][my_collumn] = 1;
                    stats_[0]++;
                }
                else /* stay unchanged */
                    args_ptr -> newboard[my_row][my_collumn] = args_ptr -> board[my_row][my_collumn];
            }
    }
    pthread_exit((void*) &stats_);
}

stats_t play(cell_t **board, cell_t **newboard, int size)
{
    int i, j, a;

    stats_t stats = {0, 0, 0, 0};

    /* for each cell, apply the rules of Life */
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            a = adjacent_to(board, size, i, j);

            /* if cell is alive */
            if(board[i][j]) 
            {
                /* death: loneliness */
                if(a < 2) {
                    newboard[i][j] = 0;
                    stats.loneliness++;
                }
                else
                {
                    /* survival */
                    if(a == 2 || a == 3)
                    {
                        newboard[i][j] = board[i][j];
                        stats.survivals++;
                    }
                    else
                    {
                        /* death: overcrowding */
                        if(a > 3)
                        {
                            newboard[i][j] = 0;
                            stats.overcrowding++;
                        }
                    }
                }
                
            }
            else /* if cell is dead */
            {
                if(a == 3) /* new born */
                {
                    newboard[i][j] = 1;
                    stats.borns++;
                }
                else /* stay unchanged */
                    newboard[i][j] = board[i][j];
            }
        }
    }

    return stats;
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