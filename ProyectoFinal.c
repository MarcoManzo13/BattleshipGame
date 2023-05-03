#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>

#define NUMBER_ROUNDS 100 // Maximum amount of rounds for a 10x10 matrix
#define ALARM_TIME 1200 // 20 minutes

int board1[10][10];
int board2[10][10];
int ships_number;
int ships_number1 = 0;
int ships_number2 = 0;
int cols = 10;
int rows = 10;
char* ship_figure = " 🛥️ ";
char* waves_figure = "  ≋ ";
char* ship_sunk = " 💥 ";
char player1_name[20] = "Player 1";
char player2_name[20] = "Player 2";
int buffer = 0;

pthread_mutex_t mutex;
pthread_cond_t consumer_condition, producer_condition;

// Alarm to signal that the game has ended.
void alarm_handler(int sig)
{
    int minutes = ALARM_TIME / 60;
    printf("\nTime's up! %d minutes have passed so the game has ended.\n", minutes);
    exit(0);
}

// Function that uses processes to clear the terminal
void clear_terminal() {
    for (int i = 10; i >= 1; i--) 
    {
    		printf("\n");
        printf("Clearing terminal for next player turn in %d seconds...\r", i);
        fflush(stdout);
        sleep(1);
    }
    printf("\n\nClearing terminal now, please give the device to the other player...\n");
    sleep(5);

    pid_t pid = fork();
    
    if (pid == -1) 
    {
        perror("fork");
        return;
    } else if (pid == 0) 
    {
        execlp("clear", "clear", NULL);
        perror("execlp");
        return;
    } else 
    {
        waitpid(pid, NULL, 0);
    }
}

// Print Board 1 for Player 1
void print_board_player1() 
{
  printf("%s's Board\n", player1_name);
  for (int i = -1; i < rows; i++) 
  {
      printf("%3d  ", i+1);
  }
  printf("\n");
  for (int i = 0; i < cols; i++) 
  {
      printf("%3d ", i+1);
      for (int j = 0; j < cols; j++) 
      {
          if (board1[i][j] == 0) 
          {
              printf("%3s ", waves_figure);
          }
          else if (board1[i][j] == 1)
          {
          	  printf(" %3s ", ship_figure);
          }
          else if (board1[i][j] == 2)
          {
          	  printf("%3s ", ship_sunk);
          }
          else if (board1[i][j] == 3)
          {
              printf("%3s ", ship_sunk);
          }
          else 
          {
              printf("%3d ", board1[i][j]);
          }
      }
      printf("\n");
  }
}

// Print Board 2 for Player 2
void print_board_player2()
{
  printf("%s's Board\n", player2_name);
  for (int i = -1; i < rows; i++) 
  {
      printf("%3d  ", i+1);
  }
  printf("\n");
  for (int i = 0; i < cols; i++) 
  {
      printf("%3d ", i+1);
      for (int j = 0; j < cols; j++) 
      {
          if (board2[i][j] == 0) 
          {
              printf("%3s ", waves_figure);
          }
          else if (board2[i][j] == 1)
          {
          	  printf(" %3s ", ship_figure);
          }
          else if (board2[i][j] == 2)
          {
          	  printf(" %3s ", ship_sunk);
          }
          else if (board2[i][j] == 3) 
          {
              printf("%3s ", ship_sunk);
          }
          else 
          {
              printf("%3d ", board2[i][j]);
          }
      }
      printf("\n");
  }
}

// Function that checks for incorrect ship placement for Player 1.
int is_valid_input_player1(int x, int y) {
    if (x < 1 || x > rows || y < 1 || y > cols) {
        printf("Invalid input: position is out of bounds.\n");
        return 0;
    }
    if (board1[x-1][y-1] == 1) {
        printf("Invalid input: position is already occupied by a ship.\n");
        return 0;
    }
    if (getchar() != '\n') {
        printf("Invalid input: too many numbers.\n");
        return 0;
    }
    return 1;
}

// Function that checks for incorrect ship placement for Player 2.
int is_valid_input_player2(int x, int y) {
    if (x < 1 || x > rows || y < 1 || y > cols) {
        printf("Invalid input: position is out of bounds.\n");
        return 0;
    }
    if (board2[x-1][y-1] == 1) {
        printf("Invalid input: position is already occupied by a ship.\n");
        return 0;
    }
    if (getchar() != '\n') {
        printf("Invalid input: too many numbers.\n");
        return 0;
    }
    return 1;
}

// Function that returns true when the player has entered a correct attack position.
bool is_valid_input_attack(int x, int y) {
    if (x < 1 || x > rows || y < 1 || y > cols) {
        printf("Invalid input: position is out of bounds.\n");
        return false;
    }
    else if (getchar() != '\n') 
    {
        printf("Invalid input: too many numbers.\n");
        return false;
    } else 
    {
	    return true;
    }
}

// Enter the position of the ships for Player 1
void ships_player_1()
{
  int x, y;
  printf("\n%s's turn to select ships position.\n", player1_name);
  for (int i = 0; i < ships_number1; i++) 
  {
    printf("\nEnter the position of ship #%d: ", i+1);
    while (scanf("%d %d", &x, &y) != 2 || !is_valid_input_player1(x, y)) {
        printf("Please enter a valid input.\n");
        while (getchar() != '\n'); // clear input buffer
    }
    board1[x-1][y-1] = 1;
  }
}

// Enter the position of the ships for Player 2
void ships_player_2()
{
  int x, y;
  printf("\n%s's turn to select ships position.\n", player2_name);
  for (int i = 0; i < ships_number2; i++) 
  {
    printf("\nEnter the position of ship #%d: ", i+1);
    while (scanf("%d %d", &x, &y) != 2 || !is_valid_input_player2(x, y)) {
        printf("Please enter a valid input.\n");
        while (getchar() != '\n'); // clear input buffer
    }
    board2[x-1][y-1] = 1;
  }
}

// Attack from Player 1
void attack_player1()
{
	int x, y;
	bool valid_input = false;

	printf("\n%s's turn.\n", player1_name);
	while (!valid_input) 
	{
		printf("\nEnter the position to attack: ");
		scanf("%d %d", &x, &y);
		valid_input = is_valid_input_attack(x, y);
	}
  
  if (board2[x-1][y-1] == 1)
  {
    board2[x-1][y-1] = 3;
    printf("\nAttacking in");
    printf("\n    3...");
    sleep(1);
    printf("\n    2...");
    sleep(1);
    printf("\n    1...");
    sleep(1);
    printf("\n  FIRE!!!\n");
    sleep(1);
    ships_number2--;
    printf("\n%s's attack has sunk the boat.\n", player1_name);
    printf("\nThere are %d ships left for %s.\n", ships_number2, player2_name);
  }
  else if (board2[x-1][y-1] == 0)
  {
    board2[x-1][y-1] = 2;
    printf("\nAttacking in");
    printf("\n    3...");
    sleep(1);
    printf("\n    2...");
    sleep(1);
    printf("\n    1...");
    sleep(1);
    printf("\n  FIRE!!!\n");
    sleep(1);
    printf("\n%s's attack failed.\n", player1_name);
    printf("\nThere are %d ships left for %s.\n", ships_number2, player2_name);
  }
  else
  {
  	board2[x-1][y-1] = 3;
    printf("\nAttacking in");
    printf("\n    3...");
    sleep(1);
    printf("\n  Wait...");
    sleep(1);
    printf("\n    2...");
    printf("\nSomething is wrong.");
    sleep(1);
    printf("\n    1...");
    sleep(1);
    printf("\n  FIRE!!!\n");
    sleep(1);
    printf("\n%s has attacked the same place twice, which is weird. Nothing happened, just a wasted bullet.\n", player1_name);
    printf("\nThere are %d ships left for %s.\n", ships_number2, player2_name);
  }
    // Counter to know when Player 2 looses
  if(ships_number2 == 0 )
  {
    pthread_kill( pthread_self(), SIGUSR1 );
  }
}

// Attack from Player 2
void attack_player2()
{
  int x, y;
  bool valid_input = false;

	printf("\n%s's turn.\n", player2_name);
	while (!valid_input) 
	{
		printf("\nEnter the position to attack: ");
		scanf("%d %d", &x, &y);
		valid_input = is_valid_input_attack(x, y);
	}
  
  if (board1[x-1][y-1] == 1)
  {
    board1[x-1][y-1] = 3;
    printf("\nAttacking in");
    printf("\n    3...");
    sleep(1);
    printf("\n    2...");
    sleep(1);
    printf("\n    1...");
    sleep(1);
    printf("\n  FIRE!!!\n");
    sleep(1);
    ships_number1--;
    printf("%s's attack has sunk the boat.\n", player2_name);
    printf("\nThere are %d ships left for %s.\n", ships_number1, player1_name);
  }
  else if(board1[x-1][y-1] == 0)
  {
    board1[x-1][y-1] = 2;
    printf("\nAttacking in");
    printf("\n    3...");
    sleep(1);
    printf("\n    2...");
    sleep(1);
    printf("\n    1...");
    sleep(1);
    printf("\n  FIRE!!!\n");
    sleep(1);
    printf("\n%s's attack failed.\n", player2_name);
    printf("\nThere are %d ships left for %s.\n", ships_number1, player1_name);
  }
  else
  {
    board1[x-1][y-1] = 3;
    printf("\nAttacking in");
    printf("\n    3...");
    sleep(1);
    printf("\n  Wait...");
    sleep(1);
    printf("\n    2...");
    printf("\nSomething is wrong.");
    sleep(1);
    printf("\n    1...");
    sleep(1);
    printf("\n  FIRE!!!\n");
    sleep(1);
    printf("\n%s has attacked the same place twice, which is weird. Nothing happened, just a wasted bullet.\n", player2_name);
    printf("\nThere are %d ships left for %s.\n", ships_number1, player1_name);
  }
  // Counter to know when Player 1 looses
  if(ships_number1 == 0 )
  {
    pthread_kill( pthread_self(), SIGUSR2 );
  }
}

// Producer = Player 1
void* producer(void* arg)
{
  for ( int i = 0; i < NUMBER_ROUNDS; i++ ) 
  {
    pthread_mutex_lock( &mutex );
    while( buffer != 0 )
    {
      // Wait until the signal arrives
      pthread_cond_wait( &producer_condition, &mutex );
    }
    
    buffer = 1;
    print_board_player1();
    attack_player1();
    print_board_player1();
    clear_terminal();
    
   	// Elements in buffer in producer
    pthread_cond_signal( &consumer_condition );
    pthread_mutex_unlock( &mutex );
  }

  // Producer End Signal
  pthread_kill( pthread_self(), SIGUSR1 );

  pthread_exit( NULL );
}

// Consumer = Player 2
void* consumer(void* arg)
{
  for ( int i = 0; i < NUMBER_ROUNDS; i++ ) 
  {
    pthread_mutex_lock( &mutex );
    // Critical Region
    while (buffer == 0)
    {
      // Wait until the signal arrives
      pthread_cond_wait( &consumer_condition, &mutex );
    }
    
    print_board_player2();
    attack_player2();
    print_board_player2();
    clear_terminal();
  
    buffer = 0;
    
    // Elements in buffer in consumer
    pthread_cond_signal( &producer_condition ); 
    pthread_mutex_unlock( &mutex );
  }

  // Consumer End Signal
  pthread_kill( pthread_self(), SIGUSR2 );

  pthread_exit(NULL);
}

// Signals that appear when a player has won.
void signal_handler(int signum)
{
  if (signum == SIGUSR1) 
  {
    printf("\n%s has won, preparing the celebration...\n", player1_name);
    sleep(3);
    printf("\nCongratulations %s!!!, you have won the opportunity to rub it in %s face!\n", player1_name, player2_name);
    printf("\n\nCredits:\n");
    printf("Student: Marco Antonio Manzo Ruiz\n");
    printf("ID: 173127\n");
    printf("Course: Operating Systems\n");
    printf("Date: 02/05/2023\n");
    exit(0);
  }
  else if (signum == SIGUSR2)
  {
    printf("\n%s has won, preparing the celebration...\n", player2_name);
    sleep(3);
    printf("\nCongratulations %s!!!, you have won the opportunity to rub it in %s face!\n", player2_name, player1_name);
    printf("\n\nCredits:\n");
    printf("Student: Marco Antonio Manzo Ruiz\n");
    printf("ID: 173127\n");
    printf("Course: Operating Systems\n");
    printf("Date: 02/05/2023\n");
    exit(0);
  }
}

int main()
{
  // Create pthread threads
  pthread_t thread_1, thread_2;
  
  // Transform the seconds to minutes to end the game and put the turns into a variable.
  int minutes = ALARM_TIME / 60;
  int turns = NUMBER_ROUNDS;

  // Welcoming and instructions
  printf("\nWelcome to battleship, a game developed in C using threads, processes, and much more!\n");
  printf("\nInstructions\n");
  printf("\nInstruction #1: Both players choose the positions of their ships. The ships are of size 1.\n");
  printf("\nInstruction #2: To select a position to put the ship, the player needs to enter its coordinates by its numbers, separated by a space.\n");
  printf("\nInstruction #3: To select a position to attack a ship, the player needs to enter its coordinates by its numbers, separated by a space.\n");
  printf("\nInstruction #4: The players will have %d minutes to end the game, if that time passes, the game will end automatically.\n", minutes);
  printf("\nInstruction #5: The players will have %d turns to play each, if those turns finish, the game will end automatically.\n", turns);
  printf("\nInstruction #6: After each player has attacked and viewed the results for 10 seconds, there will be a 5-second window to hand over the device to the other player.\n");
  printf("\n%s = Alive Ship\n", ship_figure);
  printf("\n%s = Dead Ship\n", ship_sunk);
  printf("\n%s = Water\n", waves_figure);

  // Setup for the game
  printf("\nWith how many ships per player would you like to play?: ");
  scanf("%d", &ships_number);
  printf("\nEnter the name of Player 1: ");
  scanf("%s", player1_name);
  printf("\nEnter the name of Player 2: ");
  scanf("%s", player2_name);
  
  // Put a timer for the game to end
  alarm(ALARM_TIME);
  signal(SIGALRM, alarm_handler);
  
  // Set up the number of ships to be played with
  ships_number1 = ships_number;
  ships_number2 = ships_number;
  
  // Begin Game
  print_board_player1();
  ships_player_1();
  system("clear"); // Clear terminal with system call after player has positioned their ships
  print_board_player2();
  ships_player_2();
  system("clear"); // Clear terminal with system call after player has positioned their ships
  
  // Signal Handlers
  signal(SIGUSR2, signal_handler);
  signal(SIGUSR1, signal_handler);
  
  // Mutex
  pthread_mutex_init( &mutex, 0 );
  pthread_cond_init( &consumer_condition, 0 );
  pthread_cond_init( &producer_condition, 0 );
  
  // Thread Creation of Producer and Consumer
  pthread_create( &thread_1, NULL, producer, NULL );
  pthread_create( &thread_2, NULL, consumer, NULL );
  
  // Functions that wait for the end of thread_1 and thread_2
  pthread_join( thread_1, NULL);
  pthread_join( thread_2, NULL);

  // Destroy Mutex
  pthread_mutex_destroy( &mutex );
  pthread_cond_destroy( &consumer_condition );
  pthread_cond_destroy( &producer_condition );
  
  return 0;
}
