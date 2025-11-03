#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>
#include <math.h>

#define SCREEN_WIDTH 120
#define SCREEN_HEIGHT 30
#define PADDLE_HEIGHT 4
#define BALL_SPEED 0.8f

typedef struct
{
  float x;
  float y;
} vec2;

typedef struct
{
  vec2 pos;
  vec2 vel;
} Ball;

typedef struct
{
  vec2 pos;
  int height;
} Paddle;

typedef struct
{
  int score_left;
  int score_right;
  int balls_remaining;
  Paddle player_left;
  Paddle player_right;
  Ball ball;
} GameState;

void init_game_state(GameState *state);
void game_loop(GameState *state, WINDOW *win);
void dispatch_ball(GameState *state);
void check_ball_collide(GameState *state);
void draw_box();
void draw_players(GameState *state);
void reset_ball(GameState *state);

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
    mvprintw(SCREEN_HEIGHT / 2 - 1, SCREEN_WIDTH / 2 - 10, "GAME OVER");
    mvprintw(SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2 - 15, "Final Score: %d | %d", state.score_left, state.score_right);
    mvprintw(SCREEN_HEIGHT / 2 + 2, SCREEN_WIDTH / 2 - 17, "Press ENTER to play again");
    mvprintw(SCREEN_HEIGHT / 2 + 3, SCREEN_WIDTH / 2 - 13, "Press ESC to exit");
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
        nodelay(win, true);
        break;
      }
    }
  }
}

void init_game_state(GameState *state)
{
  state->score_left = 0;
  state->score_right = 0;
  state->balls_remaining = 5;

  state->player_left = (Paddle){
      (vec2){2, SCREEN_HEIGHT / 2.0f},
      PADDLE_HEIGHT};
  state->player_right = (Paddle){
      (vec2){SCREEN_WIDTH - 3, SCREEN_HEIGHT / 2.0f},
      PADDLE_HEIGHT};

  state->ball = (Ball){
      (vec2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f},
      (vec2){0, 0}};

  dispatch_ball(state);
}

void game_loop(GameState *state, WINDOW *win)
{
  while (state->balls_remaining > 0)
  {
    int pressed = wgetch(win);

    if (pressed == 'w' || pressed == 'W')
    {
      if (state->player_left.pos.y - state->player_left.height / 2 > 1)
      {
        state->player_left.pos.y -= 1.0f;
      }
    }
    else if (pressed == 's' || pressed == 'S')
    {
      if (state->player_left.pos.y + state->player_left.height / 2 < SCREEN_HEIGHT - 1)
      {
        state->player_left.pos.y += 1.0f;
      }
    }

    if (pressed == KEY_UP)
    {
      if (state->player_right.pos.y - state->player_right.height / 2 > 1)
      {
        state->player_right.pos.y -= 1.0f;
      }
    }
    else if (pressed == KEY_DOWN)
    {
      if (state->player_right.pos.y + state->player_right.height / 2 < SCREEN_HEIGHT - 1)
      {
        state->player_right.pos.y += 1.0f;
      }
    }

    if (pressed == 27)
    {
      break;
    }

    state->ball.pos.x += state->ball.vel.x;
    state->ball.pos.y += state->ball.vel.y;

    check_ball_collide(state);

    if (state->ball.pos.x <= 0)
    {
      state->score_right++;
      state->balls_remaining--;
      if (state->balls_remaining > 0)
      {
        reset_ball(state);
      }
    }
    else if (state->ball.pos.x >= SCREEN_WIDTH)
    {
      state->score_left++;
      state->balls_remaining--;
      if (state->balls_remaining > 0)
      {
        reset_ball(state);
      }
    }

    erase();

    draw_box();

    mvaddch((int)state->ball.pos.y, (int)state->ball.pos.x, 'O');

    draw_players(state);

    mvprintw(0, SCREEN_WIDTH / 2 - 5, "%d | %d", state->score_left, state->score_right);
    mvprintw(SCREEN_HEIGHT - 1, 2, "Balls: %d", state->balls_remaining);

    refresh();
    usleep(50000);
  }
}

void dispatch_ball(GameState *state)
{
  int dir = rand() % 2 ? 1 : -1;
  state->ball.vel.x = dir * 1.0f;
  state->ball.vel.y = (rand() % 3 - 1) * 0.5f;
}

void reset_ball(GameState *state)
{
  state->ball.pos.x = SCREEN_WIDTH / 2.0f;
  state->ball.pos.y = SCREEN_HEIGHT / 2.0f;
  dispatch_ball(state);
  usleep(500000);
}

void check_ball_collide(GameState *state)
{
  Ball *ball = &state->ball;

  if (ball->pos.y <= 0 || ball->pos.y >= SCREEN_HEIGHT - 1)
    ball->vel.y = -ball->vel.y;

  if ((int)ball->pos.x == (int)state->player_left.pos.x + 1)
  {
    float dy = ball->pos.y - state->player_left.pos.y;
    if (fabs(dy) <= state->player_left.height / 2)
    {
      ball->vel.x = fabs(ball->vel.x);
      ball->vel.y = dy * 0.3f;
    }
  }

  if ((int)ball->pos.x == (int)state->player_right.pos.x - 1)
  {
    float dy = ball->pos.y - state->player_right.pos.y;
    if (fabs(dy) <= state->player_right.height / 2)
    {
      ball->vel.x = -fabs(ball->vel.x);
      ball->vel.y = dy * 0.3f;
    }
  }
}

void draw_box()
{
  for (int i = 0; i < SCREEN_WIDTH; i++)
  {
    mvaddch(0, i, '#');
    mvaddch(SCREEN_HEIGHT, i, '#');
  }

  for (int i = 1; i < SCREEN_HEIGHT - 1; i++)
  {
    if (i % 2 == 0)
    {
      mvaddch(i, SCREEN_WIDTH / 2, '|');
    }
  }
}

void draw_players(GameState *state)
{
  for (int i = -state->player_left.height / 2; i <= state->player_left.height / 2; i++)
  {
    int y = (int)state->player_left.pos.y + i;
    if (y > 0 && y < SCREEN_HEIGHT - 1)
    {
      mvaddch(y, (int)state->player_left.pos.x, '|');
    }
  }

  for (int i = -state->player_right.height / 2; i <= state->player_right.height / 2; i++)
  {
    int y = (int)state->player_right.pos.y + i;
    if (y > 0 && y < SCREEN_HEIGHT - 1)
    {
      mvaddch(y, (int)state->player_right.pos.x, '|');
    }
  }
}
