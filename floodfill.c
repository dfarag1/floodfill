#include "solver.h"
#include <stdio.h>
#include <stdlib.h>

#include "controller.h"
#include "pid.h"
#include "irs.h"

// --- Start of Size Configuration ---
#define MAZE_HEIGHT 16 // Define the height of the maze
#define MAZE_WIDTH  16 // Define the width of the maze
// --- End of Size Configuration ---
extern int g_turn_priority_left_first;

#define QUEUE_MAX_CELLS (MAZE_HEIGHT * MAZE_WIDTH) // Max items in queue

struct pair {
    short r;
    short c;
};

typedef struct {
    struct pair items[QUEUE_MAX_CELLS]; // Adjusted queue size
    int front;
    int rear;
} Queue;

void initializeQueue(Queue *q) {
    q->front = -1;
    q->rear = -1;
}

int isFull(Queue *q) {
    return (q->rear == QUEUE_MAX_CELLS - 1); // Adjusted check
}

int isEmpty(Queue *q) {
    return (q->front == -1);
}

void enqueue(Queue *q, struct pair value) {
    if (isFull(q)) {
        // Consider more robust error handling for a dynamic system
        printf("Queue is full!\n");
    } else {
        if (q->front == -1)
            q->front = 0;
        q->rear++;
        q->items[q->rear] = value;
    }
}

struct pair dequeue(Queue *q) {
    if (isEmpty(q)) {
        // Consider more robust error handling
        printf("Queue is empty!\n");
        struct pair l = {-1, -1}; // Indicate error or empty
        return l;
    } else {
        struct pair item = q->items[q->front];
        q->front++;
        if (q->front > q->rear) {
            initializeQueue(q); // Reset queue when empty
        }
        return item;
    }
}

struct node {
    short left, right, bot, top;
    short row, col;
    short dist;
};

void initialize(struct node* n, short l, short r, short b, short t, short ro, short c, short d) {
    n->left = l;
    n->right = r;
    n->bot = b;
    n->top = t;
    n->row = ro;
    n->col = c;
    n->dist = d;
}

// Global state - consider encapsulating into a struct if maze dimensions are passed at runtime
int row = MAZE_HEIGHT -1; // Start at bottom-left, assuming (0,0) is top-left for array
int col = 0;
int x = 0; // API coordinates, may need adjustment based on how they map
int y = 0; // API coordinates
char dir = 'n'; // Initial direction

// Declare the maze array with defined dimensions
struct node array[MAZE_HEIGHT][MAZE_WIDTH];

int first = 1;


// Calculates Manhattan distances to the center of the maze
void calcManToCenter(struct node maze[MAZE_HEIGHT][MAZE_WIDTH]) {
    Queue q;
    initializeQueue(&q);
    struct pair p;

    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            maze[i][j].dist = -1;
        }
    }

    // Center goals - assuming a 2x2 center for even dimensions, or adjust as needed
    // For odd dimensions, you might pick a single center cell.
    // This example targets the block of 4 cells closest to the geometric center.
    int center_r_start = (MAZE_HEIGHT / 2) - 1;
    int center_r_end = MAZE_HEIGHT / 2;
    int center_c_start = (MAZE_WIDTH / 2) - 1;
    int center_c_end = MAZE_WIDTH / 2;

    // Ensure indices are valid, especially for small mazes
    if (center_r_start < 0) center_r_start = 0;
    if (center_r_end >= MAZE_HEIGHT) center_r_end = MAZE_HEIGHT -1;
    if (center_c_start < 0) center_c_start = 0;
    if (center_c_end >= MAZE_WIDTH) center_c_end = MAZE_WIDTH -1;


    for (int i = center_r_start; i <= center_r_end; i++) {
        for (int j = center_c_start; j <= center_c_end; j++) {
             if (i < MAZE_HEIGHT && j < MAZE_WIDTH) { // Bounds check
                maze[i][j].dist = 0;
                p.r = i;
                p.c = j;
                enqueue(&q, p);
            }
        }
    }

    while (!isEmpty(&q)) {
        struct pair current = dequeue(&q);
        int r = current.r;
        int c = current.c;
        int dist = maze[r][c].dist;

        // WEST
        if (c > 0 && maze[r][c].left == 0 && maze[r][c - 1].dist == -1) {
            maze[r][c - 1].dist = dist + 1;
            p.r = r; p.c = c - 1;
            enqueue(&q, p);
        }
        // EAST
        if (c < MAZE_WIDTH - 1 && maze[r][c].right == 0 && maze[r][c + 1].dist == -1) {
            maze[r][c + 1].dist = dist + 1;
            p.r = r; p.c = c + 1;
            enqueue(&q, p);
        }
        // NORTH
        if (r > 0 && maze[r][c].top == 0 && maze[r - 1][c].dist == -1) {
            maze[r - 1][c].dist = dist + 1;
            p.r = r - 1; p.c = c;
            enqueue(&q, p);
        }
        // SOUTH
        if (r < MAZE_HEIGHT - 1 && maze[r][c].bot == 0 && maze[r + 1][c].dist == -1) {
            maze[r + 1][c].dist = dist + 1;
            p.r = r + 1; p.c = c;
            enqueue(&q, p);
        }
    }
    /*
    for(int i = 0; i < MAZE_WIDTH; i++){//x (API_setText col index)
        for(int j = 0; j < MAZE_HEIGHT; j++){//y (API_setText row index, but maze is 15-j)
            char str[10];
            // Assuming array[j][i] in API_setText means array[row_idx][col_idx]
            // And API (0,0) is bottom left, while array (0,0) is top left.
            // So API row 'y' = (MAZE_HEIGHT - 1) - array_row_idx
            // API col 'x' = array_col_idx
            sprintf(str, "%d", maze[j][i].dist); // This seems to be array[row][col]
            // API_setText( i , (MAZE_HEIGHT - 1) - j, str); // Corrected mapping
        }
    }
    */
}

// Calculates Manhattan distances to the starting cell (e.g., MAZE_HEIGHT-1, 0)
void calcManToStart(struct node maze[MAZE_HEIGHT][MAZE_WIDTH], int startR, int startC) {
    Queue q;
    initializeQueue(&q);
    struct pair p;

    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            maze[i][j].dist = -1;
        }
    }

    maze[startR][startC].dist = 0; // Target the specific start cell
    p.r = startR;
    p.c = startC;
    enqueue(&q, p);

    while (!isEmpty(&q)) {
        struct pair current = dequeue(&q);
        int r = current.r;
        int c = current.c;
        int dist = maze[r][c].dist;

        // WEST
        if (c > 0 && maze[r][c].left == 0 && maze[r][c - 1].dist == -1) {
            maze[r][c - 1].dist = dist + 1;
            p.r = r; p.c = c - 1;
            enqueue(&q, p);
        }
        // EAST
        if (c < MAZE_WIDTH - 1 && maze[r][c].right == 0 && maze[r][c + 1].dist == -1) {
            maze[r][c + 1].dist = dist + 1;
            p.r = r; p.c = c + 1;
            enqueue(&q, p);
        }
        // NORTH
        if (r > 0 && maze[r][c].top == 0 && maze[r - 1][c].dist == -1) {
            maze[r - 1][c].dist = dist + 1;
            p.r = r - 1; p.c = c;
            enqueue(&q, p);
        }
        // SOUTH
        if (r < MAZE_HEIGHT - 1 && maze[r][c].bot == 0 && maze[r + 1][c].dist == -1) {
            maze[r + 1][c].dist = dist + 1;
            p.r = r + 1; p.c = c;
            enqueue(&q, p);
        }
    }
    // ... (API_setText loop similar to calcManToCenter if needed) ...
}


void markWall(struct node maze[MAZE_HEIGHT][MAZE_WIDTH], int current_row, int current_col, int api_x, int api_y, char current_dir) {
    // Assuming readLeftIR etc. are external functions
    int Left = readLeftIR();
    int frontRight = readFrontRightIR();
    int frontLeft = readFrontLeftIR();
    int Right = readRightIR();

    // EDIT THIS IN ORDER TO CHANGE THE VALUE AT WHICH AN IR READS A WALL
    int wallThreshold = 1000; // Use a variable or #define
    int hasWallLeft = (Left > wallThreshold);       // Corrected logic: > threshold means wall detected
    int hasWallRight = (Right > wallThreshold);     // Corrected logic
    int hasWallFront = (frontRight > wallThreshold && frontLeft > wallThreshold); // Corrected logic

    if (current_dir == 'n') {
        if (hasWallFront) {
            maze[current_row][current_col].top = 1;
            if (current_row - 1 >= 0) maze[current_row - 1][current_col].bot = 1;
            // API_setWall(api_x, api_y, 'n');
        }
        if (hasWallLeft) {
            maze[current_row][current_col].left = 1;
            if (current_col - 1 >= 0) maze[current_row][current_col - 1].right = 1;
            // API_setWall(api_x, api_y, 'w');
        }
        if (hasWallRight) {
            maze[current_row][current_col].right = 1;
            if (current_col + 1 < MAZE_WIDTH) maze[current_row][current_col + 1].left = 1;
            // API_setWall(api_x, api_y, 'e');
        }
    } else if (current_dir == 's') {
        if (hasWallFront) {
            maze[current_row][current_col].bot = 1;
            if (current_row + 1 < MAZE_HEIGHT) maze[current_row + 1][current_col].top = 1;
            // API_setWall(api_x, api_y, 's');
        }
        if (hasWallLeft) { // Facing South, left is East
            maze[current_row][current_col].right = 1;
            if (current_col + 1 < MAZE_WIDTH) maze[current_row][current_col + 1].left = 1;
            // API_setWall(api_x, api_y, 'e');
        }
        if (hasWallRight) { // Facing South, right is West
            maze[current_row][current_col].left = 1;
            if (current_col - 1 >= 0) maze[current_row][current_col - 1].right = 1;
            // API_setWall(api_x, api_y, 'w');
        }
    } else if (current_dir == 'e') {
        if (hasWallFront) {
            maze[current_row][current_col].right = 1;
            if (current_col + 1 < MAZE_WIDTH) maze[current_row][current_col + 1].left = 1;
            // API_setWall(api_x, api_y, 'e');
        }
        if (hasWallLeft) { // Facing East, left is North
            maze[current_row][current_col].top = 1;
            if (current_row - 1 >= 0) maze[current_row - 1][current_col].bot = 1;
            // API_setWall(api_x, api_y, 'n');
        }
        if (hasWallRight) { // Facing East, right is South
            maze[current_row][current_col].bot = 1;
            if (current_row + 1 < MAZE_HEIGHT) maze[current_row + 1][current_col].top = 1;
            // API_setWall(api_x, api_y, 's');
        }
    } else if (current_dir == 'w') {
        if (hasWallFront) {
            maze[current_row][current_col].left = 1;
            if (current_col - 1 >= 0) maze[current_row][current_col - 1].right = 1;
            // API_setWall(api_x, api_y, 'w');
        }
        if (hasWallLeft) { // Facing West, left is South
            maze[current_row][current_col].bot = 1;
            if (current_row + 1 < MAZE_HEIGHT) maze[current_row + 1][current_col].top = 1;
            // API_setWall(api_x, api_y, 's');
        }
        if (hasWallRight) { // Facing West, right is North
            maze[current_row][current_col].top = 1;
            if (current_row - 1 >= 0) maze[current_row - 1][current_col].bot = 1;
            // API_setWall(api_x, api_y, 'n');
        }
    }
}

// ... (includes, defines, extern declaration as above) ...
// ... (Queue, node, initialize, global state variables, calcManToCenter, calcManToStart, markWall) ...


// External variable for turn priority (already declared above)
// extern int g_turn_priority_left_first;

int returning = 0;
int justSwitched = 0;

Action returnToStart() {
    int startR = MAZE_HEIGHT - 1;
    int startC = 0;

    if (justSwitched == 1) {
        calcManToStart(array, startR, startC);
        justSwitched = 0;
    }

    if (array[row][col].dist == 0) {
        returning = 0;
        justSwitched = 1;
        return IDLE;
    }

    markWall(array, row, col, x, y, dir);

    int currentDist = array[row][col].dist;
    int low = currentDist;

    // Determine the best path by checking all neighbors
    // These store the distance to the cell if a valid path exists, otherwise a very high number or -1
    int dist_N = -1, dist_S = -1, dist_E = -1, dist_W = -1;

    if (row > 0 && array[row][col].top == 0 && array[row-1][col].dist != -1) dist_N = array[row-1][col].dist;
    if (row < MAZE_HEIGHT - 1 && array[row][col].bot == 0 && array[row+1][col].dist != -1) dist_S = array[row+1][col].dist;
    if (col < MAZE_WIDTH - 1 && array[row][col].right == 0 && array[row][col+1].dist != -1) dist_E = array[row][col+1].dist;
    if (col > 0 && array[row][col].left == 0 && array[row][col-1].dist != -1) dist_W = array[row][col-1].dist;

    // Find the minimum valid distance among neighbors
    if (dist_N != -1 && dist_N < low) low = dist_N;
    if (dist_S != -1 && dist_S < low) low = dist_S;
    if (dist_E != -1 && dist_E < low) low = dist_E;
    if (dist_W != -1 && dist_W < low) low = dist_W;


    if (low == currentDist || low == -1 ) { // No valid path to a cell with strictly lower distance or stuck
        calcManToStart(array, startR, startC);
        return IDLE;
    } else {
        // Decide move based on direction and priorities
        if (dir == 'n') {
            if (dist_N == low) { row--; y++; return FORWARD; }
            if (g_turn_priority_left_first) { // Prioritize Left (West for North-facing)
                if (dist_W == low) { dir = 'w'; return LEFT; }
                if (dist_E == low) { dir = 'e'; return RIGHT; }
            } else { // Prioritize Right (East for North-facing)
                if (dist_E == low) { dir = 'e'; return RIGHT; }
                if (dist_W == low) { dir = 'w'; return LEFT; }
            }
            if (dist_S == low) { while(!PIDdone()){} turn(2); dir = 's'; return IDLE; } // Turn around
        } else if (dir == 's') {
            if (dist_S == low) { row++; y--; return FORWARD; }
            if (g_turn_priority_left_first) { // Prioritize Left (East for South-facing)
                if (dist_E == low) { dir = 'e'; return LEFT; }
                if (dist_W == low) { dir = 'w'; return RIGHT; }
            } else { // Prioritize Right (West for South-facing)
                if (dist_W == low) { dir = 'w'; return RIGHT; }
                if (dist_E == low) { dir = 'e'; return LEFT; }
            }
            if (dist_N == low) { while(!PIDdone()){} turn(2); dir = 'n'; return IDLE; }
        } else if (dir == 'e') {
            if (dist_E == low) { col++; x++; return FORWARD; }
            if (g_turn_priority_left_first) { // Prioritize Left (North for East-facing)
                if (dist_N == low) { dir = 'n'; return LEFT; }
                if (dist_S == low) { dir = 's'; return RIGHT; }
            } else { // Prioritize Right (South for East-facing)
                if (dist_S == low) { dir = 's'; return RIGHT; }
                if (dist_N == low) { dir = 'n'; return LEFT; }
            }
            if (dist_W == low) { while(!PIDdone()){} turn(2); dir = 'w'; return IDLE; }
        } else if (dir == 'w') {
            if (dist_W == low) { col--; x--; return FORWARD; }
            if (g_turn_priority_left_first) { // Prioritize Left (South for West-facing)
                if (dist_S == low) { dir = 's'; return LEFT; }
                if (dist_N == low) { dir = 'n'; return RIGHT; }
            } else { // Prioritize Right (North for West-facing)
                if (dist_N == low) { dir = 'n'; return RIGHT; }
                if (dist_S == low) { dir = 's'; return LEFT; }
            }
            if (dist_E == low) { while(!PIDdone()){} turn(2); dir = 'e'; return IDLE; }
        }
    }
    // Fallback: if somehow no action was chosen but low was better, recalculate.
    // This shouldn't be reached if logic is correct.
    calcManToStart(array, startR, startC);
    return IDLE;
}


Action floodFill() {
    if (first == 1) {
        // ... (initialization as before) ...
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                initialize(&array[i][j], 0, 0, 0, 0, i, j, -1);
            }
        }
        calcManToCenter(array);
        first = 0;
        row = MAZE_HEIGHT - 1; col = 0; x = 0; y = 0; dir = 'n';
    }

    markWall(array, row, col, x, y, dir);

    if (justSwitched == 1) {
        calcManToCenter(array);
        justSwitched = 0;
    }

    int center_r_start = (MAZE_HEIGHT / 2) - 1; // ... (center goal definitions) ...
    int center_r_end = MAZE_HEIGHT / 2;
    int center_c_start = (MAZE_WIDTH / 2) - 1;
    int center_c_end = MAZE_WIDTH / 2;
    // Ensure valid center indices for small mazes
    if (center_r_start < 0) center_r_start = 0;
    if (center_r_end >= MAZE_HEIGHT) center_r_end = MAZE_HEIGHT -1;
    if (center_c_start < 0) center_c_start = 0;
    if (center_c_end >= MAZE_WIDTH) center_c_end = MAZE_WIDTH -1;

    int isAtCenterGoal = 0;
    if (row >= center_r_start && row <= center_r_end && col >= center_c_start && col <= center_c_end && array[row][col].dist == 0) {
        isAtCenterGoal = 1;
    }

    if (isAtCenterGoal || returning == 1) {
        returning = 1;
        justSwitched = 1;
        return returnToStart();
    }

    int currentDist = array[row][col].dist;
    int low = currentDist;

    int dist_N = -1, dist_S = -1, dist_E = -1, dist_W = -1;
    if (row > 0 && array[row][col].top == 0 && array[row-1][col].dist != -1) dist_N = array[row-1][col].dist;
    if (row < MAZE_HEIGHT - 1 && array[row][col].bot == 0 && array[row+1][col].dist != -1) dist_S = array[row+1][col].dist;
    if (col < MAZE_WIDTH - 1 && array[row][col].right == 0 && array[row][col+1].dist != -1) dist_E = array[row][col+1].dist;
    if (col > 0 && array[row][col].left == 0 && array[row][col-1].dist != -1) dist_W = array[row][col-1].dist;

    if (dist_N != -1 && dist_N < low) low = dist_N;
    if (dist_S != -1 && dist_S < low) low = dist_S;
    if (dist_E != -1 && dist_E < low) low = dist_E;
    if (dist_W != -1 && dist_W < low) low = dist_W;

    if (low == currentDist || low == -1) {
        calcManToCenter(array);
        return IDLE;
    } else {
        // Apply turn priority logic
        if (dir == 'n') {
            if (dist_N == low) { row--; y++; return FORWARD; }
            if (g_turn_priority_left_first) { // Prioritize Left (West)
                if (dist_W == low) { dir = 'w'; return LEFT; }
                if (dist_E == low) { dir = 'e'; return RIGHT; }
            } else { // Prioritize Right (East)
                if (dist_E == low) { dir = 'e'; return RIGHT; }
                if (dist_W == low) { dir = 'w'; return LEFT; }
            }
            if (dist_S == low) { while(!PIDdone()){} turn(2); dir = 's'; return IDLE; } // Turn around
        } else if (dir == 's') {
            if (dist_S == low) { row++; y--; return FORWARD; }
            if (g_turn_priority_left_first) { // Prioritize Left (East)
                if (dist_E == low) { dir = 'e'; return LEFT; }
                if (dist_W == low) { dir = 'w'; return RIGHT; }
            } else { // Prioritize Right (West)
                if (dist_W == low) { dir = 'w'; return RIGHT; }
                if (dist_E == low) { dir = 'e'; return LEFT; }
            }
            if (dist_N == low) { while(!PIDdone()){} turn(2); dir = 'n'; return IDLE; }
        } else if (dir == 'e') {
            if (dist_E == low) { col++; x++; return FORWARD; }
            if (g_turn_priority_left_first) { // Prioritize Left (North)
                if (dist_N == low) { dir = 'n'; return LEFT; }
                if (dist_S == low) { dir = 's'; return RIGHT; }
            } else { // Prioritize Right (South)
                if (dist_S == low) { dir = 's'; return RIGHT; }
                if (dist_N == low) { dir = 'n'; return LEFT; }
            }
            if (dist_W == low) { while(!PIDdone()){} turn(2); dir = 'w'; return IDLE; }
        } else if (dir == 'w') {
            if (dist_W == low) { col--; x--; return FORWARD; }
            if (g_turn_priority_left_first) { // Prioritize Left (South)
                if (dist_S == low) { dir = 's'; return LEFT; }
                if (dist_N == low) { dir = 'n'; return RIGHT; }
            } else { // Prioritize Right (North)
                if (dist_N == low) { dir = 'n'; return RIGHT; }
                if (dist_S == low) { dir = 's'; return LEFT; }
            }
            if (dist_E == low) { while(!PIDdone()){} turn(2); dir = 'e'; return IDLE; }
        }
    }
    calcManToCenter(array); // Fallback recalculate
    return IDLE;
}

Action solver() {
    return floodFill();
}
