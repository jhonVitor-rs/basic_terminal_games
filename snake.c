#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>

#define MAX_SEGMENTS 256
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 30
#define INITIAL_INTERVAL 150000
#define SPEED_INCREMENT 5000
#define MIN_INTERVAL 30000

typedef struct
{
  int x;
  int y;
} vec2;

typedef struct
{
  vec2 segments[MAX_SEGMENTS];
  int score;
  vec2 head;
  vec2 dir;
  vec2 berry;
  int interval;
  char head_char;
} GameState;

void game_loop(GameState *state, WINDOW *win);
void fetch_segments(GameState *state);
void draw_box(void);
bool is_game_over(const GameState *state);
void spawn_berry(GameState *state);
bool is_position_occupied(const GameState *state, vec2 pos);
void init_game_state(GameState *state);

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
    mvprintw(SCREEN_HEIGHT / 2 - 1, SCREEN_WIDTH / 2 - 15, "GAME OVER - Score: %d", state.score);
    mvprintw(SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2 - 17, "Press ENTER to play again");
    mvprintw(SCREEN_HEIGHT / 2 + 1, SCREEN_WIDTH / 2 - 13, "Press ESC to exit");
    refresh();

    int pressed;
    while (true)
    {
      pressed = wgetch(win);
      if (pressed == 27)
      {
        endwin();
        return 0;
      }
      if (pressed == '\n' || pressed == KEY_ENTER)
      {
        erase();
        nodelay(win, true);
        break;
      }
    }
  }
}

void init_game_state(GameState *state)
{
  state->score = 0;
  for (int i = 0; i < MAX_SEGMENTS; i++)
  {
    state->segments[i] = (vec2){0, 0};
  }
  state->head = (vec2){SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2};
  state->dir = (vec2){1, 0};
  state->interval = INITIAL_INTERVAL;
  state->head_char = '>';
  spawn_berry(state);
}

void spawn_berry(GameState *state)
{
  vec2 new_berry;
  int attempts = 0;
  const int max_attempts = 100;

  do
  {
    new_berry.x = (rand() % (SCREEN_WIDTH / 2 - 2)) + 1;
    new_berry.y = (rand() % (SCREEN_HEIGHT - 2)) + 1;
    attempts++;
  } while (is_position_occupied(state, new_berry) && attempts < max_attempts);

  state->berry = new_berry;
}

bool is_position_occupied(const GameState *state, vec2 pos)
{
  if (pos.x == state->head.x && pos.y == state->head.y)
  {
    return true;
  }

  for (int i = 0; i < state->score; i++)
  {
    if (pos.x == state->segments[i].x && pos.y == state->segments[i].y)
    {
      return true;
    }
  }

  return false;
}

void game_loop(GameState *state, WINDOW *win)
{
  while (true)
  {
    int pressed = wgetch(win);

    if (pressed == KEY_LEFT && state->dir.x != 1)
    {
      state->dir = (vec2){-1, 0};
      state->head_char = '<';
    }
    else if (pressed == KEY_RIGHT && state->dir.x != -1)
    {
      state->dir = (vec2){1, 0};
      state->head_char = '>';
    }
    else if (pressed == KEY_UP && state->dir.y != 1)
    {
      state->dir = (vec2){0, -1};
      state->head_char = '^';
    }
    else if (pressed == KEY_DOWN && state->dir.y != -1)
    {
      state->dir = (vec2){0, 1};
      state->head_char = 'v';
    }
    else if (pressed == 27)
    { // ESC
      break;
    }

    fetch_segments(state);

    state->head.x += state->dir.x;
    state->head.y += state->dir.y;

    if (is_game_over(state))
    {
      break;
    }

    if (state->head.x == state->berry.x && state->head.y == state->berry.y)
    {
      state->score++;

      if (state->interval > MIN_INTERVAL)
      {
        state->interval -= SPEED_INCREMENT;
        if (state->interval < MIN_INTERVAL)
        {
          state->interval = MIN_INTERVAL;
        }
      }

      spawn_berry(state);
    }

    erase();
    draw_box();
    mvaddch(state->berry.y, state->berry.x * 2, '@' | A_BOLD | COLOR_PAIR(1));

    for (int i = 0; i < state->score; i++)
    {
      mvaddch(state->segments[i].y, state->segments[i].x * 2, 'o');
    }

    mvaddch(state->head.y, state->head.x * 2, state->head_char | A_BOLD);
    mvprintw(0, SCREEN_WIDTH + 2, "Score: %d", state->score);
    mvprintw(1, SCREEN_WIDTH + 2, "Speed: %d", (INITIAL_INTERVAL - state->interval) / SPEED_INCREMENT);

    refresh();
    usleep(state->interval);
  }
}

void fetch_segments(GameState *state)
{
  for (int i = state->score; i > 0; i--)
  {
    state->segments[i] = state->segments[i - 1];
  }
  state->segments[0] = state->head;
}

void draw_box(void)
{
  for (int i = 0; i < SCREEN_WIDTH; i++)
  {
    mvaddch(0, i, '#');
    mvaddch(SCREEN_HEIGHT, i, '#');
  }

  for (int i = 0; i <= SCREEN_HEIGHT; i++)
  {
    mvaddch(i, 0, '#');
    mvaddch(i, SCREEN_WIDTH - 1, '#');
  }
}

bool is_game_over(const GameState *state)
{
  if (state->head.x <= 0 || state->head.x >= (SCREEN_WIDTH / 2) - 1 || state->head.y <= 0 || state->head.y >= SCREEN_HEIGHT)
  {
    return true;
  }

  for (int i = 0; i < state->score; i++)
  {
    if (state->head.x == state->segments[i].x && state->head.y == state->segments[i].y)
    {
      return true;
    }
  }

  return false;
}
