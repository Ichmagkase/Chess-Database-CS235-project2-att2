#define _XOPEN_SOURCE 700
#include <ftw.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define DATA_DIR_NAME "data"

struct GameState {
  int game_number;
  int ply_number;
  char piece;
  char *origin_square;
  char *dest_square;
  char captured_piece;
  char promotion;
  int is_castle;
  int is_check;
  int is_checkmate;
  char *fen;
};

FILE *verify(FILE *f) {
  if (f == NULL) {
    perror("fopen\n");
    exit(EXIT_FAILURE);
  }
  return f;
}

void check_error(int status, char *description) {
  if (status != 0) {
    perror(description);
    exit(EXIT_FAILURE);
  }
}

void reset_game(struct GameState *game_state) {}

void update_state(struct GameState *game_state, char *token) {
  // TODO: Parse the token in here. Populate or update game_state struct. Apply
  // SANs to FENs
}

// NOTE: Claude wrote this code ...
static int rm_entry(const char *path, const struct stat *sb, int typeflag,
                    struct FTW *ftwbuf) {
  return (typeflag == FTW_DP) ? rmdir(path) : unlink(path);
}

void reset_data_dir(void) {
  // Remove if it exists (ignore error if it doesn't)
  nftw("data", rm_entry, 16, FTW_DEPTH | FTW_PHYS);

  // Recreate it — 0755 = rwxr-xr-x
  if (mkdir("data", 0755) != 0) {
    perror("mkdir");
    exit(EXIT_FAILURE);
  }
}
// NOTE: End AI code

int main(void) {
  int status;

  // Create and populate data directory
  printf("Creating data files...");
  reset_data_dir();

  FILE *move_data = verify(fopen("data/moves_data.csv", "w"));
  FILE *game_data = verify(fopen("data/games_data.csv", "w"));
  FILE *metadata_tags_data = verify(fopen("data/metadata_tags_data", "w"));
  FILE *chess_data = verify(fopen("../twic210-874.pgn", "r"));

  // Compile reges
  printf("Compiling regular expressions...");
  regex_t tag_line;
  const char *tag_line_exp = "^\\[.*\\].*";
  if (regcomp(&tag_line, tag_line_exp, REG_NEWLINE | REG_EXTENDED))
    exit(EXIT_FAILURE);

  regex_t move_number;
  const char *move_number_exp = "^[0-9]+\\.$";
  if (regcomp(&move_number, move_number_exp, REG_NEWLINE | REG_EXTENDED))
    exit(EXIT_FAILURE);

  regex_t result;
  const char *result_exp = "^(1-0|0-1|1/2-1/2)$";
  if (regcomp(&result, result_exp, REG_EXTENDED | REG_NEWLINE))
    exit(EXIT_FAILURE);

  regex_t san;
  const char *san_exp =
      "^([NBRQK][a-h1-8]?[a-h1-8]?x?[a-h][1-8]"
      "|[a-h][1-8](=[NBRQ])?"       // simple pawn push: e4, d5
      "|[a-h]x[a-h][1-8](=[NBRQ])?" // pawn capture: exd5, axb1=Q
      "|O-O-O|O-O)[+#]?$";
  if (regcomp(&san, san_exp, REG_NEWLINE | REG_EXTENDED))
    exit(EXIT_FAILURE);

  char *line = NULL;
  size_t size;
  int line_len;
  int delim_len;
  int game_number = 1;
  int ply_number = 1;
  char *token;
  char key[256];
  char value[256];
  struct GameState game;

  while ((line_len = getline(&line, &size, chess_data)) != -1) {
    if (regexec(&tag_line, line, 0, NULL, 0) == 0) {
      sscanf(line, "[%255s \"%255[^\"]\"]", key, value);
      fprintf(metadata_tags_data, "%d,%s,%s\n", game_number, key, value);
      // TODO: if tag_line is a FEN, add it to the struct
      // GameState game -> fen

    } else {
      char *copy = strdup(line);
      token = strtok(copy, " \n\t\r");

      while (token != NULL) {
        // NOTE: Everything under this while is parsing moves out of a game
        if (regexec(&san, token, 0, NULL, 0) == 0) {

          update_state(&game, token);

          fprintf(move_data, "%d,%d,%c,%s,%s,%c,%c,%d,%d,%d,%s\n",
                  game.game_number, game.ply_number, game.piece,
                  game.origin_square, game.dest_square, game.captured_piece,
                  game.promotion, game.is_castle, game.is_check,
                  game.is_checkmate, game.fen);
          ply_number++;
        } else if (regexec(&result, token, 0, NULL, 0) == 0) {
          fprintf(game_data, "%d,%s,%d\n", game_number, token, ply_number);
          game_number++;
          ply_number = 0;
          // Zero struct GameState
          memset(&game, 0, sizeof(struct GameState));
        } else {
          printf("Skipping/Ignoring:\n%s\n\n", token);
        }
        token = strtok(NULL, " \n\t\r");
      }
      free(copy);
    }
  }

  // Clean  up
  free(line);

  regfree(&tag_line);
  regfree(&move_number);
  regfree(&san);

  status = fclose(move_data);
  check_error(status, "fclose");

  status = fclose(game_data);
  check_error(status, "fclose");

  status = fclose(metadata_tags_data);
  check_error(status, "fclose");

  status = fclose(chess_data);
  check_error(status, "fclose");
}
