#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include "unistd.h"
#include "stdio.h"
#include "time.h"
#include "stdbool.h"
#include "assert.h"

struct board_t {
    int width;
    int height;
    int players;
    int current_player;
    int symbols;
    char** matrix;
    int* scores;
};

typedef struct board_t board_t;

enum direction_t {
    UP = 0,
    LEFT = 1,
    RIGHT = 2,
    DOWN = 3
};

typedef enum direction_t direction_t;

struct score_t {
    char nickname[20];
    int score;
};

typedef struct score_t score_t;


const int MIN_SYMBOLS = 4;
const int MAX_SYMBOLS = 6;

const char* SYMBOLS = "UXIOSN";

const char SYMBOLS_COLOR[6][8] = {
        "\033[0;31m",
        "\033[0;32m",
        "\033[0;33m",
        "\033[0;34m",
        "\033[0;35m",
        "\033[0;36m"
};

const char RESET_COLOR[8] = "\033[0m";

const char EMPTY_SYMBOL = ' ';

const char* SCORES_FILE = "./scores.save";

char generate_random_symbol(int symbols) {
    assert(symbols >= MIN_SYMBOLS && symbols <= MAX_SYMBOLS);

    return SYMBOLS[rand() % symbols];
}

// Fonction pour échanger deux nombres
void swap(char *x, char *y) {
    char t = *x; *x = *y; *y = t;
}
 
// Fonction pour inverser `buffer[i…j]`
char* reverse(char *buffer, int i, int j)
{
    while (i < j) {
        swap(&buffer[i++], &buffer[j--]);
    }
 
    return buffer;
}
 
// Fonction itérative pour implémenter la fonction `itoa()` en C
char* itoa(int value, char* buffer, int base)
{
    // entrée invalide
    if (base < 2 || base > 32) {
        return buffer;
    }
 
    // considère la valeur absolue du nombre
    int n = abs(value);
 
    int i = 0;
    while (n)
    {
        int r = n % base;
 
        if (r >= 10) {
            buffer[i++] = 65 + (r - 10);
        }
        else {
            buffer[i++] = 48 + r;
        }
 
        n = n / base;
    }
 
    // si le nombre est 0
    if (i == 0) {
        buffer[i++] = '0';
    }
 
    // Si la base est 10 et la valeur est négative, la string résultante
    // est précédé d'un signe moins (-)
    // Avec toute autre base, la valeur est toujours considérée comme non signée
    if (value < 0 && base == 10) {
        buffer[i++] = '-';
    }
 
    buffer[i] = '\0'; // string de fin nulle
 
    // inverse la string et la renvoie
    return reverse(buffer, 0, i - 1);
}
void replace_empty_symbols(board_t* board) {
    for (int j = 0; j < board->width; ++j) {
        for (int i = board->height - 1; i >= 0; --i) {
            if (board->matrix[i][j] == EMPTY_SYMBOL) {
                board->matrix[i][j] = generate_random_symbol(board->symbols);
            }
        }
    }
}

/**
 * Vérifie si la case en position (i,j) est alignée avec deux autres cases (à gauche ou à droite)
 */
bool is_horizontally_aligned(board_t* board, int i, int j) {
    assert(i >= 0 && i < board->height);
    assert(j >= 0 && j < board->width);

    for (int k = 0; k < 3; ++k) {
        if (j - 2 + k < 0 || j + k >= board->width) continue;

        if (board->matrix[i][j - 2 + k] == board->matrix[i][j - 1 + k] && board->matrix[i][j - 1 + k] == board->matrix[i][j + k]) {
            return true;
        }
    }

    return false;
}

/**
 * Vérifie si la case en position (i,j) est alignée avec deux autres cases (en haut ou en bas)
 */
bool is_vertically_aligned(board_t* board, int i, int j) {
    assert(i >= 0 && i < board->height);
    assert(j >= 0 && j < board->width);

    for (int k = 0; k < 3; ++k) {
        if (i - 2 + k < 0 || i + k >= board->height) continue;

        if (board->matrix[i - 2 + k][j] == board->matrix[i - 1 + k][j] && board->matrix[i - 1 + k][j] == board->matrix[i + k][j]) {
            return true;
        }
    }

    return false;
}

/**
 * Vérifie si la case en position (i,j) est alignée avec deux autres cases (en haut ou en bas)
 */
bool is_diagonally_aligned(board_t* board, int i, int j) {
    assert(i >= 0 && i < board->height);
    assert(j >= 0 && j < board->width);

    for (int k = 0; k < 3; ++k) {
        if (j - 2 + k < 0 || j - 2 + k >= board->width) continue;
        if (i - 2 + k < 0 || i - 2 + k >= board->height) continue;

        if (j - 1 + k < 0 || j - 1 + k >= board->width) continue;
        if (i - 1 + k < 0 || i - 1 + k >= board->height) continue;

        if (j + k < 0 || j + k >= board->width) continue;
        if (i + k < 0 || i + k >= board->height) continue;

        if (board->matrix[i - 2 + k][j - 2 + k] == board->matrix[i - 1 + k][j - 1 + k] && board->matrix[i - 1 + k][j - 1 + k] == board->matrix[i + k][j + k]) {
            return true;
        }
    }

    for (int k = 0; k < 3; ++k) {
        if (j - 2 + k < 0 || j - 2 + k >= board->width) continue;
        if (i + 2 - k < 0 || i + 2 - k >= board->height) continue;

        if (j - 1 + k < 0 || j - 1 + k >= board->width) continue;
        if (i + 1 - k < 0 || i + 1 - k >= board->height) continue;

        if (j + k < 0 || j + k >= board->width) continue;
        if (i - k < 0 || i - k >= board->height) continue;


        if (board->matrix[i + 2 - k][j - 2 + k] == board->matrix[i + 1 - k][j - 1 + k] && board->matrix[i + 1 - k][j - 1 + k] == board->matrix[i - k][j + k]) {
            return true;
        }
    }

    return false;
}

void clear() {
    system("cls");
    system("clear");
}

bool is_aligned(board_t* board, int i, int j) {
    assert(i >= 0 && i < board->height);
    assert(j >= 0 && j < board->width);

    return is_diagonally_aligned(board, i, j)
           || is_horizontally_aligned(board, i, j)
           || is_vertically_aligned(board, i, j);
}

bool has_aligned_symbols(board_t* board) {
    for (int i = 0; i < board->height; ++i) {
        for (int j = 0; j < board->width; ++j) {
            if (is_aligned(board, i, j)) {
                return true;
            }
        }
    }

    return false;
}

void free_board_matrix(char** matrix, int height) {
    for (int i = 0; i < height; ++i) {
        free(matrix[i]);
    }

    free(matrix);
}

char** copy_board_matrix(board_t* board) {
    char** matrix = malloc(board->height * sizeof(char*));

    for (int i = 0; i < board->height; ++i) {
        matrix[i] = malloc(board->width * sizeof(char));

        for (int j = 0; j < board->width; ++j) {
            matrix[i][j] = board->matrix[i][j];
        }
    }

    return matrix;
}

int apply_gravity(board_t *board) {
    int count = 0;
    for (int j = 0; j < board->width; ++j) {
        for (int i = board->height - 1; i >= 0; --i) {
            if (board->matrix[i][j] == EMPTY_SYMBOL) {
                for (int k = i - 1; k >= 0; --k) {
                    if (board->matrix[k][j] != EMPTY_SYMBOL) {
                        board->matrix[i][j] = board->matrix[k][j];
                        board->matrix[k][j] = EMPTY_SYMBOL;
                        ++count;
                        break;
                    }
                }
            }
        }
    }

    return count;
}

int remove_aligned_symbols(board_t* board) {
    char** copy = copy_board_matrix(board);
    int count = 0;

    for (int i = 0; i < board->height; ++i) {
        for (int j = 0; j < board->width; ++j) {
            if (is_aligned(board, i, j)) {
                copy[i][j] = EMPTY_SYMBOL;
                count++;
            }
        }
    }

    free_board_matrix(board->matrix, board->height);
    board->matrix = copy;

    return count;
}


board_t *init_board(int width, int height, int players, int symbols) {
    board_t *board = (board_t *) malloc(sizeof(board_t));

    assert(symbols >= MIN_SYMBOLS && symbols <= MAX_SYMBOLS);

    board->width = width;
    board->height = height;
    board->players = players;
    board->symbols = symbols;
    board->current_player = 0;

    board->scores = (int *) malloc(players * sizeof(int));

    for (int i = 0; i < players; ++i) {
        board->scores[i] = 0;
    }

    board->matrix = (char **) malloc(height * sizeof (char *));

    printf("Initialisation du plateau de jeu...\n");

    for (int i = 0; i < height; ++i) {
        board->matrix[i] = (char *) malloc(width * sizeof(char));

        for (int j = 0; j < width; ++j) {
            board->matrix[i][j] = generate_random_symbol(symbols);
        }
    }

    while (has_aligned_symbols(board)) {
        remove_aligned_symbols(board);
        apply_gravity(board);
        replace_empty_symbols(board);
    }

    printf("Plateau de jeu initialisé !\n");

    return board;
}

/**
 * Compte le nombre de chiffres d'un nombre
 */
int count_digits(int number) {
    if (number == 0) return 1;

    int digits = 0;

    while (number != 0) {
        number /= 10;
        digits++;
    }

    return digits;
}

char* get_symbol_color(char symbol) {
    for (int i = 0; i < MAX_SYMBOLS; ++i) {
        if (SYMBOLS[i] == symbol) {
            return (char *)SYMBOLS_COLOR[i];
        }
    }

    return "";
}

/**
 * Affiche le plateau de jeu
 */
void print_board(board_t* board) {
    int digits = count_digits(board->width - 1);

    clear();

    for (int i = 0; i < board->players; ++i) {
        printf("Joueur %d : %d points ", i + 1, board->scores[i]);

        if (i < board->players - 1) {
            printf("| ");
        }
    }

    printf("\n\n");

    for (int i = 0; i < board->width + 1; ++i) {
        if (i == 0) {
            printf("|");

            for (int j = 0; j < digits + 2; ++j) {
                printf("_");
            }

            printf("|");
        }

        if (i == board->width) {
            printf("|");
            continue;
        }

        for (int k = 0; k < digits; ++k) {
            printf("_");
        }

        printf("_");

        for (int k = 0; k < digits; ++k) {
            printf("_");
        }
    }
    printf("\n");

    for (int i = 0; i < board->width + 1; ++i) {
        if (i == 0) {
            printf("|");

            for (int j = 0; j < digits + 2; ++j) {
                printf(" ");
            }

            printf("|");
        }

        if (i == board->width) {
            printf("|");
            continue;
        }

        for (int k = 0; k < digits; ++k) {
            printf(" ");
        }

        printf("%d", i);

        for (int k = 0; k < digits - count_digits(i) + 1; ++k) {
            printf(" ");
        }
    }
    printf("\n");

    for (int i = 0; i < board->width + 1; ++i) {
        if (i == 0) {
            printf("|");

            for (int j = 0; j < digits + 2; ++j) {
                printf("|");
            }

            printf("|");
        }

        if (i == board->width) {
            printf("|");
            continue;
        }

        for (int k = 0; k < digits; ++k) {
            printf("_");
        }

        printf("_");

        for (int k = 0; k < digits; ++k) {
            printf("_");
        }
    }

    printf("\n");

    for (int i = 0; i < board->height; ++i) {
        int lineDigits = count_digits(i);

        for (int j = 0; j < board->width; ++j) {
            if (j == 0) {
                printf("| ");

                printf("%d", i);

                /**
                 * Ajustement des espaces pour que les chiffres soient alignés
                 */
                for (int k = 0; k < digits - lineDigits + 1; ++k) {
                    printf(" ");
                }

                printf("|");
            }


            for (int k = 0; k < digits; ++k) {
                printf(" ");
            }

            printf("%s%c%s", get_symbol_color(board->matrix[i][j]), board->matrix[i][j], RESET_COLOR);

            for (int k = 0; k < digits; ++k) {
                printf(" ");
            }

            if(j == board->width - 1) {
                printf("|");
                continue;
            }

        }
        printf("\n");
    }

    for (int i = 0; i < board->width + 1; ++i) {
        if (i == 0) {
            printf("|");

            for (int j = 0; j < digits + 2; ++j) {
                printf("_");
            }

            printf("|");
        }

        if (i == board->width) {
            printf("|");
            continue;
        }

        for (int k = 0; k < digits; ++k) {
            printf("_");
        }

        printf("_");

        for (int k = 0; k < digits; ++k) {
            printf("_");
        }
    }

    printf("\n");
}


void free_board(board_t* board) {
    for (int i = 0; i < board->height; ++i) {
        free(board->matrix[i]);
    }

    free(board->matrix);
    free(board->scores);
    free(board);
}

void remove_board_symbol(board_t* board, int i, int j) {
    assert(i >= 0 && i < board->height);
    assert(j >= 0 && j < board->width);

    board->matrix[i][j] = EMPTY_SYMBOL;
}

int* copy_board_scores(board_t* board) {
    int* scores = malloc(board->players * sizeof(int));

    for (int i = 0; i < board->players; ++i) {
        scores[i] = board->scores[i];
    }

    return scores;
}

board_t *copy_board(board_t* board) {
    board_t* copy = malloc(sizeof(board_t));

    copy->height = board->height;
    copy->width = board->width;
    copy->symbols = board->symbols;
    copy->current_player = board->current_player;
    copy->players = board->players;
    copy->matrix = copy_board_matrix(board);
    copy->scores = copy_board_scores(board);

    return copy;
}
void move_symbol(board_t *board, int i, int j, direction_t direction) {
    assert(i >= 0 && i < board->height);
    assert(j >= 0 && j < board->width);

    char save = board->matrix[i][j];

    if (direction == UP) {
        assert(i - 1 >= 0);
        board->matrix[i][j] = board->matrix[i - 1][j];
        board->matrix[i - 1][j] = save;
    } else if (direction == DOWN) {
        assert(i + 1 < board->height);
        board->matrix[i][j] = board->matrix[i + 1][j];
        board->matrix[i + 1][j] = save;
    } else if (direction == LEFT) {
        assert(j - 1 >= 0);
        board->matrix[i][j] = board->matrix[i][j - 1];
        board->matrix[i][j - 1] = save;
    } else if (direction == RIGHT) {
        assert(j + 1 < board->width);
        board->matrix[i][j] = board->matrix[i][j + 1];
        board->matrix[i][j + 1] = save;
    }
}

direction_t get_opposite_direction(direction_t direction) {
    if (direction == UP) {
        return DOWN;
    } else if (direction == DOWN) {
        return UP;
    } else if (direction == LEFT) {
        return RIGHT;
    }

    return LEFT;
}

bool is_valid_direction(board_t* board, int i, int j, direction_t direction) {
    assert(i >= 0 && i < board->height);
    assert(j >= 0 && j < board->width);

    if (direction == UP) {
        return i - 1 >= 0;
    } else if (direction == DOWN) {
        return i + 1 < board->height;
    } else if (direction == LEFT) {
        return j - 1 >= 0;
    }

    return j + 1 < board->width;
}

bool can_move_symbol(board_t *board, int i, int j, direction_t direction) {
    assert(i >= 0 && i < board->height);
    assert(j >= 0 && j < board->width);

    if (!is_valid_direction(board, i, j, direction)) {
        return false;
    }

    int di = 0;
    int dj = 1;

    if (direction == UP) {
        di = -1;
        dj = 0;
    } else if (direction == DOWN) {
        di = 1;
        dj = 0;
    } else if (direction == LEFT) {
        dj = -1;
    }

    move_symbol(board, i, j, direction);

    bool is_valid = is_aligned(board, i + di, j + dj);

    move_symbol(board, i + di, j + dj, get_opposite_direction(direction));

    return is_valid;
}

bool has_symbol_valid_move(board_t *board, int i, int j) {
    assert(i >= 0 && i < board->height);
    assert(j >= 0 && j < board->width);

    return can_move_symbol(board, i, j, UP)
           || can_move_symbol(board, i, j, DOWN)
           || can_move_symbol(board, i, j, LEFT)
           || can_move_symbol(board, i, j, RIGHT);
}

bool has_valid_move(board_t* board) {
    for (int i = 0; i < board->height; ++i) {
        for (int j = 0; j < board->width; ++j) {
            if (has_symbol_valid_move(board, i, j)) {
                return true;
            }
        }
    }

    return false;
}

void next_player(board_t *board) {
    board->current_player = (board->current_player + 1) % board->players;
}

void add_score(board_t *board, int score) {
    board->scores[board->current_player] += score;
}

int get_winner(board_t *board) {
    int winner = 0;
    int max_score = board->scores[0];

    for (int i = 1; i < board->players; ++i) {
        if (board->scores[i] > max_score) {
            winner = i;
            max_score = board->scores[i];
        }
    }
    return winner;
}

bool file_exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return true;
    }
    return false;
}

void save_score(char *nickname, int score) {
    bool exists = file_exists(SCORES_FILE);
    FILE *file = fopen(SCORES_FILE, "a+");

    if (file == NULL) {
        printf("Une erreur est survenue lors de la sauvegarde du score.\n");
        return;
    }

    if (!exists) {
        fprintf(file, "0\n");
    }

    fprintf(file, "%d %s\n", score, nickname);

    fclose(file);

    FILE *edit_amount_file = fopen(SCORES_FILE, "r+");

    if (edit_amount_file == NULL) {
        printf("Une erreur est survenue lors de la sauvegarde du score.\n");
        return;
    }

    /**
     * incrémentation du nombre de scores à la première ligne
     */
    int scores_count = 0;

    fseek(edit_amount_file, 0, SEEK_SET);
    int error = fscanf(edit_amount_file, "%d", &scores_count);

    if (error == EOF || error == 0) {
        scores_count = 0;
    }

    fseek(edit_amount_file, 0, SEEK_SET);
    fprintf(edit_amount_file, "%d\n", scores_count + 1);

    fclose(edit_amount_file);
}

void quick_sort(score_t* scores, int start, int end) {
    if (start < end) {
        int pivot = scores[end].score;
        int i = start - 1;

        for (int j = start; j < end; ++j) {
            if (scores[j].score > pivot) {
                ++i;
                score_t temp = scores[i];
                scores[i] = scores[j];
                scores[j] = temp;
            }
        }

        score_t temp = scores[i + 1];
        scores[i + 1] = scores[end];
        scores[end] = temp;

        quick_sort(scores, start, i);
        quick_sort(scores, i + 2, end);
    }
}

int max(int a, int b) {
    return a > b ? a : b;
}

score_t* load_scores() {
    FILE *file = fopen(SCORES_FILE, "r");

    if (file == NULL) {
        return NULL;
    }

    int amount = 0;
    int error = fscanf(file, "%d", &amount);

    if (error == EOF || error == 0) {
        amount = 0;
        fprintf(file, "%d\n", 0);
    }

    score_t* scores = malloc(sizeof(score_t) * max(amount, 10));

    for (int i = 0; i < max(amount, 10); ++i) {
        if (i >= amount) {
            scores[i].score = -1;
            strcpy(scores[i].nickname, "#");

            continue;
        }

        error = fscanf(file, "%d %s\n", &(scores[i].score), scores[i].nickname);

        if (i > amount || error == EOF || error == 0) {
            scores[i].score = -1;
            strcpy(scores[i].nickname, "#");
        }
    }

    fclose(file);

    quick_sort(scores, 0, amount - 1);

    return scores;
}

void save_board(board_t *board, int slot, int* error) {
    char filename[8];
    *error = 0;

    itoa(slot, filename, 10);

    strcat(filename, ".save");
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        *error = 1;
        return;
    }

    fprintf(file, "%d %d %d %d %d\n", board->height, board->width, board->players, board->current_player, board->symbols);

    for (int i = 0; i < board->players; ++i) {
        fprintf(file, "%d ", board->scores[i]);
    }

    fprintf(file, "\n");

    for (int i = 0; i < board->height; ++i) {
        for (int j = 0; j < board->width; ++j) {
            fprintf(file, "%c\n", board->matrix[i][j]);
        }
    }

    fclose(file);
}

board_t *load_board(int slot, int* error) {
    *error = 0;
    char filename[8];

    itoa(slot, filename, 10);

    strcat(filename, ".save");
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        *error = 1;
        return NULL;
    }

    int width, height, players, current_player, symbols;

    int data_error = fscanf(file, "%d %d %d %d %d", &height, &width, &players, &current_player, &symbols);

    if (data_error == EOF) {
        *error = 2;
        fclose(file);
        return NULL;
    }

    board_t *board = init_board(width, height, players, symbols);

    for (int i = 0; i < board->players; ++i) {
        int score_error = fscanf(file, "%d", &board->scores[i]);

        if (score_error == EOF) {
            *error = 2;
            free_board(board);
            fclose(file);
            return NULL;
        }
    }

    int line_error = fscanf(file, "\n");

    if (line_error == EOF) {
        *error = 2;
        free_board(board);
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < board->height; ++i) {
        for (int j = 0; j < board->width; ++j) {
            int matrix_error = fscanf(file, "%c\n", &board->matrix[i][j]);

            if (matrix_error == EOF) {
                *error = 2;
                free_board(board);
                fclose(file);
                return NULL;
            }
        }
    }

    fclose(file);

    return board;
}

int scanf_int(char* message, int minimum, int maximum) {
    printf("%s", message);

    int result;
    int scanf_result = scanf("%d", &result);

    if (scanf_result == EOF || scanf_result == 0) {
        printf("Erreur lors de la saisie, veuillez réessayer.\n");
        fflush(stdin);
        return scanf_int(message, minimum, maximum);
    } else if (result < minimum || result > maximum) {
        printf("Erreur: la valeur doit être comprise entre %d et %d.\n", minimum, maximum);
        fflush(stdin);
        return scanf_int(message, minimum, maximum);
    }

    return result;
}

int scanf_direction(char* message, board_t *board, int i, int j) {
    printf("%s", message);

    char result[6];
    direction_t parsed_result;
    int scanf_result = scanf("%s", &result, sizeof(result));

    if (scanf_result == EOF || scanf_result == 0) {
        printf("Erreur lors de la saisie, veuillez réessayer.\n");
        fflush(stdin);

        return scanf_direction(message, board, i, j);
    }

    printf("%s\n", result);

    if (strcmp(result, "up") == 0) {
        parsed_result = UP;
    } else if (strcmp(result, "down") == 0) {
        parsed_result = DOWN;
    } else if (strcmp(result, "left") == 0) {
        parsed_result = LEFT;
    } else if (strcmp(result, "right") == 0) {
        parsed_result = RIGHT;
    } else {
        printf("Erreur: la direction doit être up, down, left ou right.\n");
        fflush(stdin);
        return scanf_direction(message, board, i, j);
    }

    if (!can_move_symbol(board, i, j, parsed_result)) {
        printf("Erreur: le symbole ne peut pas être déplacé dans cette direction.\n");
        fflush(stdin);
        return scanf_direction(message, board, i, j);
    }

    return parsed_result;
}

bool scanf_bool(char* message) {
    printf("%s", message);

    char result[4];
    int scanf_result = scanf("%s", &result, sizeof(result));

    if (scanf_result == EOF || scanf_result == 0) {
        printf("Erreur lors de la saisie, veuillez réessayer.\n");
        fflush(stdin);
        return scanf_bool(message);
    }

    if (strcmp(result, "oui") == 0) {
        return true;
    } else if (strcmp(result, "non") == 0) {
        return false;
    } else {
        printf("Erreur: la réponse doit être oui ou non.\n");
        fflush(stdin);
        return scanf_bool(message);
    }
}

void scanf_str(char* message, char* result) {
    printf("%s", message);

    int scanf_result = scanf("%s", result, sizeof(result));

    printf("%d", scanf_result);

    if (scanf_result == EOF || scanf_result == 0) {
        printf("Erreur lors de la saisie, veuillez réessayer.\n");
        fflush(stdin);

        return scanf_str(message, result);
    }
}

void ask_coordinates(board_t *board, int *i, int *j, direction_t *direction) {
    *i = scanf_int("Entrez la coordonnée de la ligne: ", 0,  board->height - 1);
    *j = scanf_int("Entrez la coordonnée de la colone: ", 0, board->width - 1);

    if (!has_symbol_valid_move(board, *i, *j)) {
        printf("Erreur: le symbole ne peut pas être déplacé.\n");
        fflush(stdin);
        ask_coordinates(board, i, j, direction);
    }

    *direction = scanf_direction("Dans quelle direction ? (up, left, right, down): ", board, *i, *j);
}

int main() {
    setbuf(stdout, 0);
    srand(time(NULL));

    bool animations = scanf_bool("Voulez-vous activer les animations ? (oui, non): ");

    int load_error = 1;
    bool wants_load = true;
    board_t *board;

    while (load_error == 1 || load_error == 2) {
        wants_load = scanf_bool("Voulez-vous charger une partie ? (oui, non): ");

        if (!wants_load) break;

        int slot = scanf_int("Dans quelle slot voulez-vous sauvegarder la partie ? (1, 2, 3): ", 1, 3);
        board = load_board(slot, &load_error);

        if (load_error == 1) {
            printf("Erreur: la sauvegarde n'existe pas.\n");
        } else if(load_error == 2) {
            printf("Erreur: la sauvegarde est corrompue.\n");
        } else {
            sleep(1);
        }
    }

    clear();

    if (load_error == 1 || load_error == 2 || !wants_load) {
        int width, height, players, symbols;

        width = scanf_int("Entrez la largeur du plateau de jeu: ", 1, 1000);
        height = scanf_int("Entrez la hauteur du plateau de jeu: ", 1, 1000);
        players = scanf_int("Entrez le nombre de joueurs (maximum 2): ", 1, 2);
        symbols = scanf_int("Entrez le nombre de symboles (entre 4 et 6): ", 4, 6);

        board = init_board(width, height, players, symbols);
    }

    bool ended = false;

    while (has_valid_move(board)) {
        print_board(board);

        int player = board->current_player;

        printf("\n");
        printf("Joueur %d, à vous de jouer !\n", player + 1);

        int i, j;
        direction_t direction;

        printf("\n");
        ask_coordinates(board, &i, &j, &direction);

        move_symbol(board, i, j, direction);

        if (animations)
            print_board(board);

        while (has_aligned_symbols(board)) {
            if (animations)
                sleep(1);

            int points = remove_aligned_symbols(board);
            add_score(board, points);

            if (animations) {
                print_board(board);
                sleep(1);
            }

            int count = apply_gravity(board);

            if (animations && count > 0) {
                print_board(board);
                sleep(1);
            }

            replace_empty_symbols(board);

            if (animations)
                print_board(board);
        }

        print_board(board);

        if (has_valid_move(board)) {
            printf("\n");
            bool wants_save_party = scanf_bool("Voulez-vous sauvegarder la partie ? (oui, non): ");

            if (wants_save_party) {
                int slot = scanf_int("Dans quelle slot voulez-vous sauvegarder la partie ? (1, 2, 3): ", 1, 3);
                int save_error = 0;

                save_board(board, slot, &save_error);

                if (save_error == 1) {
                    printf("Erreur: la sauvegarde n'a pas pu être effectuée.\n");
                } else {
                    printf("Partie sauvegardée dans le slot %d !\n", slot);
                }

                sleep(1);
            }

            ended = scanf_bool("Voulez-vous arrêter la partie ? (oui, non): ");

            if (ended) {
                break;
            }
        }

        next_player(board);
    }

    if (ended) {
        return 0;
    }

    int winner = get_winner(board);

    if (board->players == 1) {
        printf("Vous avez gagné avec un score de %d !\n", board->scores[winner]);
    } else {
        printf("Joueur %d a gagné avec un score de %d !\n", winner + 1, board->scores[winner]);
    }

    bool wants_to_save = scanf_bool("Voulez-vous sauvegarder votre score ? (oui, non): ");

    if (wants_to_save) {
        char* nickname = malloc(20 * sizeof(char));
        scanf_str("Entrez votre pseudo: ", nickname);

        save_score(nickname, board->scores[winner]);
        free(nickname);
    }

    printf("Chargement des meilleurs scores...\n");

    score_t* scores = load_scores();

    printf("Meilleures scores: \n");

    if (scores == NULL) {
        printf("Une erreur est survenue lors du chargement des scores.\n");
        return 0;
    }

    for (int i = 0; i < 10; ++i) {
        if (scores[i].score != -1) {
            printf("%s : %d\n", scores[i].nickname, scores[i].score);
        } else {
            break;
        }
    }

    return 0;
}
