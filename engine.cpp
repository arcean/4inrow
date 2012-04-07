#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include "engine.h"

Engine::Engine(QObject *parent) :
    QObject(parent)
{
    game_in_progress = false;
    move_in_progress = false;
    seed_chosen = false;
    states_allocated = 0;

    int player[2], level[2], turn = 0, num_of_players, move;
    int width, height, num_to_connect;
    int x1, y1, x2, y2;

    width = 20;
    height = 16;
    num_to_connect = 4;
    num_of_players = 1;

    switch (num_of_players) {

        case 0:
            player[0] = player[1] = COMPUTER;
            turn = 0;
            printf("\nUnsupported!\n");
            break;

        case 1:
            player[0] = HUMAN;
            player[1] = COMPUTER;
            level[1] = 5; // AI skill
            turn = 0; // HUMAN first
            break;

        case 2:
            player[0] = player[1] = HUMAN;
            turn = 0;
            break;
    }
    newGame(width, height, num_to_connect);
    //poll(print_dot, CLOCKS_PER_SEC/2);

    do {
        if (player[turn] == HUMAN) {
            do {
                move = 2; // NUM OF THE COLUMN !!
            }
            while (!makeMove(turn, move, NULL));
        }
        else {
            /* move + 1 = column chosen by AI. */
            autoMove(turn, level[turn], &move, NULL);
        }
        turn = !turn;
    } while (!isWinner(0) && !isWinner(1) && !isTie());

    if (isWinner(0)) {
        if (num_of_players == 1)
            printf("You won!");
        else
            printf("Player won!");
        winCoords(&x1, &y1, &x2, &y2);
        printf("  (%d,%d) to (%d,%d)\n\n", x1+1, y1+1, x2+1, y2+1);
    }
    else if (isWinner(1)) {
        if (num_of_players == 1)
            printf("I won!");
        else
            printf("Player won!");
        winCoords(&x1, &y1, &x2, &y2);
        printf("  (%d,%d) to (%d,%d)\n\n", x1+1, y1+1, x2+1, y2+1);
    }
    else {
        printf("There was a tie!\n\n");
    }

    endGame();
}

void Engine::poll(void (*poll_func)(void), clock_t interval)
{
    poll_function = poll_func;
    poll_interval = interval;
}

void Engine::newGame(int width, int height, int num)
{
    register int i, j, k, x;
    int win_index, column;
    int *win_indices;

    assert(!game_in_progress);
    assert(width >= 1 && height >= 1 && num >= 1);

    size_x = width;
    size_y = height;
    total_size = width * height;
    num_to_connect = num;
    magic_win_number = 1 << num_to_connect;
    win_places = numOfWinPlaces(size_x, size_y, num_to_connect);

    /* Set up a random seed for making random decisions when there is */
    /* equal goodness between two moves.                              */

    if (!seed_chosen) {
        srand((unsigned int) time((time_t *) 0));
        seed_chosen = true;
    }

    /* Set up the board */

    depth = 0;
    current_state = &state_stack[0];

    current_state->board = (char **) emalloc(size_x * sizeof(char *));
    for (i = 0; i < size_x; i++) {
        current_state->board[i] = (char *) emalloc(size_y);
        for (j = 0; j < size_y; j++)
            current_state->board[i][j] = NONE;
    }

    /* Set up the score array */

    current_state->scoreArray[0] = (int *) emalloc(win_places * sizeof(int));
    current_state->scoreArray[1] = (int *) emalloc(win_places * sizeof(int));
    for (i = 0; i < win_places; i++) {
        current_state->scoreArray[0][i] = 1;
        current_state->scoreArray[1][i] = 1;
    }

    current_state->score[0] = current_state->score[1] = win_places;
    current_state->winner = NONE;
    current_state->numOfPieces = 0;

    states_allocated = 1;

    /* Set up the map */

    map = (int ***) emalloc(size_x * sizeof(int **));
    for (i = 0; i < size_x; i++) {
        map[i] = (int **) emalloc(size_y * sizeof(int *));
        for (j = 0; j<size_y; j++) {
            map[i][j] = (int *) emalloc((num_to_connect*4 + 1) * sizeof(int));
            map[i][j][0] = -1;
        }
    }

    win_index = 0;

    /* Fill in the horizontal win positions */
    for (i = 0; i < size_y; i++)
        for (j = 0; j < size_x-num_to_connect + 1; j++) {
            for (k = 0; k < num_to_connect; k++) {
                win_indices = map[j+k][i];
                for (x = 0; win_indices[x] != -1; x++)
                    ;
                win_indices[x++] = win_index;
                win_indices[x] = -1;
            }
            win_index++;
        }

    /* Fill in the vertical win positions */
    for (i = 0; i < size_x; i++)
        for (j = 0; j < size_y-num_to_connect+1; j++) {
            for (k = 0; k < num_to_connect; k++) {
                win_indices = map[i][j+k];
                for (x = 0; win_indices[x] != -1; x++)
                    ;
                win_indices[x++] = win_index;
                win_indices[x] = -1;
            }
            win_index++;
        }

    /* Fill in the forward diagonal win positions */
    for (i = 0; i < size_y-num_to_connect+1; i++)
        for (j = 0; j < size_x-num_to_connect+1; j++) {
            for (k=0; k < num_to_connect; k++) {
                win_indices = map[j+k][i+k];
                for (x = 0; win_indices[x] != -1; x++)
                    ;
                win_indices[x++] = win_index;
                win_indices[x] = -1;
            }
            win_index++;
        }

    /* Fill in the backward diagonal win positions */
    for (i = 0; i < size_y-num_to_connect+1; i++)
        for (j = size_x-1; j >= num_to_connect-1; j--) {
            for (k = 0; k < num_to_connect; k++) {
                win_indices = map[j-k][i+k];
                for (x = 0; win_indices[x] != -1; x++)
                    ;
                win_indices[x++] = win_index;
                win_indices[x] = -1;
            }
            win_index++;
        }

    /* Set up the order in which automatic moves should be tried. */
    /* The columns nearer to the center of the board are usually  */
    /* better tactically and are more likely to lead to a win.    */
    /* By ordering the search such that the central columns are   */
    /* tried first, alpha-beta cutoff is much more effective.     */

    drop_order = (int *) emalloc(size_x * sizeof(int));
    column = (size_x-1) / 2;
    for (i = 1; i <= size_x; i++) {
        drop_order[i-1] = column;
        column += ((i%2)? i : -i);
    }

    game_in_progress = true;
}

bool Engine::makeMove(int player, int column, int *row)
{
    assert(game_in_progress);
    assert(!move_in_progress);

    if (column >= size_x || column < 0)
        return false;

    int result = dropPiece(real_player(player), column);
    if (row != NULL && result >= 0)
        *row = result;
    return (result >= 0);
}

bool Engine::autoMove(int player, int level, int *column, int *row)
{
    int best_column = -1, goodness = 0, best_worst = -(INT_MAX);
    int num_of_equal = 0, real_player, current_column, result;

    assert(game_in_progress);
    assert(!move_in_progress);
    assert(level >= 1 && level <= MAX_LEVEL);

    real_player = real_player(player);

    /* It has been proven that the best first move for a standard 7x6 game  */
    /* of connect-4 is the center column.  See Victor Allis' masters thesis */
    /* ("ftp://ftp.cs.vu.nl/pub/victor/connect4.ps") for this proof.        */

    if (current_state->numOfPieces < 2 &&
                        size_x == 7 && size_y == 6 && num_to_connect == 4 &&
                        (current_state->numOfPieces == 0 ||
                         current_state->board[3][0] != NONE)) {
        if (column != NULL)
            *column = 3;
        if (row != NULL)
            *row = current_state->numOfPieces;
        dropPiece(real_player, 3);
        return true;
    }

    move_in_progress = true;

    /* Simulate a drop in each of the columns and see what the results are. */

    for (int i=0; i<size_x; i++) {
        pushState();
        current_column = drop_order[i];

        result = dropPiece(real_player, current_column);

        /* If this column is full, ignore it as a possibility. */
        if (result < 0) {
            pop_state();
            continue;
        }

        /* If this drop wins the game, take it! */
        else if (current_state->winner == real_player) {
            best_column = current_column;
            pop_state();
            break;
        }

        /* Otherwise, look ahead to see how good this move may turn out */
        /* to be (assuming the opponent makes the best moves possible). */
        else {
            next_poll = clock() + poll_interval;
            goodness = evaluate(real_player, level, -(INT_MAX), -best_worst);
        }

        /* If this move looks better than the ones previously considered, */
        /* remember it.                                                   */
        if (goodness > best_worst) {
            best_worst = goodness;
            best_column = current_column;
            num_of_equal = 1;
        }

        /* If two moves are equally as good, make a random decision. */
        else if (goodness == best_worst) {
            num_of_equal++;
            if ((rand()>>4) % num_of_equal == 0)
                best_column = current_column;
        }

        pop_state();
    }

    move_in_progress = false;

    /* Drop the piece in the column decided upon. */

    if (best_column >= 0) {
        result = dropPiece(real_player, best_column);
        if (column != NULL)
            *column = best_column;
        if (row != NULL)
            *row = result;
        return true;
    }
    else
        return false;
}

char** Engine::board(void)
{
    assert(game_in_progress);
    return current_state->board;
}

int Engine::scoreOfPlayer(int player)
{
    assert(game_in_progress);
    return current_state->score[real_player(player)];
}

bool Engine::isWinner(int player)
{
    assert(game_in_progress);
    return (current_state->winner == real_player(player));
}

bool Engine::isTie(void)
{
    assert(game_in_progress);
    return (current_state->numOfPieces == total_size &&
            current_state->winner == NONE);
}

void Engine::winCoords(int *x1, int *y1, int *x2, int *y2)
{
    register int i, j, k;
    int winner, win_pos = 0;
    bool found;

    assert(game_in_progress);

    winner = current_state->winner;
    assert(winner != NONE);

    while (current_state->scoreArray[winner][win_pos] != magic_win_number)
        win_pos++;

    /* Find the lower-left piece of the winning connection. */

    found = false;
    for (j=0; j<size_y && !found; j++)
        for (i=0; i<size_x && !found; i++)
            for (k=0; map[i][j][k] != -1; k++)
                if (map[i][j][k] == win_pos) {
                    *x1 = i;
                    *y1 = j;
                    found = true;
                    break;
                }

    /* Find the upper-right piece of the winning connection. */

    found = false;
    for (j=size_y-1; j>=0 && !found; j--)
        for (i=size_x-1; i>=0 && !found; i--)
            for (k=0; map[i][j][k] != -1; k++)
                if (map[i][j][k] == win_pos) {
                    *x2 = i;
                    *y2 = j;
                    found = true;
                    break;
                }
}

void Engine::endGame(void)
{
    int i, j;

    assert(game_in_progress);
    assert(!move_in_progress);

    /* Free up the memory used by the map. */

    for (i=0; i<size_x; i++) {
        for (j=0; j<size_y; j++)
            free(map[i][j]);
        free(map[i]);
    }
    free(map);

    /* Free up the memory of all the states used. */

    for (i=0; i<states_allocated; i++) {
        for (j=0; j<size_x; j++)
            free(state_stack[i].board[j]);
        free(state_stack[i].board);
        free(state_stack[i].scoreArray[0]);
        free(state_stack[i].scoreArray[1]);
    }
    states_allocated = 0;

    /* Free up the memory used by the drop_order array. */

    free(drop_order);

    game_in_progress = false;
}

void Engine::reset(void)
{
    assert(!move_in_progress);
    if (game_in_progress)
        endGame();
    poll_function = NULL;
}

int Engine::numOfWinPlaces(int x, int y, int n)
{
    if (x < n && y < n)
        return 0;
    else if (x < n)
        return x * ((y-n)+1);
    else if (y < n)
        return y * ((x-n)+1);
    else
        return 4*x*y - 3*x*n - 3*y*n + 3*x + 3*y - 4*n + 2*n*n + 2;
}

void Engine::updateScore(int player, int x, int y)
{
    register int i;
    int win_index;
    int this_difference = 0, other_difference = 0;
    int **current_scoreArray = current_state->scoreArray;
    int other_player = other(player);

    for (i=0; map[x][y][i] != -1; i++) {
        win_index = map[x][y][i];
        this_difference += current_scoreArray[player][win_index];
        other_difference += current_scoreArray[other_player][win_index];

        current_scoreArray[player][win_index] <<= 1;
        current_scoreArray[other_player][win_index] = 0;

        if (current_scoreArray[player][win_index] == magic_win_number)
            if (current_state->winner == NONE)
                current_state->winner = player;
    }

    current_state->score[player] += this_difference;
    current_state->score[other_player] -= other_difference;
}

int Engine::dropPiece(int player, int column)
{
    int y = 0;

    while (current_state->board[column][y] != NONE && ++y < size_y)
        ;

    if (y == size_y)
        return -1;

    current_state->board[column][y] = player;
    current_state->numOfPieces++;
    updateScore(player, column, y);

    return y;
}

void Engine::pushState(void)
{
    register int i, win_places_array_size;
    Game_state *old_state, *new_state;

    win_places_array_size = win_places * sizeof(int);
    old_state = &state_stack[depth++];
    new_state = &state_stack[depth];

    if (depth == states_allocated) {

        /* Allocate space for the board */

        new_state->board = (char **) emalloc(size_x * sizeof(char *));
        for (i=0; i<size_x; i++)
            new_state->board[i] = (char *) emalloc(size_y);

        /* Allocate space for the score array */

        new_state->scoreArray[0] = (int *) emalloc(win_places_array_size);
        new_state->scoreArray[1] = (int *) emalloc(win_places_array_size);

        states_allocated++;
    }

    /* Copy the board */

    for (i=0; i<size_x; i++)
        memcpy(new_state->board[i], old_state->board[i], size_y);

    /* Copy the score array */

    memcpy(new_state->scoreArray[0], old_state->scoreArray[0],
           win_places_array_size);
    memcpy(new_state->scoreArray[1], old_state->scoreArray[1],
           win_places_array_size);

    new_state->score[0] = old_state->score[0];
    new_state->score[1] = old_state->score[1];
    new_state->winner = old_state->winner;
    new_state->numOfPieces = old_state->numOfPieces;

    current_state = new_state;
}

int Engine::evaluate(int player, int level, int alpha, int beta)
{
    if (poll_function != NULL && next_poll <= clock()) {
        next_poll += poll_interval;
        (*poll_function)();
    }

    if (current_state->winner == player)
        return INT_MAX - depth;
    else if (current_state->winner == other(player))
        return -(INT_MAX - depth);
    else if (current_state->numOfPieces == total_size)
        return 0; /* a tie */
    else if (level == depth)
        return goodness_of(player);
    else {
        /* Assume it is the other player's turn. */
        int best = -(INT_MAX);
        int maxab = alpha;
        for(int i=0; i<size_x; i++) {
            if (current_state->board[drop_order[i]][size_y-1] != NONE)
                continue; /* The column is full. */
            pushState();
            dropPiece(other(player), drop_order[i]);
            int goodness = evaluate(other(player), level, -beta, -maxab);
            if (goodness > best) {
                best = goodness;
                if (best > maxab)
                    maxab = best;
            }
            pop_state();
            if (best > beta)
                break;
        }

        /* What's good for the other player is bad for this one. */
        return -best;
    }
}

void * Engine::emalloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "c4: emalloc() - Can't allocate %ld bytes.\n",
                (long) size);
        exit(1);
    }
    return ptr;
}
