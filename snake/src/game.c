#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>

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
    if(g_game_over != 1){
        int oldCellLocation =  *(int*)ll_get_last(snake_p->snake_row) * width + *(int*)ll_get_last(snake_p->snake_col);
        int newCellLocation = *(int*)ll_get_first(snake_p->snake_row) * width + *(int*)ll_get_first(snake_p->snake_col);
        int row_change = 0;
        int col_change = 0;
        bool isGrowing = false;
        enum input_key oldDir = snake_p->snake_dir;

        if (input != INPUT_NONE) {
            bool reversing =
                (oldDir == INPUT_UP    && input == INPUT_DOWN) ||
                (oldDir == INPUT_DOWN  && input == INPUT_UP) ||
                (oldDir == INPUT_LEFT  && input == INPUT_RIGHT) ||
                (oldDir == INPUT_RIGHT && input == INPUT_LEFT);

            if (!reversing || ll_length(snake_p->snake_col) == 1) {
                snake_p->snake_dir = input;
            }
        }
        switch(snake_p->snake_dir){
            case INPUT_RIGHT:
                newCellLocation = newCellLocation + 1;
                col_change++;
            break;
            case INPUT_LEFT:
                newCellLocation = newCellLocation - 1;
                col_change--;
            break;
            case INPUT_UP:
                newCellLocation = newCellLocation - width;
                row_change--;
            break;
            case INPUT_DOWN:
                newCellLocation = newCellLocation + width;
                row_change++;
            break;
        }
        int cell_content = cells[newCellLocation];
        bool hitsSnake = (cell_content & FLAG_SNAKE) == FLAG_SNAKE;
        bool hitsTail = (newCellLocation == oldCellLocation);

        if (cell_content == FLAG_WALL ||
            (hitsSnake && !(hitsTail && !isGrowing))) {
            g_game_over = 1;
        }else{
            if((cell_content & FLAG_FOOD) == FLAG_FOOD){
                g_score++;
                place_food(cells, width, height);
                cells[newCellLocation] = cells[newCellLocation] ^ FLAG_FOOD;
                if(growing == 1 ){
                    if(g_EC == 1){
                        grow_remaining += (growPerFood);
                    }else{
                        grow_remaining++;
                    }
                }
            }
            if(grow_remaining > 0){
                isGrowing = true;
                grow_remaining -= 1;
            }
            if(!isGrowing){
                cells[oldCellLocation] = cells[oldCellLocation] ^ FLAG_SNAKE;
            }
            cells[newCellLocation] = FLAG_SNAKE | cells[newCellLocation];
        }
        int* newRow = malloc(sizeof(int));
        int* newCol = malloc(sizeof(int));
        *newRow = (*(int*)ll_get_first(snake_p->snake_row) + row_change);
        *newCol = (*(int*)ll_get_first(snake_p->snake_col) + col_change);
        ll_insert_first(&(snake_p->snake_row), newRow);
        ll_insert_first(&(snake_p->snake_col), newCol);

        //Extra credit 
        

        if(!isGrowing){
            int* oldRow = ll_remove_last(&(snake_p->snake_row));
            int* oldCol = ll_remove_last(&(snake_p->snake_col));
            free(oldRow);
            free(oldCol);
        }


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
    // TODO: implement! 
    
    printf("Name > ");
    fflush(stdout);
    
    int bytes_read = read(0, write_into, 1000);
    if (bytes_read <= 1) {
        if (bytes_read == 0) {
            printf("Name Invalid: must be longer than 0 characters.\n");
        } else {
            printf("An error has occurred, please try again.\n");
        }
        read_name(write_into); 
    } else {
        write_into[bytes_read - 1] = '\0'; 
    }
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
    free(cells);
    // TODO: implement! (part 3A)

    while((snake_p->snake_row)!= NULL){
        int* row = (int*)ll_remove_first(&(snake_p->snake_row));
        free(row);
    }

    while((snake_p->snake_col)!= NULL){
        int* col = (int*)ll_remove_first(&(snake_p->snake_col));
        free(col);
    }
}


//EXTRA CREDIT FUNCTIONS

double euclidean_distance(int x1, int y1, int x2, int y2){
    return sqrt((x2-x1) * (x2-x1) + (y2-y1) * (y2-y1));
}

double find_distance(int* cells, int width, int cur_row, int cur_col, int col_change, int row_change, 
    int food_row, int food_col){
    int new_row = cur_row + row_change;
    int new_col = cur_col + col_change;
    if((cells[new_row * width + new_col] & FLAG_WALL) == FLAG_WALL 
    || (cells[new_row * width + new_col] & FLAG_SNAKE) == FLAG_SNAKE){
        return INFINITY;
    }
    return euclidean_distance(new_col, new_row, food_col, food_row);
}

int compare_doubles_asc(const void *a, const void *b) {
    double doubleA = *(const double *)a;
    double doubleB = *(const double *)b;

    if (doubleA < doubleB) return -1;
    if (doubleA > doubleB) return 1;
    return 0;
}
//Greedy Best-First Search using Euclidean heuristic
enum input_key greedy_bfs(int* cells, size_t width, size_t height, snake_t* snake_p){
    int cur_row = *(int*)ll_get_first(snake_p->snake_row);
    int cur_col = *(int*)ll_get_first(snake_p->snake_col);
    int board_size = width * height;
    int food_row = -1;
    int food_col = -1;
    for(int i=0; i< board_size; i++){
        if((cells[i] & FLAG_FOOD) == FLAG_FOOD){
            food_row = i / width;
            food_col = i % width;
        }
    }
    if(food_col == -1 || food_row == -1){
        printf("ERROR NO FOOD FOUND");
        return INPUT_NONE;
    }
    enum input_key oldDir = snake_p->snake_dir;
    double distance_list[4];

    distance_list[0] = find_distance(cells, width, cur_row, cur_col, -1, 0, food_row, food_col); //left
    distance_list[1] = find_distance(cells, width, cur_row, cur_col,  1, 0, food_row, food_col); //right
    distance_list[2] = find_distance(cells, width, cur_row, cur_col, 0, -1, food_row, food_col); //up
    distance_list[3] = find_distance(cells, width, cur_row, cur_col, 0, 1, food_row, food_col); //down




    int i = 0;
    double current_min = INFINITY;
    enum input_key current_min_direction;
    while(i<4){
        double distance = distance_list[i];
        enum input_key old_min_direction = current_min_direction;
        if(distance < current_min){
            switch(i){
                case 0:
                current_min_direction = INPUT_LEFT;
                break;
                case 1:
                current_min_direction = INPUT_RIGHT;
                break;
                case 2:
                current_min_direction = INPUT_UP;
                break;
                case 3:
                current_min_direction = INPUT_DOWN;
                break;
            }
            bool reversing =
                (oldDir == INPUT_UP    && current_min_direction == INPUT_DOWN) ||
                (oldDir == INPUT_DOWN  && current_min_direction == INPUT_UP) ||
                (oldDir == INPUT_LEFT  && current_min_direction == INPUT_RIGHT) ||
                (oldDir == INPUT_RIGHT && current_min_direction == INPUT_LEFT);
            if(!reversing){
                current_min = distance;
            }else{
                current_min_direction = old_min_direction;
            }
        }
        i++;        
    }
    if(current_min == INFINITY){
        return INPUT_NONE;
    }
    
    return current_min_direction;
}