/* --- TODO ---
  - Make grid size have no limit (A-Z on X axis)
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
} minesweeper_struct;

// =====================
//  COLORS
// =====================

char_colormap colorMap[] = {
    {'0', 16},
    {'1', 21},
    {'2', 27},
    {'3', 33},
    {'4', 39},
    {'5', 45},
    {'6', 51},
    {'7', 87},
    {'8', 123},
    {starting_char, 48},
    {flag_char, 196},
    {mine_char, 124}};

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
  scanf("%31s", moveCoord);

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

// _is_safe_zone(): Check if the coordinates (r, c) are within a 3x3 zone around (safe_row, safe_col)
// @param r: Row to check
// @param c: Column to check
// @param safe_row: Center row of the safe zone
// @param safe_col: Center column of the safe zone
// @return: 1 if inside the safe zone, 0 otherwise
int _is_safe_zone(int r, int c, int safe_row, int safe_col)
{
  return (r >= safe_row - 1 && r <= safe_row + 1 &&
          c >= safe_col - 1 && c <= safe_col + 1);
}

// _getColor(): Returns the value linked to the key in the colorMap
// @param character: The character to get the color of
// @return colorCode: 15 if it cannot find the 'colorCode'
int _getColor(char character)
{
  int size = sizeof(colorMap) / sizeof(colorMap[0]);
  for (int i = 0; i < size; i++)
  {
    if (colorMap[i].character == character)
      return colorMap[i].colorCode;
  }
  return 15;
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
      char dataAttribute = matrix[r][c].data;
      int color = _getColor(dataAttribute);
      printf("\x1b[38;5;%dm%c\x1b[0m ", color, dataAttribute);
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
      if (matrix[r][c].data == mine_char)
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
            if (matrix[nr][nc].data == mine_char)
              count++;
          }
        }
      }

      matrix[r][c].data = '0' + count;
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
  int total = rows * cols;

  int *indices = malloc(total * sizeof(int));

  for (int i = 0; i < total; i++)
    indices[i] = i;

  for (int i = total - 1; i > 0; i--)
  {
    int j = rand() % (i + 1);
    int tmp = indices[i];
    indices[i] = indices[j];
    indices[j] = tmp;
  }

  int placed = 0;

  for (int k = 0; k < total && placed < mineCount; k++)
  {
    int r = indices[k] / cols;
    int c = indices[k] % cols;

    if (_is_safe_zone(r, c, safe_row, safe_col))
      continue;

    matrix[r][c].data = mine_char;
    placed++;
  }

  free(indices);
}

// _renderMove(): Reveal a cell and recursively reveal adjacent empty cells
// @param game: Pointer to the game state
// @param row: Row of the move
// @param col: Column of the move
void _renderMove(minesweeper_struct *game, int row, int col)
{
  if (row < 0 || row >= game->rows || col < 0 || col >= game->cols)
    return;

  if (game->visible_matrix[row][col].data != starting_char)
    return;

  if (game->hidden_matrix[row][col].data == mine_char)
  {
    game->game_over = 1;
    return;
  }

  if (!game->mines_initialized)
  {
    _renderMines(game->hidden_matrix, game->rows, game->cols, game->mines_amt, row, col);

    _renderNumbers(game->hidden_matrix, game->rows, game->cols);

    game->mines_initialized = 1;
  }

  game->visible_matrix[row][col].data = game->hidden_matrix[row][col].data;

  if (game->hidden_matrix[row][col].data == '0')
  {
    for (int dr = -1; dr <= 1; dr++)
    {
      for (int dc = -1; dc <= 1; dc++)
      {
        int nr = row + dr;
        int nc = col + dc;
        if (nr >= 0 && nr < game->rows && nc >= 0 && nc < game->cols && !(dr == 0 && dc == 0))
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
  if (row < 0 || row >= game->rows || col < 0 || col >= game->cols)
    return;

  char current = game->visible_matrix[row][col].data;

  if (current == starting_char)
  {
    game->visible_matrix[row][col].data = flag_char;
  }
  else if (current == flag_char)
  {
    game->visible_matrix[row][col].data = starting_char;
  }
}

// check_win(): Check if the player has won the game
// @param game: Pointer to the game state
// @return: 1 if all non-mine cells are revealed, 0 otherwise
int check_win(minesweeper_struct *game)
{
  for (int r = 0; r < game->rows; r++)
  {
    for (int c = 0; c < game->cols; c++)
    {
      if (game->visible_matrix[r][c].data == starting_char &&
          game->hidden_matrix[r][c].data != mine_char)
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
// @param startingChar: Initial character for each cell's data
// @return: Pointer to the allocated matrix
Vector2D **init_Matrix2D(int rows, int cols, char startingChar)
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
      matrix[r][c].data = startingChar;
    }
  }

  return matrix;
}

// init_hiddenMatrix(): Initialize the hidden matrix for minesweeper
// @param rows: Number of rows
// @param cols: Number of columns
// @param startingChar: Initial character for each cell's data
// @return: Pointer to the allocated hidden matrix
Vector2D **init_hiddenMatrix(int rows, int cols, char startingChar)
{
  return init_Matrix2D(rows, cols, startingChar);
}

// reveal_board(): Reveal all cells on the visible matrix by copying from the hidden matrix
// @param game: Pointer to the game state
void reveal_board(minesweeper_struct *game)
{
  for (int r = 0; r < game->rows; r++)
  {
    for (int c = 0; c < game->cols; c++)
    {
      game->visible_matrix[r][c].data = game->hidden_matrix[r][c].data;
    }
  }
}

// set_MatrixData(): Set the data of a specific cell in the matrix
// @param matrix: The matrix
// @param row: Row index of the cell
// @param col: Column index of the cell
// @param value: Character to set as the cell's data
void set_MatrixData(Vector2D **matrix, int row, int col, char value)
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
minesweeper_struct *minesweeper_init(int rows, int cols, char startingChar)
{
  assert(rows < 27);
  assert(cols < 27);
  assert(mine_amount < rows * cols);

  minesweeper_struct *game = malloc(sizeof(minesweeper_struct));

  game->rows = rows;

  game->cols = cols;

  game->mines_amt = mine_amount;

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

    _printMatrixData(game->visible_matrix, game->rows, game->cols);

    char *move_str = _inputListener();
    size_t len = strlen(move_str);

    if (strcmp(move_str, "--QUIT") == 0)
    {
        printf("\nEXITING GAME.\n");
        break;
    }

    if (strcmp(move_str, "--HELP") == 0)
    {
        printf("\n--- COMMANDS ---\n\n- Playing moves: <Character><Integer>. Characters are on the X-Axis and the Integers are on the Y-Axis. (e.x. A1, B2, etc.)\n\n- Flagging: <Character><Integer>%c. Flags a certain cell; Doing it on a cell with a flag will un-flag it. (e.x. C4%c, G10%c, etc.)\n\n- Quitting the game: --quit.\n", flag_char, flag_char, flag_char);
        _waitForEnter();
        continue;
    }

    input_coordinate coords;

    int is_flag = 0;
    if (len > 1 && move_str[len-1] == flag_char)
    {
        move_str[len-1] = '\0';
        is_flag = 1;
    }

    _parseMove(move_str, &coords.row, &coords.col);

    if (is_flag) move_str[len-1] = flag_char;

    if (coords.row == -1 || coords.col == -1 ||
        coords.row >= game->rows || coords.col >= game->cols)
    {
        printf("\n--- Invalid command. Type '--help' for all available commands ---\n");
        _waitForEnter();
        continue;
    }

    if (is_flag)
    {
        _toggleFlag(game, coords.row, coords.col);
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

      _printMatrixData(game->visible_matrix, game->rows, game->cols);

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

    _printMatrixData(game->visible_matrix, game->rows, game->cols);

    printf("\n--- BOMB HIT. GAME OVER. ---\n");
  }
}

// minesweeper_destroy(): Free all memory associated with a minesweeper game
// @param game: Pointer to the game state
void minesweeper_destroy(minesweeper_struct *game)
{
  assert(game != NULL);

  _freeMatrix(game->hidden_matrix, game->rows);
  _freeMatrix(game->visible_matrix, game->rows);
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