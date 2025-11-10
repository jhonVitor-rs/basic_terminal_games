#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#define SCREEN_WIDTH 25
#define SCREEN_HEIGHT 13

typedef struct
{
  int x;
  int y;
} vec2;

typedef struct
{
  vec2 cursor;
  int board[9][9];
  int solution[9][9];
  bool fixed[9][9];
} GameState;

void init_game_state(GameState *state);
void game_loop(GameState *state, WINDOW *win);
void handle_input(GameState *state, WINDOW *win);
void draw_table(GameState *state);
bool is_valid(GameState *state, int num, int row, int col);
bool is_winner(GameState *state);
bool solve_sudoku(int board[9][9], int row, int col);
vec2 cursor_to_cell(vec2 cursor);

int main(void)
{
  srand(time(NULL));

  WINDOW *win = initscr();
  keypad(win, true);
  nodelay(win, true);
  curs_set(0);
  noecho();

  GameState state;

  while (true)
  {
    init_game_state(&state);
    game_loop(&state, win);

    nodelay(win, false);
    erase();
    mvprintw(SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2 - 10, "PARABÉNS VOCÊ VENCEU");
    mvprintw(SCREEN_HEIGHT / 2 + 2, SCREEN_WIDTH / 2 - 15, "Pressione ENTER para jogar novamente");
    mvprintw(SCREEN_HEIGHT / 2 + 3, SCREEN_WIDTH / 2 - 10, "Pressione ESC para sair");
    refresh();

    while (true)
    {
      int pressed = wgetch(win);
      if (pressed == 27)
      {
        endwin();
        return 0;
      }
      if (pressed == '\n' || pressed == KEY_ENTER)
      {
        nodelay(win, true);
        break;
      }
    }
  }
}

void init_game_state(GameState *state)
{
  memset(state->board, 0, sizeof(state->board));
  memset(state->solution, 0, sizeof(state->solution));
  memset(state->fixed, false, sizeof(state->fixed));

  state->cursor.x = 2;
  state->cursor.y = 1;

  solve_sudoku(state->solution, 0, 0);

  for (int row = 0; row < 9; row++)
  {
    for (int col = 0; col < 9; col++)
    {
      if ((rand() % 100) < 40)
      {
        state->board[row][col] = state->solution[row][col];
        state->fixed[row][col] = true;
      }
      else
      {
        state->board[row][col] = 0;
        state->fixed[row][col] = false;
      }
    }
  }
}

bool solve_sudoku(int board[9][9], int row, int col)
{
  if (row == 9)
    return true;
  if (col == 9)
    return solve_sudoku(board, row + 1, 0);
  if (board[row][col] != 0)
    return solve_sudoku(board, row, col + 1);

  int nums[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  for (int i = 0; i < 9; i++)
  {
    int j = rand() % 9;
    int temp = nums[i];
    nums[i] = nums[j];
    nums[j] = temp;
  }

  for (int i = 0; i < 9; i++)
  {
    int num = nums[i];

    bool valid = true;
    for (int c = 0; c < 9; c++)
    {
      if (board[row][c] == num)
      {
        valid = false;
        break;
      }
    }
    if (!valid)
      continue;

    for (int r = 0; r < 9; r++)
    {
      if (board[r][col] == num)
      {
        valid = false;
        break;
      }
    }
    if (!valid)
      continue;

    int start_row = (row / 3) * 3;
    int start_col = (col / 3) * 3;
    for (int r = 0; r < 3; r++)
    {
      for (int c = 0; c < 3; c++)
      {
        if (board[start_row + r][start_col + c] == num)
        {
          valid = false;
          break;
        }
      }
      if (!valid)
        break;
    }
    if (!valid)
      continue;

    board[row][col] = num;
    if (solve_sudoku(board, row, col + 1))
      return true;
    board[row][col] = 0;
  }
  return false;
}

bool is_valid(GameState *state, int num, int row, int col)
{
  for (int c = 0; c < 9; c++)
  {
    if (c != col && state->board[row][c] == num)
    {
      return false;
    }
  }

  for (int r = 0; r < 9; r++)
  {
    if (r != row && state->board[r][col] == num)
    {
      return false;
    }
  }

  int start_row = (row / 3) * 3;
  int start_col = (col / 3) * 3;
  for (int r = 0; r < 3; r++)
  {
    for (int c = 0; c < 3; c++)
    {
      int curr_row = start_row + r;
      int curr_col = start_col + c;
      if ((curr_row != row || curr_col != col) && state->board[curr_row][curr_col] == num)
      {
        return false;
      }
    }
  }

  return true;
}

vec2 cursor_to_cell(vec2 cursor)
{
  vec2 cell;
  cell.x = (cursor.x - 2) / 2;
  if (cursor.x > 8)
    cell.x--;
  if (cursor.x > 16)
    cell.x--;

  cell.y = cursor.y - 1;
  if (cursor.y > 4)
    cell.y--;
  if (cursor.y > 8)
    cell.y--;

  return cell;
}

void game_loop(GameState *state, WINDOW *win)
{
  while (!is_winner(state))
  {
    handle_input(state, win);

    erase();
    draw_table(state);
    mvprintw(SCREEN_HEIGHT - 1, 0, "Use setas para mover | 1-9 inserir | 0/DEL para apagar | ESC para sair");
    refresh();
    usleep(50000);
  }
}

bool is_winner(GameState *state)
{
  for (int x = 0; x < 9; x++)
  {
    for (int y = 0; y < 9; y++)
    {
      if (state->solution[x][y] != state->board[x][y])
        return false;
    }
  }

  return true;
}

void handle_input(GameState *state, WINDOW *win)
{
  int pressed = wgetch(win);

  if (pressed == KEY_UP)
  {
    state->cursor.y--;
    if (state->cursor.y <= 0)
      state->cursor.y = 11;
    if (state->cursor.y == 4)
      state->cursor.y = 3;
    if (state->cursor.y == 8)
      state->cursor.y = 7;
  }
  else if (pressed == KEY_DOWN)
  {
    state->cursor.y++;
    if (state->cursor.y >= 12)
      state->cursor.y = 1;
    if (state->cursor.y == 4)
      state->cursor.y = 5;
    if (state->cursor.y == 8)
      state->cursor.y = 9;
  }
  else if (pressed == KEY_RIGHT)
  {
    state->cursor.x += 2;
    if (state->cursor.x >= 24)
      state->cursor.x = 2;
    if (state->cursor.x == 8)
      state->cursor.x = 10;
    if (state->cursor.x == 16)
      state->cursor.x = 18;
  }
  else if (pressed == KEY_LEFT)
  {
    state->cursor.x -= 2;
    if (state->cursor.x <= 0)
      state->cursor.x = 22;
    if (state->cursor.x == 8)
      state->cursor.x = 6;
    if (state->cursor.x == 16)
      state->cursor.x = 14;
  }

  vec2 cell = cursor_to_cell(state->cursor);
  if (cell.x >= 0 && cell.x < 9 && cell.y >= 0 && cell.y < 9)
  {
    if (!state->fixed[cell.y][cell.x])
    {
      if (pressed >= '1' && pressed <= '9')
      {
        int num = pressed - '0';
        if (is_valid(state, num, cell.y, cell.x))
        {
          state->board[cell.y][cell.x] = num;
        }
      }
      else if (pressed == '0' || pressed == KEY_BACKSPACE || pressed == KEY_DC)
      {
        state->board[cell.y][cell.x] = 0;
      }
    }
  }

  if (pressed == 27)
  {
    endwin();
    exit(0);
  }
}

void draw_table(GameState *state)
{
  for (int row = 0; row <= 3; row++)
  {
    int y = row * 4;
    for (int x = 0; x <= 24; x++)
    {
      if (x % 8 == 0 || x == 24)
      {
        mvaddch(y, x, '+');
      }
      else
      {
        mvaddch(y, x, '-');
      }
    }
  }

  for (int col = 0; col <= 3; col++)
  {
    int x = col * 8;
    for (int y = 1; y < 12; y++)
    {
      if (y == 4 || y == 8)
        continue;
      mvaddch(y, x, '|');
    }
  }

  for (int row = 0; row < 9; row++)
  {
    for (int col = 0; col < 9; col++)
    {
      int screen_x = col * 2 + 2;
      int screen_y = row + 1;

      if (col >= 3)
        screen_x += 2;
      if (col >= 6)
        screen_x += 2;
      if (row >= 3)
        screen_y += 1;
      if (row >= 6)
        screen_y += 1;

      if (state->cursor.x == screen_x && state->cursor.y == screen_y)
      {
        attron(A_REVERSE);
      }

      if (state->board[row][col] == 0)
      {
        mvaddch(screen_y, screen_x, '.');
      }
      else
      {
        if (state->fixed[row][col])
        {
          attron(A_BOLD);
        }
        mvprintw(screen_y, screen_x, "%d", state->board[row][col]);
        if (state->fixed[row][col])
        {
          attroff(A_BOLD);
        }
      }

      attroff(A_REVERSE);
    }
  }
}
