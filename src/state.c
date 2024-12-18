#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row,
                         unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);

/* Task 1 */
game_state_t *create_default_state() {
  const unsigned int ROWS = 18;
  const unsigned int COLS = 20;
  const unsigned int NSNAKES = 1;

  game_state_t *state = (game_state_t *)malloc(sizeof(game_state_t));

  state->num_rows = ROWS;
  state->num_snakes = NSNAKES;
  state->board = (char **)malloc(ROWS * sizeof(char *));
  state->snakes = (snake_t *)malloc(NSNAKES * sizeof(snake_t));

  for (int row = 0; row < ROWS; row++) {
    state->board[row] = (char *)malloc((COLS + 1) * sizeof(char));
    char *row_cpy;
    if (row == 0 || row == ROWS - 1) {
      row_cpy = "####################";
    } else if (row == 2) {
      row_cpy = "# d>D    *         #";
    } else {
      row_cpy = "#                  #";
    }
    strcpy(state->board[row], row_cpy);
  }

  state->snakes[0].live = true;
  state->snakes[0].head_row = 2;
  state->snakes[0].head_col = 4;
  state->snakes[0].tail_row = 2;
  state->snakes[0].tail_col = 2;

  return state;
}

/* Task 2 */
void free_state(game_state_t *state) {
  for (int i = 0; i < state->num_rows; i++) {
    free(state->board[i]);
  }
  free(state->board);
  free(state->snakes);
  free(state);
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp) {
  for (int row = 0; row < state->num_rows; row++) {
    fprintf(fp, "%s\n", state->board[row]);
  }
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t *state, unsigned int row,
                         unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  return c == 'w' || c == 'a' || c == 's' || c == 'd';
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  return c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x';
}

static bool is_body(char c) {
  return c == '^' || c == '<' || c == '>' || c == 'v';
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) { return is_head(c) || is_body(c) || is_tail(c); }

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  switch (c) {
  case '^':
    return 'w';
  case '<':
    return 'a';
  case 'v':
    return 's';
  case '>':
    return 'd';
  default:
    return ' ';
  }
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  switch (c) {
  case 'W':
    return '^';
  case 'A':
    return '<';
  case 'S':
    return 'v';
  case 'D':
    return '>';
  default:
    return ' ';
  }
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  if (c == 'v' || c == 's' || c == 'S') {
    return cur_row + 1;
  } else if (c == '^' || c == 'w' || c == 'W') {
    return cur_row - 1;
  } else {
    return cur_row;
  }
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  if (c == '>' || c == 'd' || c == 'D') {
    return cur_col + 1;
  } else if (c == '<' || c == 'a' || c == 'A') {
    return cur_col - 1;
  } else {
    return cur_col;
  }
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake
  is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t *state, unsigned int snum) {
  snake_t snake = state->snakes[snum];
  char curr_ch = state->board[snake.head_row][snake.head_col];
  unsigned int next_row = get_next_row(snake.head_row, curr_ch);
  unsigned int next_col = get_next_col(snake.head_col, curr_ch);
  return state->board[next_row][next_col];
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the
  head.
*/
static void update_head(game_state_t *state, unsigned int snum) {
  snake_t *snake = &state->snakes[snum];
  char curr_head_ch = state->board[snake->head_row][snake->head_col];
  unsigned int next_row = get_next_row(snake->head_row, curr_head_ch);
  unsigned int next_col = get_next_col(snake->head_col, curr_head_ch);
  char body = head_to_body(curr_head_ch);
  set_board_at(state, snake->head_row, snake->head_col, body);
  set_board_at(state, next_row, next_col, curr_head_ch);
  snake->head_row = next_row;
  snake->head_col = next_col;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t *state, unsigned int snum) {
  snake_t *snake = &state->snakes[snum];
  char curr_tail_ch = state->board[snake->tail_row][snake->tail_col];

  unsigned int next_row = get_next_row(snake->tail_row, curr_tail_ch);
  unsigned int next_col = get_next_col(snake->tail_col, curr_tail_ch);
  char curr_body_ch = get_board_at(state, next_row, next_col);
  char next_tail_ch = body_to_tail(curr_body_ch);

  set_board_at(state, snake->tail_row, snake->tail_col, ' ');
  set_board_at(state, next_row, next_col, next_tail_ch);
  snake->tail_row = next_row;
  snake->tail_col = next_col;
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
  for (unsigned int s = 0; s < state->num_snakes; s++) {
    char next_ch = next_square(state, s);
    snake_t *snake = &state->snakes[s];
    if (next_ch == '*') { // FOOD
      update_head(state, s);
      add_food(state);
    } else if (next_ch == '#' || is_snake(next_ch)) { // HIT SOMETHING
      set_board_at(state, snake->head_row, snake->head_col, 'x');
      snake->live = false;
    } else { // JUST MOVE
      update_head(state, s);
      update_tail(state, s);
    }
  }
}

/* Task 5.1 */
char *read_line(FILE *fp) {
  int BUF_SIZE = 16;
  unsigned long size = 1;
  char *line = (char *)malloc(sizeof(char));

  char buf[BUF_SIZE];
  char *eol;

  do {
    fgets(buf, BUF_SIZE, fp);
    if (ferror(fp) || feof(fp)) {
      return NULL;
    }
    eol = strchr(buf, '\n');
    unsigned long new_char_size = eol == NULL ? (unsigned long)BUF_SIZE - 1
                                              : (long unsigned)(eol - buf + 1);
    if (new_char_size > 0) {
      line = realloc(line, (size + new_char_size) * sizeof(char));
    }
    strncpy(line + size - 1, buf, new_char_size);
    size = size + new_char_size;
    line[size - 1] = '\0';
  } while (eol == NULL);

  return line;
}

/* Task 5.2 */
game_state_t *load_board(FILE *fp) {
  game_state_t *state = (game_state_t *)malloc(sizeof(game_state_t));
  state->num_rows = 0;
  state->num_snakes = 0;
  state->snakes = (snake_t *)malloc(0);
  char *line;
  while ((line = read_line(fp)) != NULL) {
    char *cpy = (char *)malloc(strlen(line) * sizeof(char));
    strncpy(cpy, line, strlen(line));
    cpy[strlen(line) - 1] = '\0';
    free(line);
    state->num_rows++;
    state->board = realloc(state->board, state->num_rows * sizeof(char *));
    state->board[state->num_rows - 1] = cpy;
  }
  return state;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t *state, unsigned int snum) {
  snake_t *snake = &state->snakes[snum];
  char cur_ch;
  unsigned int row = snake->tail_row;
  unsigned int col = snake->tail_col;
  while (!is_head(cur_ch = get_board_at(state, row, col))) {
    row = get_next_row(row, cur_ch);
    col = get_next_col(col, cur_ch);
  }
  snake->head_row = row;
  snake->head_col = col;
}

/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state) {
  char **board = state->board;
  for (unsigned int row = 0; row < state->num_rows; row++) {
    for (unsigned int col = 0; col < strlen(board[row]); col++) {
      char cur_ch = get_board_at(state, row, col);
      if (is_tail(cur_ch)) {
        unsigned int new_num = state->num_snakes + 1;
        state->num_snakes = new_num;
        state->snakes = realloc(state->snakes, new_num * sizeof(snake_t));
        state->snakes[new_num - 1].tail_row = row;
        state->snakes[new_num - 1].tail_col = col;
        find_head(state, new_num - 1);
      }
    }
  }
  return state;
}
