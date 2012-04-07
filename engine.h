#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <time.h>
#include <stdbool.h>

#define NONE      2
#define MAX_LEVEL 20

#define other(x)        ((x) ^ 1)
#define real_player(x)  ((x) & 1)

#define pop_state() \
        (current_state = &state_stack[--depth])

#define goodness_of(player) \
        (current_state->score[player] - current_state->score[other(player)])

enum {HUMAN = 0, COMPUTER = 1};

class Engine : public QObject
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent = 0);

    typedef struct {
        char **board;
        int *(scoreArray[2]);
        int score[2];
        short int winner;
        int numOfPieces;
    } Game_state;
    
signals:
    
public slots:
    void newGame(int width, int height, int num);
    void endGame(void);
    void winCoords(int *x1, int *y1, int *x2, int *y2);
    int numOfWinPlaces(int x, int y, int n);
    int dropPiece(int player, int column);
    bool makeMove(int player, int column, int *row);
    bool makeMove(int player, int column);
    int getLastAIcolumn();
    int getLastAIrow();

private:
    void poll(void (*poll_func)(void), clock_t interval);
    bool autoMove(int player, int level, int *column, int *row);
    char** board(void);
    int scoreOfPlayer(int player);
    bool isWinner(int player);
    bool isTie(void);
    void reset(void);
    void updateScore(int player, int x, int y);
    void pushState(void);
    int evaluate(int player, int level, int alpha, int beta);
    void * emalloc(size_t size);

    int size_x, size_y, total_size;
    int num_to_connect;
    int win_places;

    int ***map;  /* map[x][y] is an array of win place indices, */
                        /* terminated by a -1.                         */

    int magic_win_number;
    bool game_in_progress, move_in_progress;
    bool seed_chosen;
    void (*poll_function)(void);
    clock_t poll_interval, next_poll;
    Game_state state_stack[MAX_LEVEL+1];
    Game_state *current_state;
    int depth;
    int states_allocated;
    int *drop_order;
    
    int lastAIcolumn;
    int lastAIrow;
};

#endif // ENGINE_H
