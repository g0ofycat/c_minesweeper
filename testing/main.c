#include "../src/minesweeper.h"

int main(int argc, char* argv[]) {
    int rows = argc > 1 ? atoi(argv[1]) : row_amount;
    int cols = argc > 2 ? atoi(argv[2]) : col_amount;
    int mines = argc > 3 ? atoi(argv[3]) : mine_amount;

    if (rows <= 0 || cols <= 0 || mines <= 0) {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }

    minesweeper_struct *game = minesweeper_init(time(NULL), rows, cols, mines);

    minesweeper_game_loop(game);
    minesweeper_destroy(game);

    return 0;
}
