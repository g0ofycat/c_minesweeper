#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>

// =====================
//  DEFINES
// =====================

#define row_amount 16
#define col_amount 16
#define mine_amount 32

#define mine_char '*'
#define starting_char 'X'
#define flag_char '?'

// =====================
//  STRUCTS
// =====================

typedef struct
{
    char character;
    int colorCode;
} char_colormap;

typedef struct
{
    int row;
    int col;
} input_coordinate;

typedef struct
{
    int x;
    int y;
    char data;
} Vector2D;

typedef struct
{
    Vector2D **hidden_matrix;
    Vector2D **visible_matrix;
    int rows;
    int cols;
    int mines_amt;
    int game_over;
    int mines_initialized;
    int current_seed;
} minesweeper_struct;

// =====================
//  COLORS
// =====================

extern char_colormap colorMap[];

// =====================
//  HELPERS
// =====================

// _freeMatrix(): Free all allocated data from the given matrix
// @param matrix: The matrix
// @param rows: How many rows the matrix has
void _freeMatrix(Vector2D **matrix, int rows);

// _clearScreen(): Helper function to clear screen
void _clearScreen(void);

// _waitForEnter(): Pause execution until the user presses Enter
void _waitForEnter(void);

// _inputListener(): Prompt the user for input and return it in uppercase
// @return: Pointer to a static buffer containing the user input
char *_inputListener(void);

// =====================
//  MISC
// =====================

// _parseMove(): Convert a move string (like "A1") into row and column indices
// @param move: Input string representing the move
// @param row: Pointer to store the parsed row index
// @param col: Pointer to store the parsed column index
void _parseMove(const char *move, int *row, int *col);

// _isSafeZone(): Check if the coordinates (r, c) are within a 3x3 zone around (safe_row, safe_col)
// @param r: Row to check
// @param c: Column to check
// @param safe_row: Center row of the safe zone
// @param safe_col: Center column of the safe zone
// @return: 1 if inside the safe zone, 0 otherwise
int _isSafeZone(int r, int c, int safe_row, int safe_col);

// _getColor(): Returns the value linked to the key in the colorMap
// @param character: The character to get the color of
// @return: colorCode: 15 if it cannot find the 'colorCode'
int _getColor(char character);

// =====================
//  PRINTING
// =====================

// _printMatrixData(): Print the data of a matrix of Vector2D structs
// @param matrix: The matrix to print
// @param rows: Number of rows in the matrix
// @param cols: Number of columns in the matrix
void _printMatrixData(Vector2D **matrix, int rows, int cols);

// _printMatrixVectors(): Print the X and Y coordinates of a matrix of Vector2D structs
// @param matrix: The matrix to print
// @param rows: Number of rows in the matrix
// @param cols: Number of columns in the matrix
void _printMatrixVectors(Vector2D **matrix, int rows, int cols);

// =====================
//  BOARD API
// =====================

// init_matrix_2D(): Allocate and initialize a 2D matrix of Vector2D structs
// @param rows: Number of rows
// @param cols: Number of columns
// @param startingChar: Initial character for each cell's data
// @return: Pointer to the allocated matrix
Vector2D **init_matrix_2D(int rows, int cols, char startingChar);

// init_hidden_matrix(): Wrapper for init_matrix_2D, used to create the hidden minesweeper board
// @param rows: Number of rows
// @param cols: Number of columns
// @param startingChar: Initial character for each cell's data
// @return: Pointer to the allocated hidden matrix
Vector2D **init_hidden_matrix(int rows, int cols, char startingChar);

// set_matrix_data(): Set the data of a specific cell in the matrix
// @param matrix: The matrix
// @param row: Row index of the cell
// @param col: Column index of the cell
// @param value: Character to set as the cell's data
void set_matrix_data(Vector2D **matrix, int row, int col, char value);

// =====================
//  GAME API
// =====================

// _renderNumbers(): Calculate and set the number of adjacent mines for each cell
// @param matrix: The matrix representing the board
// @param rows: Number of rows
// @param cols: Number of columns
void _renderNumbers(Vector2D **matrix, int rows, int cols);

// _renderMines(): Randomly place mines in the matrix while avoiding the safe zone
// @param matrix: The matrix representing the board
// @param rows: Number of rows
// @param cols: Number of columns
// @param mineCount: Total number of mines to place
// @param safe_row: Row of the initial safe move
// @param safe_col: Column of the initial safe move
void _renderMines(Vector2D **matrix, int rows, int cols, int mineCount, int safe_row, int safe_col);

// _renderMove(): Reveal a cell and recursively reveal adjacent empty cells
// @param game: Pointer to the game state
// @param row: Row of the move
// @param col: Column of the move
void _renderMove(minesweeper_struct *game, int row, int col);

// _revealBoard(): Reveal all cells on the visible matrix by copying from the hidden matrix
// @param game: Pointer to the game state
void _revealBoard(minesweeper_struct *game);

// _toggleFlag(): Place or remove a flag on a cell
// @param game: Pointer to the game state
// @param row: Row of the cell
// @param col: Column of the cell
void _toggleFlag(minesweeper_struct *game, int row, int col);

// _checkWin(): Check if the player has won the game
// @param game: Pointer to the game state
// @return: 1 if all non-mine cells are revealed, 0 otherwise
int _checkWin(minesweeper_struct *game);

// _parseAndValidateMove(): Parses user input and validates coordinates
// @param move_str: String containing the user's move input
// @param game: Pointer to the minesweeper game struct
// @param coords: Pointer to coordinate struct to store parsed coordinates
// @param is_flag: Pointer to int that will be set to 1 if move is a flag command
// @return: 1 if coordinates are valid, 0 if invalid
int _parseAndValidateMove(char *move_str, minesweeper_struct *game,
        input_coordinate *coords, int *is_flag);

// _displayGame(): Clears screen and displays the current game board
// @param game: Pointer to the minesweeper game struct
void _displayGame(minesweeper_struct *game);

// _showHelp(): Displays the help menu with available commands
void _showHelp(void);

// _showSeed(): Display the current seed
// @param game: Pointer to the minesweeper game struct
void _showSeed(minesweeper_struct *game);

// _showGameEnd(): Reveals the board and displays win/loss message
// @param game: Pointer to the minesweeper game struct
// @param won: 1 if player won, 0 if player lost
void _showGameEnd(minesweeper_struct *game, int won);

// =====================
//  BOT API
// =====================

// get_Radius(): Gets all of the directons given a coordinate
// @param coords: Pointer to coordinate struct to store parsed coordinates
// @param game: Pointer to the minesweeper game struct
Vector2D *get_Radius(input_coordinate *coords, minesweeper_struct *game, int *out_len);

// =====================
//  MAIN API
// =====================

// minesweeper_init(): Initialize a new minesweeper game
// @param seed: The random seed to set
// @param rows: Number of rows
// @param cols: Number of columns
// @param mines_amt: The amount of mines to place
// @return: Pointer to the initialized game struct
minesweeper_struct *minesweeper_init(int seed, int rows, int cols, int mines_amt);

// minesweeper_game_loop(): Main game loop handling input, moves, and win/lose conditions
// @param game: Pointer to the game state
void minesweeper_game_loop(minesweeper_struct *game);

// minesweeper_destroy(): Free all memory associated with a minesweeper game
// @param game: Pointer to the game state
void minesweeper_destroy(minesweeper_struct *game);

#endif
