#include "solver.h"
#include <stdio.h>
#include <stdlib.h>


#include "controller.h"
#include "pid.h"
#include "irs.h"

#define MAX_SIZE 512



struct pair {
    short r;
    short c;
};

typedef struct {
    struct pair items[MAX_SIZE];
    int front;
    int rear;
} Queue;

void initializeQueue(Queue *q) {
    q->front = -1;
    q->rear = -1;
}

int isFull(Queue *q) {
    return (q->rear == MAX_SIZE - 1);
}

int isEmpty(Queue *q) {
    return (q->front == -1);
}

void enqueue(Queue *q, struct pair value) {
    if (isFull(q)) {
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
        printf("Queue is empty!\n");
        struct pair l = {-1, -1};
        return l;
    } else {
        struct pair item = q->items[q->front];
        q->front++;
        if (q->front > q->rear) {
            q->front = q->rear = -1;
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

int row = 15, col = 0, x = 0, y = 0;
char dir = 'n';
struct node array[16][16];

int first = 1;


void calcMan(struct node array[16][16]) {
    Queue q;
    initializeQueue(&q);
    struct pair p;

    // Set all distances to -1
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array[i][j].dist = -1;
        }
    }

    // Center goals
    for (int i = 7; i <= 8; i++) {
        for (int j = 7; j <= 8; j++) {
            array[i][j].dist = 0;
            p.r = i;
            p.c = j;
            enqueue(&q, p);
        }
    }

    while (!isEmpty(&q)) {
        struct pair current = dequeue(&q);
        int row = current.r;
        int col = current.c;
        int dist = array[row][col].dist;

        // WEST
        if (col > 0 && array[row][col].left == 0 && array[row][col - 1].dist == -1) {
            array[row][col - 1].dist = dist + 1;
            p.r = row;
            p.c = col - 1;
            enqueue(&q, p);

        }

        // EAST
        if (col < 15 && array[row][col].right == 0 && array[row][col + 1].dist == -1) {
            array[row][col + 1].dist = dist + 1;
            p.r = row;
            p.c = col + 1;
            enqueue(&q, p);

        }

        // NORTH
        if (row > 0 && array[row][col].top == 0 && array[row - 1][col].dist == -1) {
            array[row - 1][col].dist = dist + 1;
            p.r = row - 1;
            p.c = col;
            enqueue(&q, p);
           \
        }

        // SOUTH
        if (row < 15 && array[row][col].bot == 0 && array[row + 1][col].dist == -1) {
            array[row + 1][col].dist = dist + 1;
            p.r = row + 1;
            p.c = col;
            enqueue(&q, p);

        }
    }

    /*
    for(int i = 0; i < 16; i++){//x
        for(int j = 0; j < 16; j++){//y
            char str[10];
            sprintf(str, "%d", array[j][i].dist);
            //API_setText( i , 15-j, str);

        }
    }
    */

}

void calcManBack(struct node array[16][16]) {
    Queue q;
    initializeQueue(&q);
    struct pair p;

    // Set all distances to -1
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array[i][j].dist = -1;
        }
    }

    // Center goals

            array[15][0].dist = 0;
            p.r = 15;
            p.c = 0;
            enqueue(&q, p);


    while (!isEmpty(&q)) {
        struct pair current = dequeue(&q);
        int row = current.r;
        int col = current.c;
        int dist = array[row][col].dist;

        // WEST
        if (col > 0 && array[row][col].left == 0 && array[row][col - 1].dist == -1) {
            array[row][col - 1].dist = dist + 1;
            p.r = row;
            p.c = col - 1;
            enqueue(&q, p);

        }

        // EAST
        if (col < 15 && array[row][col].right == 0 && array[row][col + 1].dist == -1) {
            array[row][col + 1].dist = dist + 1;
            p.r = row;
            p.c = col + 1;
            enqueue(&q, p);

        }

        // NORTH
        if (row > 0 && array[row][col].top == 0 && array[row - 1][col].dist == -1) {
            array[row - 1][col].dist = dist + 1;
            p.r = row - 1;
            p.c = col;
            enqueue(&q, p);
           \
        }

        // SOUTH
        if (row < 15 && array[row][col].bot == 0 && array[row + 1][col].dist == -1) {
            array[row + 1][col].dist = dist + 1;
            p.r = row + 1;
            p.c = col;
            enqueue(&q, p);

        }
    }
/*
    for(int i = 0; i < 16; i++){//x
        for(int j = 0; j < 16; j++){//y
            char str[10];
            sprintf(str, "%d", array[j][i].dist);
            //API_setText( i , 15-j, str);

        }
    }
    */

}





void markWall(struct node array[16][16], int row, int col, int x, int y, char dir) {

	  int Left = readLeftIR();
	  int frontRight = readFrontRightIR();
	  int frontLeft = readFrontLeftIR();
	  int Right = readRightIR();

	  //EDIT THIS IN ORDER TO CHANGE THE VALUE AT WHICH AN IR READS A WALL
	  int wallLeft = 1;
	  int wallRight = 1;
	  int wallFront = 1;

	  if(Left > 1000)
		  wallLeft = 0;
	  if(Right > 1000)
		  wallRight = 0;
	  if(frontRight > 1000 && frontLeft > 1000)
		  wallFront = 0;


    if (dir == 'n') {
        if (wallFront) {
            array[row][col].top = 1;
            if (row - 1 >= 0)
                array[row - 1][col].bot = 1;
            //API_setWall(x, y, 'n');
        }
        if (wallLeft) {
            array[row][col].left = 1;
            if (col - 1 >= 0)
                array[row][col - 1].right = 1;
          //  API_setWall(x, y, 'w');
        }
        if (wallRight) {
            array[row][col].right = 1;
            if (col + 1 <= 15)
                array[row][col + 1].left = 1;
          //  API_setWall(x, y, 'e');
        }
    }

    else if (dir == 's') {
        if (wallFront) {
            array[row][col].bot = 1;
            if (row + 1 <= 15)
                array[row + 1][col].top = 1;
           // API_setWall(x, y, 's');
        }
        if (wallLeft) {
            array[row][col].right = 1;
            if (col + 1 <= 15)
                array[row][col + 1].left = 1;
           // API_setWall(x, y, 'e');
        }
        if (wallRight) {
            array[row][col].left = 1;
            if (col - 1 >= 0)
                array[row][col - 1].right = 1;
            //API_setWall(x, y, 'w');
        }
    }

    else if (dir == 'e') {
        if (wallFront) {
            array[row][col].right = 1;
            if (col + 1 <= 15)
                array[row][col + 1].left = 1;
           // API_setWall(x, y, 'e');
        }
        if (wallLeft) {
            array[row][col].top = 1;
            if (row - 1 >= 0)
                array[row - 1][col].bot = 1;
          //  API_setWall(x, y, 'n');
        }
        if (wallRight) {
            array[row][col].bot = 1;
            if (row + 1 <= 15)
                array[row + 1][col].top = 1;
          //  API_setWall(x, y, 's');
        }
    }

    else if (dir == 'w') {
        if (wallFront) {
            array[row][col].left = 1;
            if (col - 1 >= 0)
                array[row][col - 1].right = 1;
           // API_setWall(x, y, 'w');
        }
        if (wallLeft) {
            array[row][col].bot = 1;
            if (row + 1 <= 15)
                array[row + 1][col].top = 1;
         //   API_setWall(x, y, 's');
        }
        if (wallRight) {
            array[row][col].top = 1;
            if (row - 1 >= 0)
                array[row - 1][col].bot = 1;
          //  API_setWall(x, y, 'n');
        }
    }
}


/*

    this is 0,0 in array: row, col
        O O O O O O O O O
        O O O O O O O O O
        O O O O O O O O O
        O O O O O O O O O
        O O O O O O O O O
    this is 0,0 in API: x, y
*/

int returning = 0;
int firstRunback = 1;
int justSwitched = 0;

Action back(){

    if(justSwitched == 1){
    calcManBack(array);
    justSwitched = 0;
    }

    if(array[row][col].dist == 0){
        returning = 0;
        justSwitched = 1;
        return IDLE;
    }




    markWall(array , row, col , x , y, dir);


    int low = array[row][col].dist;

    // Find the lowest neighbor distance (only considering unblocked directions)
    switch(dir){
        case 'n':
            if (array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist < low)
                low = array[row][col-1].dist;
            if (array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist < low)
                low = array[row-1][col].dist;
            if (array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist < low)
                low = array[row][col+1].dist;
            if (array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist < low) {
                low = array[row+1][col].dist;
                while(!PIDdone()){

                }
                turn(2);
                dir = 's';
            }
            break;

        case 's':
            if (array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist < low)
                low = array[row][col-1].dist;
            if (array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist < low)
                low = array[row+1][col].dist;
            if (array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist < low)
                low = array[row][col+1].dist;
            if (array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist < low) {
                low = array[row-1][col].dist;
                while(!PIDdone()){

                }
                turn(2);
                dir = 'n';
            }
            break;

        case 'e':
            if (array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist < low)
                low = array[row+1][col].dist;
            if (array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist < low)
                low = array[row][col+1].dist;
            if (array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist < low)
                low = array[row-1][col].dist;
            if (array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist < low) {
                low = array[row][col-1].dist;
                while(!PIDdone()){

                }
                turn(2);
                dir = 'w';
            }
            break;

        case 'w':
            if (array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist < low)
                low = array[row+1][col].dist;
            if (array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist < low)
                low = array[row][col-1].dist;
            if (array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist < low)
                low = array[row-1][col].dist;
            if (array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist < low) {
                low = array[row][col+1].dist;
                while(!PIDdone()){

                                }
                turn(2);
                dir = 'e';
            }
            break;
    }


    if (low == array[row][col].dist) {
        calcManBack(array);
        return IDLE;
    } else {
        // Make the actual movement (now with wall checks)
        switch(dir){
            case 'n':
                if(array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist == low){
                    row--;
                    y++;
                    return FORWARD;
                }
                if(array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist == low){
                    dir = 'e';
                    return RIGHT;
                }
                if(array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist == low){
                    dir = 'w';
                    return LEFT;
                }
                break;

            case 's':
                if(array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist == low){
                    row++;
                    y--;
                    return FORWARD;
                }
                if(array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist == low){
                    dir = 'w';
                    return RIGHT;
                }
                if(array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist == low){
                    dir = 'e';
                    return LEFT;
                }
                break;

            case 'e':
                if(array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist == low){
                    col++;
                    x++;
                    return FORWARD;
                }
                if(array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist == low){
                    dir = 'n';
                    return LEFT;
                }
                if(array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist == low){
                    dir = 's';
                    return RIGHT;
                }
                break;

            case 'w':
                if(array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist == low){
                    col--;
                    x--;
                    return FORWARD;
                }
                if(array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist == low){
                    dir = 's';
                    return LEFT;
                }
                if(array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist == low){
                    dir = 'n';
                    return RIGHT;
                }
                break;
        }
    }

    return IDLE;

}

Action floodFill() {



    if(first == 1){
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                initialize(&array[i][j], 0, 0, 0, 0, i, j, -1);
            }
        }
        calcMan(array);
        first = 0;
    }

    markWall(array , row, col , x , y, dir);

    if(justSwitched == 1){
        calcMan(array);
        justSwitched = 0;
        }

    if(array[row][col].dist==0|| returning == 1){
        returning = 1;
        justSwitched = 1;
        return back();
    }

    int low = array[row][col].dist;

    // Find the lowest neighbor distance (only considering unblocked directions)
    switch(dir){
        case 'n':
            if (array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist < low)
                low = array[row][col-1].dist;
            if (array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist < low)
                low = array[row-1][col].dist;
            if (array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist < low)
                low = array[row][col+1].dist;
            if (array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist < low) {
                low = array[row+1][col].dist;
                while(!PIDdone()){

                                }
                turn(2);
                dir = 's';
            }
            break;

        case 's':
            if (array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist < low)
                low = array[row][col-1].dist;
            if (array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist < low)
                low = array[row+1][col].dist;
            if (array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist < low)
                low = array[row][col+1].dist;
            if (array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist < low) {
                low = array[row-1][col].dist;
                while(!PIDdone()){

                                }
                turn(2);
                dir = 'n';
            }
            break;

        case 'e':
            if (array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist < low)
                low = array[row+1][col].dist;
            if (array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist < low)
                low = array[row][col+1].dist;
            if (array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist < low)
                low = array[row-1][col].dist;
            if (array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist < low) {
                low = array[row][col-1].dist;
                while(!PIDdone()){

                                }
                turn(2);
                dir = 'w';
            }
            break;

        case 'w':
            if (array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist < low)
                low = array[row+1][col].dist;
            if (array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist < low)
                low = array[row][col-1].dist;
            if (array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist < low)
                low = array[row-1][col].dist;
            if (array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist < low) {
                low = array[row][col+1].dist;
                while(!PIDdone()){

                                }
                turn(2);
                dir = 'e';
            }
            break;
    }


    if (low == array[row][col].dist) {
        calcMan(array);
        return IDLE;
    } else {
        // Make the actual movement (now with wall checks) TO CHANGE PRIORTIES CHANGE IT HERE
        switch(dir){
            case 'n':

              if(array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist == low){
                row--;
                y++;
                return FORWARD;
              }
                if(array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist == low){
                    dir = 'e';
                    return RIGHT;
                }
                if(array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist == low){
                    dir = 'w';
                    return LEFT;
                }

                break;

            case 's':
                if(array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist == low){
                    row++;
                    y--;
                    return FORWARD;
                }
                if(array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist == low){
                    dir = 'w';
                    return RIGHT;
                }
                if(array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist == low){
                    dir = 'e';
                    return LEFT;
                }
                break;

            case 'e':
                if(array[row][col].right == 0 && col+1 < 16 && array[row][col+1].dist == low){
                    col++;
                    x++;
                    return FORWARD;
                }
                if(array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist == low){
                    dir = 'n';
                    return LEFT;
                }
                if(array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist == low){
                    dir = 's';
                    return RIGHT;
                }
                break;

            case 'w':
                if(array[row][col].left == 0 && col-1 >= 0 && array[row][col-1].dist == low){
                    col--;
                    x--;
                    return FORWARD;
                }
                if(array[row][col].bot == 0 && row+1 < 16 && array[row+1][col].dist == low){
                    dir = 's';
                    return LEFT;
                }
                if(array[row][col].top == 0 && row-1 >= 0 && array[row-1][col].dist == low){
                    dir = 'n';
                    return RIGHT;
                }
                break;
        }
    }

    return IDLE;
}


Action solver() {
    return floodFill();
}

//NEXT TO WORK ON, IT GETS STUCK IN DEAD ENDS, I NEED TO FIGURE OUT A WAY TO TELL IT TO TURN AROUND AND ALSO JUST THINK ABOUT GENERAL LOGIC, DO I WANT TURNS AKA
