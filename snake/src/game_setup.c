#include "game_setup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "game.h"
#include "common.h"
#include <string.h>

// Some handy dandy macros for decompression
#define E_CAP_HEX 0x45
#define E_LOW_HEX 0x65
#define G_CAP_HEX 0x47
#define G_LOW_HEX 0x67
#define S_CAP_HEX 0x53
#define S_LOW_HEX 0x73
#define W_CAP_HEX 0x57
#define W_LOW_HEX 0x77
#define DIGIT_START 0x30
#define DIGIT_END 0x39

/** Initializes the board with walls around the edge of the board, and a ring
 * of grass just inside the wall.
 *
 * Modifies values pointed to by cells_p, width_p, and height_p and initializes
 * cells array to reflect this default board.
 *
 * Returns INIT_SUCCESS to indicate that it was successful.
 *
 * Arguments:
 *  - cells_p: a pointer to a memory location where a pointer to the first
 *             element in a newly initialized array of cells should be stored.
 *  - width_p: a pointer to a memory location where the newly initialized
 *             width should be stored.
 *  - height_p: a pointer to a memory location where the newly initialized
 *              height should be stored.
 */
enum board_init_status initialize_default_board(int** cells_p, size_t* width_p,
                                                size_t* height_p) {
    *width_p = 20;
    *height_p = 10;
    int* cells = malloc(20 * 10 * sizeof(int));
    *cells_p = cells;
    for (int i = 0; i < 20 * 10; i++) {
        cells[i] = PLAIN_CELL;
    }

    // Set edge cells!
    // Top and bottom edges:
    for (int i = 0; i < 20; ++i) {
        cells[i] = FLAG_WALL;
        cells[i + (20 * (10 - 1))] = FLAG_WALL;
    }
    // Left and right edges:
    for (int i = 0; i < 10; ++i) {
        cells[i * 20] = FLAG_WALL;
        cells[i * 20 + 20 - 1] = FLAG_WALL;
    }

    // Set grass cells!
    // Top and bottom edges:
    for (int i = 1; i < 19; ++i) {
        cells[i + 20] = FLAG_GRASS;
        cells[i + (20 * (9 - 1))] = FLAG_GRASS;
    }
    // Left and right edges:
    for (int i = 1; i < 9; ++i) {
        cells[i * 20 + 1] = FLAG_GRASS;
        cells[i * 20 + 19 - 1] = FLAG_GRASS;
    }

    // Add snake
    cells[20 * 2 + 2] = FLAG_SNAKE;

    return INIT_SUCCESS;
}

/** Initialize variables relevant to the game board.
 * Arguments:
 *  - cells_p: a pointer to a memory location where a pointer to the first
 *             element in a newly initialized array of cells should be stored.
 *  - width_p: a pointer to a memory location where the newly initialized
 *             width should be stored.
 *  - height_p: a pointer to a memory location where the newly initialized
 *              height should be stored.
 *  - snake_p: a pointer to your snake struct (not used until part 3!)
 *  - board_rep: a string representing the initial board. May be NULL for
 * default board.
 */
enum board_init_status initialize_game(int** cells_p, size_t* width_p,
                                       size_t* height_p, snake_t* snake_p,
                                       char* board_rep) {
    // TODO: implement!
    //board_rep = "B7x10|W10|W1E4W5|W2E5G2W1|W1E8W1|W1E4W1E3W1|W1E2S1E1W1E3W1|W10";
    board_rep = "B80x24|W80|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W1E9S1E68W1|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W1E78W1|W80";
    if(board_rep != NULL){
        decompress_board_str(cells_p, width_p, height_p, snake_p, board_rep);
    }else{
        snake_col = 2; 
        snake_row = 2;
        initialize_default_board(cells_p, width_p, height_p);
    }
    snake_dir = INPUT_RIGHT;
    snake_dir = INPUT_UP;
    place_food(*cells_p, *width_p, *height_p);
    return INIT_SUCCESS;
}





void set_cell(int** cells_p, int pos, int count, char type){
    for(int i = pos; i < pos + count; i++){
        switch(type){
            case 'E':
                (*cells_p)[i]  = PLAIN_CELL;
            break;
            case 'S':
                (*cells_p)[i]  = FLAG_SNAKE;
            break;
            case 'W':
                (*cells_p)[i]  = FLAG_WALL;
            break;
            case 'G':
                (*cells_p)[i]  = FLAG_GRASS;
            break;
        }
    }
}

/*
Sets the size parameters of the board with the given String
*/

int set_board_size(size_t* width_p, size_t* height_p, char* string){

    int width;
    int height;
    int number_of_items = sscanf(string, "B%dx%d", &width, &height);
    if(number_of_items == 2){
    *height_p = height;
    *width_p = width;
    }else{
        return 1;
    }
    return 0;
}

/*
Sets the size parameters of the board with the given String
*/

enum board_init_status set_board_cells(int** cells_p, char* string, int size, int* pos){
    int len = strlen(string); 
    int i=0;
    while(i < len && *pos < size){
        char cell_type;
        switch(string[i]){
            case 'E':
                cell_type = 'E';
            break;
            case 'S':
                cell_type = 'S';
            break;
            case 'W':
                cell_type = 'W';
            break;
            case 'G':
                cell_type = 'G';
            break;
            case '\0':
                return INIT_SUCCESS;
            break;
            default:
                return INIT_ERR_BAD_CHAR;
            break;
        }
        i++;
        char number[16] = {0};
        while(i < len && (int)string[i] < 58 && (int)string[i] > 46){
            strncat(number, &string[i], 1);
            i++;
        }
        int count = strtol(number, NULL, 10);
        set_cell(cells_p, *pos, count, cell_type);
        *pos += count;
    }
    return INIT_SUCCESS;
}

int check_board_error(int** cells_p, int size) {
    if (cells_p == NULL || *cells_p == NULL) {
        return INIT_ERR_INCORRECT_DIMENSIONS;
    }

    int count_of_snake = 0;

    for (int i = 0; i < size; i++) {
        if ((*cells_p)[i] == FLAG_SNAKE) {
            count_of_snake++;
        }
    }

    if (count_of_snake != 1) {
        return INIT_ERR_WRONG_SNAKE_NUM;
    }

    return INIT_SUCCESS;
}

void print_board(int* cells, size_t width, size_t height) {
    for (size_t r = 0; r < height; r++) {
        for (size_t c = 0; c < width; c++) {
            int cell = cells[r * width + c];
            char out;

            switch (cell) {
                case FLAG_WALL:  out = 'W'; break;
                case FLAG_GRASS: out = 'G'; break;
                case FLAG_SNAKE: out = 'S'; break;
                case PLAIN_CELL: out = 'E'; break;
                default:         out = '?'; break;
            }

            printf("%c", out);
        }
        printf("\n");
    }
}


/** Takes in a string `compressed` and initializes values pointed to by
 * cells_p, width_p, and height_p accordingly. Arguments:
 *      - cells_p: a pointer to the pointer representing the cells array
 *                 that we would like to initialize.
 *      - width_p: a pointer to the width variable we'd like to initialize.
 *      - height_p: a pointer to the height variable we'd like to initialize.
 *      - snake_p: a pointer to your snake struct (not used until part 3!)
 *      - compressed: a string that contains the representation of the board.
 * Note: We assume that the string will be of the following form:
 * B24x80|E5W2E73|E5W2S1E72... To read it, we scan the string row-by-row
 * (delineated by the `|` character), and read out a letter (E, S or W) a number
 * of times dictated by the number that follows the letter.
 */
enum board_init_status decompress_board_str(int** cells_p, size_t* width_p,
                                            size_t* height_p, snake_t* snake_p,
                                            char* compressed) {
    // TODO: implement!
    // sample string B7x10|W10|W1E4W5|W2E5G2W1|W1E8W1|W1E4W1E3W1|W1E2S1E1W1E3W1|W10
    
    char* compressed_copy = strdup(compressed); 
    char* token;
    const char* delim = "|";
    char* original_str;
    token = strtok_r(compressed_copy, delim, &original_str);
    if(set_board_size(width_p, height_p, token) == 1){
        return INIT_ERR_BAD_CHAR;
    }
    *cells_p = malloc((*width_p) * (*height_p) * sizeof(int));
    int total_cells = (*width_p) * (*height_p);
    for (int i = 0; i < total_cells; i++) {
        (*cells_p)[i] = PLAIN_CELL;
    }                     
    token = strtok_r(NULL, delim, &original_str);
    int pos = 0;
    while(token != NULL){
        if(set_board_cells(cells_p, token, total_cells, &pos) == INIT_ERR_BAD_CHAR ){
            return  INIT_ERR_BAD_CHAR;
        }
        token = strtok_r(NULL, delim, &original_str); //Next Token
    }
    
    print_board(*cells_p, *width_p, *height_p);
    printf(" %ld   %ld \n", *width_p, *height_p);
    for (int i = 0; i < total_cells; i++) {
        if ((*cells_p)[i] == FLAG_SNAKE) {
            snake_row = i / (*width_p);
            snake_col = i % (*width_p);
        }
    }
    free(compressed_copy);

    return check_board_error(cells_p, total_cells);
}



