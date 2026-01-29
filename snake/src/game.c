#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <assert.h>

#include "linked_list.h"
#include "mbstrings.h"
#include "common.h"

/** Updates the game by a single step, and modifies the game information
 * accordingly. Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - width: width of the board.
 *  - height: height of the board.
 *  - snake_p: pointer to your snake struct (not used until part 3!)
 *  - input: the next input.
 *  - growing: 0 if the snake does not grow on eating, 1 if it does.
 */
void update(int* cells, size_t width, size_t height, snake_t* snake_p,
            enum input_key input, int growing) {
    // `update` should update the board, your snake's data, and global
    // variables representing game information to reflect new state. If in the
    // updated position, the snake runs into a wall or itself, it will not move
    // and global variable g_game_over will be 1. Otherwise, it will be moved
    // to the new position. If the snake eats food, the game score (`g_score`)
    // increases by 1. This function assumes that the board is surrounded by
    // walls, so it does not handle the case where a snake runs off the board.

    // TODO: implement!
    if(input == INPUT_NONE && g_game_over != 1){
        int oldCellLocatition = snake_row * 20 + snake_col;
        int newCellLocatition = snake_row * 20 + snake_col;

        int row_change = 0;
        int col_change = 0;
        //printf("\n %d \n", newCellLocatition);
        switch(snake_dir){
            case INPUT_RIGHT:
                newCellLocatition = newCellLocatition + 1;
                col_change++;
            break;
        }
        //printf("\n %d \n", newCellLocatition);
        int cell_content = cells[newCellLocatition];
        if(cell_content == FLAG_WALL || FLAG_SNAKE == (cell_content & FLAG_SNAKE)){
            g_game_over = 1;
        }else if(cell_content == FLAG_GRASS){
            if(cells[oldCellLocatition] == FLAG_SNAKE){
                cells[oldCellLocatition] = PLAIN_CELL;
            }else{
                cells[oldCellLocatition] = cell_content ^ FLAG_SNAKE;
            }
            cells[newCellLocatition] = FLAG_SNAKE | FLAG_GRASS;
        }else{
            cells[oldCellLocatition] = PLAIN_CELL;
            cells[newCellLocatition] = FLAG_SNAKE;
        }
        snake_row += row_change;
        snake_col += col_change;


    }
}

/** Sets a random space on the given board to food.
 * Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - width: the width of the board
 *  - height: the height of the board
 */
void place_food(int* cells, size_t width, size_t height) {
    /* DO NOT MODIFY THIS FUNCTION */
    size_t max = width * height;
    size_t attempts = 0;

    while(attempts < max) {
        unsigned int food_index = generate_index(width * height);
        // check that the cell is empty or only contains grass
        if ((*(cells + food_index) == PLAIN_CELL) ||
            (*(cells + food_index) == FLAG_GRASS)) {
            *(cells + food_index) |= FLAG_FOOD;
            return;
        }
        attempts++;
    }

    assert(0 && "ERROR:  No empty or grass-only cells to place food on board.  Are you sure the board has been initialized correctly?");
    /* DO NOT MODIFY THIS FUNCTION */
}

/** Prompts the user for their name and saves it in the given buffer.
 * Arguments:
 *  - `write_into`: a pointer to the buffer to be written into.
 */
void read_name(char* write_into) {
    // TODO: implement! (remove the call to strcpy once you begin your
    // implementation)
    strcpy(write_into, "placeholder");
}

/** Cleans up on game over — should free any allocated memory so that the
 * LeakSanitizer doesn't complain.
 * Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - snake_p: a pointer to your snake struct. (not needed until part 3)
 */
void teardown(int* cells, snake_t* snake_p) {  
    // TODO: Free the board

    // TODO: implement! (part 3A)
}
