#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncursesw/ncurses.h>
#include <time.h>
#include <windows.h>

//Structure definitions
typedef struct snake_node{
	int x_coord;
	int y_coord;
	struct snake_node *next;
	struct snake_node *previous;

}snake_node;

typedef struct food{
	
	int x_coord;
	int y_coord;
}food;

//Global variables

//'1' will be used to represent a part of the snake, '\0' will be used to represent an empty pos, and '2' will be used to represent food
char positions[50][50] = {{'\0'}};

//0 will be left, 1 will be down, 2 will be right, 3 will be up
//initially the snake will be on the top left corner so I made the default direction right
short move_direction = 3;

snake_node *head = NULL;
snake_node *tail = NULL;
int length = 2;

snake_node list_of_nodes[2498];

int gamestate=0;

//Function definitions
int set_x_coord(snake_node *node, int new_x_coord);
int set_y_coord(snake_node *node, int new_y_coord);
int change_pos(snake_node *node, int new_x, int new_y);
int move_snake(snake_node *node);
void add_node(snake_node *node);
int change_move_direction(int new_direction);
void display_state(void);
void react_to_user_key(void);
void generate_food(food *cur);

//only for timing
//got this from https://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
void usleeps(__int64 usec)
{
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}

void freeMem(void){
	snake_node *node = head;
	snake_node *next;
	while(node != NULL){
		next = node -> next;
		free(node);
		node = next;
	}
	head = NULL;
	tail = NULL;
}


//ncurses variables:

int main(void){
	//ncurses stuff:
	WINDOW *game_screen = initscr();

	wrefresh(game_screen);
	keypad(game_screen, true);
	nocbreak();
	noecho();
	nodelay(game_screen, true);
	

	//snake initialization
	head = malloc( sizeof(snake_node));
	tail = malloc( sizeof(snake_node));
	if(head == NULL || tail == NULL) return -1;
	
	//random generation stuff for food
	time_t t;
	srand((unsigned) time(&t));

	//starting pos
	head -> x_coord = 4;
	head -> y_coord = 4;
	tail -> x_coord = 4;
	tail -> y_coord = 5;
	
	positions[4][4] = '1';
	positions[4][5] = '1';

	head -> next = NULL;
	head -> previous = tail;

	tail -> next = head;
	tail -> previous = NULL;
	
	sleep(3);
	while(gamestate!=-1){
		
		display_state();	
		
		char c = wgetch(game_screen);
		flushinp();
		
		//pause the program
		usleeps(350000);

		food *cur;
                generate_food(cur);
                positions[cur -> x_coord][cur -> y_coord] = '2';

		switch(c){
			case('W'):
			case('w'):
				move_direction = change_move_direction(3);
				break;
			case('D'):
			case('d'):
				move_direction = change_move_direction(2);
				break;
			case('S'):
			case('s'):
				move_direction = change_move_direction(1);
				break;
			case('A'):
			case('a'):
				move_direction = change_move_direction(0);
				break;
		}
		
		move_snake(&(list_of_nodes[length-2]));
		
	}

	freeMem();
	endwin();
}



//functions and logic

//How I think I'm going to implement losing: basically I think I'm gonna have a move_snake() function
//that moves each node of the snake to the position of the next node starting from the tail until it reaches the head
//once it reaches the head I will make the head move in the direction given by the user after it's called I'll call a function
//that compares the snake's nodes positions. I feel like there's probably an easier way of doing this that I just didn't think of yet

int set_x_coord(snake_node *node, int new_x_coord){
	//out of bounds
	if( (new_x_coord < 0) || (new_x_coord > 49) ) return -1;
	node -> x_coord = new_x_coord;
	
	return 0;
}

int set_y_coord(snake_node *node, int new_y_coord){
	//out of bounds
	if( (new_y_coord < 0) || (new_y_coord > 49) ) return -1;
	node -> y_coord = new_y_coord;

	return 0;
}

int change_pos(snake_node *node, int new_x, int new_y){
	
	short return_val = 0;

	//it is possible for the head to crash into the tail but then the tail moves away
	if(node == head && new_x == tail -> x_coord && new_y == tail -> y_coord){
		return_val = 1;
	}
	else if(positions[new_x][new_y] == '1'){
		return -1;
	}

	int old_x = node -> x_coord;
	int old_y = node -> y_coord;

	if( ((set_x_coord(node, new_x) == 0) && (set_y_coord(node, new_y) == 0)) ){
		
		if(node == tail && gamestate == 1){
			positions[new_x][new_y] = '1';
		}
		else{
			positions[old_x][old_y] = '\0';
			positions[new_x][new_y] = '1';
		}
		return return_val;
	}
	return -1;
}

int move_snake(snake_node *node){
	int next_x = head -> x_coord;
	int next_y = head -> y_coord;
	switch(move_direction){
		//left
		case(0):
			if(positions[head -> x_coord][(head -> y_coord) -1] == '2') add_node(node);
			
			gamestate = change_pos(head, head -> x_coord, (head -> y_coord) -1);
			break;
		//down
		case(1):
			if(positions[(head -> x_coord) +1][head -> y_coord] == '2') add_node(node);

			gamestate = change_pos(head, (head -> x_coord) +1, head -> y_coord);
			break;
		//right
		case(2):
			if(positions[head -> x_coord][(head -> y_coord) +1] == '2') add_node(node);
			
			gamestate = change_pos(head, head -> x_coord, (head -> y_coord) +1);
			break;
		//up
		case(3):
			if(positions[(head -> x_coord) -1][head -> y_coord] == '2') add_node(node);
			
			gamestate = change_pos(head, (head -> x_coord) -1, head -> y_coord);
			break;
	}
	
	if(gamestate == -1) return gamestate;

	int head_tail_collision = gamestate;

	snake_node *cur = head -> previous;
	while(cur != NULL){
		int old_x = cur -> x_coord;
		int old_y = cur -> y_coord;

		gamestate = change_pos(cur, next_x, next_y);
		if(gamestate == -1) return gamestate;
		
		gamestate = head_tail_collision;

		next_x = old_x;
		next_y = old_y;

		cur = cur -> previous;
	}
	
	if(tail -> x_coord == head -> x_coord && tail -> y_coord == head -> y_coord){
		gamestate = -1;
		return gamestate;
	}
	return 0;
}

// USE WHEN FOOD IS ADDED
void add_node(snake_node *node){
	
	int x_difference = (tail -> x_coord) - ((tail -> next) -> x_coord);
	int y_difference = (tail -> y_coord) - ((tail -> next) -> y_coord);

	node -> next = tail -> next;
	node -> previous = tail;
	
	(tail -> next) -> previous = node;

	tail -> next = node;

	node -> x_coord = tail -> x_coord;
	node -> y_coord = tail -> y_coord;
	
	tail -> x_coord = (node -> x_coord) + x_difference;
	tail -> y_coord = (node -> y_coord) + y_difference;

	length++;
}


int change_move_direction(int new_direction){
	//only 90 degree rotations (can't go left when the snake is moving right)
	if(move_direction - new_direction == -2 || move_direction - new_direction == 2) return move_direction;
	
	return new_direction;
}

void display_state(void){
	for(int x = 0; x < 50; x++){
		for(int y = 0; y < 50; y++){
			if(positions[x][y] == '\0'){
				mvprintw(x, y, "0");
			}
			else if(positions[x][y] == '1'){
				mvprintw(x, y, "1");
			}
			else{
				mvprintw(x, y, "2");
			}
		}
	}
}

void generate_food(food *tmp){
	//get random number from 0 and 49
	int rand_x = (int) (rand() % 50);
	int rand_y = (int) (rand() % 50);
	
	tmp -> x_coord = rand_x;
	tmp -> y_coord = rand_y;

	while(positions[tmp -> x_coord][tmp -> y_coord] != '\0'){
		rand_x = (int) (rand() % 50);
		rand_y = (int) (rand() % 50);
		tmp -> x_coord = rand_x;
		tmp -> y_coord = rand_y;
	}

}

