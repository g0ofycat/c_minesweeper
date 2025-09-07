/* --- TODO ---
  - Make grid size have no limit (A-Z on X axis), Add color, Fix Alphabetical 'flag_char'
*/

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

#define mine_char "*"
#define starting_char "X"
#define flag_char "?"

// =====================
//  STRUCTS
// =====================

typedef struct
{
  int row;
  int col;
} input_coordinate;

typedef struct
{
  double x;
  double y;
  char *data;
} Vector2D;

typedef struct
{
  Vector2D **hidden_matrix;
  Vector2D **visible_matrix;
  int game_over;
  int mines_initialized;
} minesweeper_struct;

// =====================
//  HELPERS
// =====================

// _freeMatrix(): Free all allocated data from the given matrix
// @param matrix: The matrix
// @param rows: How many rows the matrix has
void _freeMatrix(Vector2D **matrix, int rows)
{
  assert(matrix != NULL);

  for (int i = 0; i < rows; i++)
  {
    free(matrix[i]);
  }

  free(matrix);
}

// _waitForEnter(): Pause execution until the user presses Enter
void _waitForEnter()
{
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
  printf("\nPress Enter to continue...");
  getchar();
}

// _inputListener(): Prompt the user for input and return it in uppercase
// @return: Pointer to a static buffer containing the user input
char *_inputListener()
{
  static char moveCoord[32];

  printf("\nType '--help' for commands: ");
  scanf("%9s", moveCoord);

  for (int i = 0; moveCoord[i]; i++)
  {
    moveCoord[i] = toupper(moveCoord[i]);
  }

  return moveCoord;
}

// _parseMove(): Convert a move string (like "A1") into row and column indices
// @param move: Input string representing the move
// @param row: Pointer to store the parsed row index
// @param col: Pointer to store the parsed column index
void _parseMove(const char *move, int *row, int *col)
{
  if (move[0] >= 'A' && move[0] <= 'Z')
  {
    *col = move[0] - 'A';
  }
  else
  {
    *col = -1;
  }

  int number = atoi(move + 1);

  if (number > 0)
  {
    *row = number - 1;
  }
  else
  {
    *row = -1;
  }
}

// is_safe_zone(): Check if the coordinates (r, c) are within a 3x3 zone around (safe_row, safe_col)
// @param r: Row to check
// @param c: Column to check
// @param safe_row: Center row of the safe zone
// @param safe_col: Center column of the safe zone
// @return: 1 if inside the safe zone, 0 otherwise
int is_safe_zone(int r, int c, int safe_row, int safe_col)
{
  return (r >= safe_row - 1 && r <= safe_row + 1 &&
          c >= safe_col - 1 && c <= safe_col + 1);
}

// _returnInputVector(): Prompt the user for a move and return it as an input_coordinate struct
// @return: input_coordinate containing parsed row and column indices
input_coordinate _returnInputVector()
{
  input_coordinate coordinate;

  char *move = _inputListener();

  _parseMove(move, &coordinate.row, &coordinate.col);

  return coordinate;
}

// =====================
//  PRINTING
// =====================

// _printMatrixData(): Print the data of a matrix of Vector2D structs
// @param matrix: The matrix to print
// @param rows: Number of rows in the matrix
// @param cols: Number of columns in the matrix
void _printMatrixData(Vector2D **matrix, int rows, int cols)
{
  for (int r = 0; r < rows; r++)
  {
    printf("%2d ", r + 1);

    for (int c = 0; c < cols; c++)
    {
      char *dataAttribute = matrix[r][c].data;
      if (dataAttribute != NULL)
      {
        printf("%s ", dataAttribute);
      }
      else
      {
        printf("? ");
      }
    }
    printf("\n");
  }

  printf("   ");

  for (int c = 0; c < cols; c++)
  {
    printf("%c ", 'A' + c);
  }

  printf("\n");
}

// _printMatrixVectors(): Print the x and y coordinates of a matrix of Vector2D structs
// @param matrix: The matrix to print
// @param rows: Number of rows in the matrix
// @param cols: Number of columns in the matrix
void _printMatrixVectors(Vector2D **matrix, int rows, int cols)
{
  for (int r = 0; r < rows; r++)
  {
    for (int c = 0; c < cols; c++)
    {
      printf("(%.2f, %.2f) ", matrix[r][c].x, matrix[r][c].y);
    }
    printf("\n");
  }
}

// =====================
//  GAME API
// =====================

// _renderNumbers(): Calculate and set the number of adjacent mines for each cell
// @param matrix: The matrix representing the board
// @param rows: Number of rows
// @param cols: Number of columns
void _renderNumbers(Vector2D **matrix, int rows, int cols)
{
  for (int r = 0; r < rows; r++)
  {
    for (int c = 0; c < cols; c++)
    {
      if (strcmp(matrix[r][c].data, mine_char) == 0)
        continue;

      int count = 0;
      for (int dr = -1; dr <= 1; dr++)
      {
        for (int dc = -1; dc <= 1; dc++)
        {
          int nr = r + dr;
          int nc = c + dc;
          if (nr >= 0 && nr < rows && nc >= 0 && nc < cols)
          {
            if (strcmp(matrix[nr][nc].data, mine_char) == 0)
              count++;
          }
        }
      }

      char buffer[2];
      buffer[0] = count + '0';
      buffer[1] = '\0';

      if (matrix[r][c].data != starting_char && matrix[r][c].data != mine_char)
        free(matrix[r][c].data);

      matrix[r][c].data = strdup(buffer);
    }
  }
}

// _renderMines(): Randomly place mines in the matrix while avoiding the safe zone
// @param matrix: The matrix representing the board
// @param rows: Number of rows
// @param cols: Number of columns
// @param mineCount: Total number of mines to place
// @param safe_row: Row of the initial safe move
// @param safe_col: Column of the initial safe move
void _renderMines(Vector2D **matrix, int rows, int cols, int mineCount, int safe_row, int safe_col)
{
  int placed = 0;
  while (placed < mineCount)
  {
    int r = rand() % rows;
    int c = rand() % cols;

    if (is_safe_zone(r, c, safe_row, safe_col) || strcmp(matrix[r][c].data, mine_char) == 0)
      continue;

    free(matrix[r][c].data);
    matrix[r][c].data = strdup(mine_char);
    placed++;
  }
}

// _renderMove(): Reveal a cell and recursively reveal adjacent empty cells
// @param game: Pointer to the game state
// @param row: Row of the move
// @param col: Column of the move
void _renderMove(minesweeper_struct *game, int row, int col)
{
  if (row < 0 || row >= row_amount || col < 0 || col >= col_amount)
    return;

  if (strcmp(game->visible_matrix[row][col].data, starting_char) != 0)
    return;

  if (strcmp(game->hidden_matrix[row][col].data, mine_char) == 0)
  {
    game->game_over = 1;

    return;
  }

  if (!game->mines_initialized)
  {
    _renderMines(game->hidden_matrix, row_amount, col_amount, mine_amount, row, col);

    _renderNumbers(game->hidden_matrix, row_amount, col_amount);

    game->mines_initialized = 1;
  }

  game->visible_matrix[row][col].data = game->hidden_matrix[row][col].data;

  if (strcmp(game->hidden_matrix[row][col].data, "0") == 0)
  {
    for (int dr = -1; dr <= 1; dr++)
    {
      for (int dc = -1; dc <= 1; dc++)
      {
        int nr = row + dr;
        int nc = col + dc;
        if (nr >= 0 && nr < row_amount && nc >= 0 && nc < col_amount && !(dr == 0 && dc == 0))
          _renderMove(game, nr, nc);
      }
    }
  }
}

// _toggleFlag(): Place or remove a flag on a cell
// @param game: Pointer to the game state
// @param row: Row of the cell
// @param col: Column of the cell
void _toggleFlag(minesweeper_struct *game, int row, int col)
{
  if (row < 0 || row >= row_amount || col < 0 || col >= col_amount)
    return;

  if (strcmp(game->visible_matrix[row][col].data, starting_char) == 0)
  {
    free(game->visible_matrix[row][col].data);
    game->visible_matrix[row][col].data = strdup(flag_char);
  }
  else if (strcmp(game->visible_matrix[row][col].data, flag_char) == 0)
  {
    free(game->visible_matrix[row][col].data);
    game->visible_matrix[row][col].data = strdup(starting_char);
  }
}

// check_win(): Check if the player has won the game
// @param game: Pointer to the game state
// @return: 1 if all non-mine cells are revealed, 0 otherwise
int check_win(minesweeper_struct *game)
{
  for (int r = 0; r < row_amount; r++)
  {
    for (int c = 0; c < col_amount; c++)
    {
      if (strcmp(game->visible_matrix[r][c].data, starting_char) == 0 &&
          strcmp(game->hidden_matrix[r][c].data, mine_char) != 0)
      {
        return 0;
      }
    }
  }
  return 1;
}

// =====================
//  BOARD API
// =====================

// init_Matrix2D(): Allocate and initialize a 2D matrix of Vector2D structs
// @param rows: Number of rows
// @param cols: Number of columns
// @param startingChar: Initial string for each cell's data
// @return: Pointer to the allocated matrix
Vector2D **init_Matrix2D(int rows, int cols, char *startingChar)
{
  Vector2D **matrix = (Vector2D **)malloc(rows * sizeof(Vector2D *));

  for (int i = 0; i < rows; i++)
  {
    matrix[i] = (Vector2D *)malloc(cols * sizeof(Vector2D));
  }

  for (int r = 0; r < rows; r++)
  {
    for (int c = 0; c < cols; c++)
    {
      matrix[r][c].x = c;
      matrix[r][c].y = r;
      matrix[r][c].data = strdup(startingChar);
    }
  }

  return matrix;
}

// init_hiddenMatrix(): Initialize the hidden matrix for minesweeper
// @param rows: Number of rows
// @param cols: Number of columns
// @param startingChar: Initial string for each cell's data
// @return: Pointer to the allocated hidden matrix
Vector2D **init_hiddenMatrix(int rows, int cols, char *startingChar)
{
  return init_Matrix2D(rows, cols, startingChar);
}

// reveal_board(): Reveal all cells on the visible matrix by copying from the hidden matrix
// @param game: Pointer to the game state
void reveal_board(minesweeper_struct *game)
{
  for (int r = 0; r < row_amount; r++)
  {
    for (int c = 0; c < col_amount; c++)
    {
      game->visible_matrix[r][c].data = game->hidden_matrix[r][c].data;
    }
  }
}

// set_MatrixData(): Set the data of a specific cell in the matrix
// @param matrix: The matrix
// @param row: Row index of the cell
// @param col: Column index of the cell
// @param value: String to set as the cell's data
void set_MatrixData(Vector2D **matrix, int row, int col, char *value)
{
  matrix[row][col].data = value;
}

// =====================
//  MAIN API
// =====================

// minesweeper_init(): Initialize a new minesweeper game
// @param rows: Number of rows
// @param cols: Number of columns
// @param startingChar: Initial character for each cell
// @return: Pointer to the initialized game struct
minesweeper_struct *minesweeper_init(int rows, int cols, char *startingChar)
{
  assert(row_amount < 27);
  assert(col_amount < 27);

  minesweeper_struct *game = malloc(sizeof(minesweeper_struct));

  game->visible_matrix = init_Matrix2D(rows, cols, startingChar);

  game->hidden_matrix = init_hiddenMatrix(rows, cols, startingChar);

  game->game_over = 0;

  game->mines_initialized = 0;

  return game;
}

// minesweeper_game_loop(): Main game loop handling input, moves, and win/lose conditions
// @param game: Pointer to the game state
void minesweeper_game_loop(minesweeper_struct *game)
{
  while (!game->game_over)
  {

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    _printMatrixData(game->visible_matrix, row_amount, col_amount);

    char *move_str = _inputListener();

    if (strncmp(move_str, flag_char, 1) == 0)
    {
      input_coordinate coords;
      _parseMove(move_str + 1, &coords.row, &coords.col);
      _toggleFlag(game, coords.row, coords.col);
      continue;
    }

    if (strcmp(move_str, "--QUIT") == 0)
    {
      printf("\nEXITING GAME.\n");
      break;
    }

    if (strcmp(move_str, "--HELP") == 0)
    {
      printf("\n--- COMMANDS ---\n\n- Playing moves: <Character><Integer>. Characters are on the X-Axis and the Integers are on the Y-Axis. (e.x. A1, B2, etc.)\n\n- Flagging: ?<Character><Integer>. Flags a certain cell; Doing it on a cell with a flag will un-flag it. (e.x. ?C4, ?G10, etc.)\n\n- Quitting the game: --quit.\n");
      _waitForEnter();
      continue;
    }

    input_coordinate coords;

    _parseMove(move_str, &coords.row, &coords.col);

    if (coords.row == -1 || coords.col == -1 ||
        coords.row >= row_amount || coords.col >= col_amount)
    {
      printf("\n--- Invalid command. Type '--help' for all available commands ---\n");
      _waitForEnter();
      continue;
    }

    _renderMove(game, coords.row, coords.col);

    if (check_win(game))
    {
      reveal_board(game);

#ifdef _WIN32
      system("cls");
#else
      system("clear");
#endif

      _printMatrixData(game->visible_matrix, row_amount, col_amount);

      printf("\n--- YOU WIN! ---\n");

      break;
    }
  }

  if (game->game_over)
  {
    reveal_board(game);

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    _printMatrixData(game->visible_matrix, row_amount, col_amount);

    printf("\n--- BOMB HIT. GAME OVER. ---\n");
  }
}

// minesweeper_destroy(): Free all memory associated with a minesweeper game
// @param game: Pointer to the game state
void minesweeper_destroy(minesweeper_struct *game)
{
  assert(game != NULL);

  for (int r = 0; r < row_amount; r++)
  {
    for (int c = 0; c < col_amount; c++)
    {

      char *hiddenData = game->hidden_matrix[r][c].data;

      if (hiddenData != starting_char && hiddenData != mine_char)
      {
        free(hiddenData);
      }

      char *visibleData = game->visible_matrix[r][c].data;

      if (visibleData != hiddenData)
      {
        free(visibleData);
      }
    }
  }

  _freeMatrix(game->hidden_matrix, row_amount);
  _freeMatrix(game->visible_matrix, row_amount);
  free(game);
}

// =====================
//  MAIN
// =====================

int main()
{
  srand(time(NULL));

  minesweeper_struct *game = minesweeper_init(row_amount, col_amount, starting_char);

  minesweeper_game_loop(game);

  minesweeper_destroy(game);

  return 0;
}