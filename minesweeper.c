#include <curses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define FIELD_WIDTH 20
#define FIELD_HEIGHT 15
#define BOM_PERCENTAGE 15

typedef struct
{
  int x;
  int y;
} vec2;

typedef struct
{
  bool has_bomb;
  bool has_marked;
  bool has_revealed;
  int adjacent_bombs;
} Cell;

typedef struct
{
  Cell field[FIELD_WIDTH][FIELD_HEIGHT];
  vec2 cursor;
  bool game_over;
  bool won;
  int bombs_total;
  int cells_revealed;
  int flags_placed;
} GameState;

void init_game_state(GameState *state);
void game_loop(GameState *state, WINDOW *win);
void handle_input(GameState *state, WINDOW *win);
void draw_field(GameState *state);
void reveal_cell(GameState *state, int x, int y);
void calculate_adjacent_bombs(GameState *state);
int count_adjacent_bombs(GameState *state, int x, int y);
bool check_win(GameState *state);

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

    if (state.won)
    {
      mvprintw(FIELD_HEIGHT / 2, FIELD_WIDTH - 5, "🎉 PARABÉNS! VOCÊ VENCEU! 🎉");
    }
    else
    {
      mvprintw(FIELD_HEIGHT / 2, FIELD_WIDTH - 3, "💣 GAME OVER! 💣");
    }

    mvprintw(FIELD_HEIGHT / 2 + 2, FIELD_WIDTH - 10, "Pressione ENTER para jogar novamente");
    mvprintw(FIELD_HEIGHT / 2 + 3, FIELD_WIDTH - 5, "Pressione ESC para sair");
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
  state->bombs_total = 0;
  state->cells_revealed = 0;
  state->flags_placed = 0;
  state->game_over = false;
  state->won = false;
  state->cursor.x = 0;
  state->cursor.y = 0;

  for (int x = 0; x < FIELD_WIDTH; x++)
  {
    for (int y = 0; y < FIELD_HEIGHT; y++)
    {
      state->field[x][y].has_marked = false;
      state->field[x][y].has_marked = false;
      state->field[x][y].has_revealed = false;
      state->field[x][y].adjacent_bombs = 0;

      if ((rand() % 100) < BOM_PERCENTAGE)
      {
        state->field[x][y].has_bomb = true;
        state->bombs_total++;
      }
      else
      {
        state->field[x][y].has_bomb = false;
      }
    }
  }

  calculate_adjacent_bombs(state);
}

void calculate_adjacent_bombs(GameState *state)
{
  for (int x = 0; x < FIELD_WIDTH; x++)
  {
    for (int y = 0; y < FIELD_HEIGHT; y++)
    {
      if (!state->field[x][y].has_bomb)
      {
        state->field[x][y].adjacent_bombs = count_adjacent_bombs(state, x, y);
      }
    }
  }
}

int count_adjacent_bombs(GameState *state, int x, int y)
{
  int count = 0;

  for (int dx = -1; dx <= 1; dx++)
  {
    for (int dy = -1; dy <= 1; dy++)
    {
      if (dx == 0 && dy == 0)
        continue;

      int nx = x + dx;
      int ny = y + dy;

      if (nx >= 0 && nx < FIELD_WIDTH && ny >= 0 && ny < FIELD_HEIGHT)
      {
        if (state->field[nx][ny].has_bomb)
        {
          count++;
        }
      }
    }
  }

  return count;
}

void reveal_cell(GameState *state, int x, int y)
{
  if (x < 0 || x >= FIELD_WIDTH || y < 0 || y >= FIELD_HEIGHT)
  {
    return;
  }

  Cell *cell = &state->field[x][y];

  if (cell->has_revealed || cell->has_marked)
  {
    return;
  }

  cell->has_revealed = true;
  state->cells_revealed++;

  if (cell->has_bomb)
  {
    state->game_over = true;
    state->won = false;
    return;
  }

  if (cell->adjacent_bombs == 0)
  {
    for (int dx = -1; dx <= 1; dx++)
    {
      for (int dy = -1; dy <= 1; dy++)
      {
        if (dx == 0 && dy == 0)
          continue;
        reveal_cell(state, x + dx, y + dy);
      }
    }
  }
}

bool check_win(GameState *state)
{
  int safe_cells = (FIELD_WIDTH * FIELD_HEIGHT) - state->bombs_total;
  return state->cells_revealed >= safe_cells;
}

void game_loop(GameState *state, WINDOW *win)
{
  while (!state->game_over && !state->won)
  {
    handle_input(state, win);

    if (check_win(state))
    {
      state->won = true;
      state->game_over = true;
    }

    erase();
    draw_field(state);

    mvprintw(FIELD_HEIGHT + 2, 0, "Bombas: %d | Bandeiras: %d | Reveladas: %d/%d",
             state->bombs_total, state->flags_placed, state->cells_revealed,
             (FIELD_WIDTH * FIELD_HEIGHT) - state->bombs_total);
    mvprintw(FIELD_HEIGHT + 3, 0, "ENTER: Revelar | ESPAÇO: Marcar | ESC: Sair");

    refresh();
    usleep(50000);
  }
}

void handle_input(GameState *state, WINDOW *win)
{
  int pressed = wgetch(win);

  if (pressed == KEY_UP)
  {
    state->cursor.y--;
    if (state->cursor.y < 0)
    {
      state->cursor.y = FIELD_HEIGHT - 1;
    }
  }
  else if (pressed == KEY_DOWN)
  {
    state->cursor.y++;
    if (state->cursor.y >= FIELD_HEIGHT)
    {
      state->cursor.y = 0;
    }
  }
  else if (pressed == KEY_RIGHT)
  {
    state->cursor.x++;
    if (state->cursor.x >= FIELD_WIDTH)
    {
      state->cursor.x = 0;
    }
  }
  else if (pressed == KEY_LEFT)
  {
    state->cursor.x--;
    if (state->cursor.x < 0)
    {
      state->cursor.x = FIELD_WIDTH - 1;
    }
  }
  else if (pressed == '\n' || pressed == KEY_ENTER)
  {
    reveal_cell(state, state->cursor.x, state->cursor.y);
  }
  else if (pressed == ' ')
  {
    Cell *cell = &state->field[state->cursor.x][state->cursor.y];
    if (!cell->has_revealed)
    {
      if (cell->has_marked)
      {
        cell->has_marked = false;
        state->flags_placed--;
      }
      else
      {
        cell->has_marked = true;
        state->flags_placed++;
      }
    }
  }
  else if (pressed == 27)
  {
    endwin();
    exit(0);
  }
}

void draw_field(GameState *state)
{
  mvaddch(0, 0, '+');
  for (int x = 0; x < FIELD_WIDTH; x++)
  {
    mvaddch(0, x + 1, '-');
  }
  mvaddch(0, FIELD_WIDTH, '+');

  for (int y = 0; y < FIELD_HEIGHT; y++)
  {
    mvaddch(y + 1, 0, '|');
    for (int x = 0; x < FIELD_WIDTH; x++)
    {
      Cell *cell = &state->field[x][y];
      int screen_x = x * 2 + 1;
      int screen_y = y + 1;

      if (state->cursor.x == x && state->cursor.y == y)
      {
        attron(A_REVERSE);
      }

      if (cell->has_revealed)
      {
        if (cell->has_bomb)
        {
          attron(A_BOLD);
          mvaddch(screen_y, screen_x, '*');
          attroff(A_BOLD);
        }
        else if (cell->adjacent_bombs > 0)
        {
          mvprintw(screen_y, screen_x, "%d", cell->adjacent_bombs);
        }
        else
        {
          mvaddch(screen_y, screen_x, ' ');
        }
      }
      else if (cell->has_marked)
      {
        attron(A_BOLD);
        mvaddch(screen_y, screen_x, 'F');
        attroff(A_BOLD);
      }
      else
      {
        mvaddch(screen_y, screen_x, '#');
      }

      attroff(A_REVERSE);
      mvaddch(screen_y, screen_x + 1, ' ');
    }

    mvaddch(y + 1, FIELD_WIDTH * 2 + 1, '|');
  }

  mvaddch(FIELD_HEIGHT + 1, 0, '*');
  for (int x = 0; x < FIELD_WIDTH * 2; x++)
  {
    mvaddch(FIELD_HEIGHT + 1, x + 1, '-');
  }
  mvaddch(FIELD_HEIGHT + 1, FIELD_WIDTH * 2 + 1, '+');

  if (state->game_over && !state->won)
  {
    for (int x = 0; x < FIELD_WIDTH; x++)
    {
      for (int y = 0; y < FIELD_HEIGHT; y++)
      {
        if (state->field[x][y].has_bomb)
        {
          attron(A_BOLD);
          mvaddch(y + 1, x * 2 + 1, '*');
          attroff(A_BOLD);
        }
      }
    }
  }
}
