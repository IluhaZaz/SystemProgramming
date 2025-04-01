#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

volatile int guess = 0;
volatile int correct = 0;

int main(int argc, char *argv[]) {
  int N = atoi(argv[1]);
  srand(time(NULL));

  pid_t pid = fork();

  pid_t curr_pid = getpid();

  struct mq_attr attr;

  attr.mq_flags = 0;
  attr.mq_maxmsg = 1;
  attr.mq_msgsize = sizeof(int);
  attr.mq_curmsgs = 0;

  mqd_t guesses = mq_open("/guesses", O_CREAT | O_RDWR, 0666, &attr);
  mqd_t results = mq_open("/results", O_CREAT | O_RDWR, 0666, &attr);

  bool is_guesser = false;
  if (pid == 0) {
    is_guesser = true;
  }

  for (int i = 1; i < 11; ++i) {
    sleep(2);

    printf("Round #%d. Pid %d \n", i, curr_pid);

    correct = 0;

    if (is_guesser) {
      int attempt = 0;
      int current_guess;
	  
	  int is_ready = 0;
	  while(!is_ready){
		mq_receive(results, (char *)&is_ready, sizeof(int), NULL);
	  }

      while (!correct) {
        attempt++;
        current_guess = attempt;
		
		printf("Guesser: Attempt %d: Guessing %d. Pid %d \n", attempt,
               current_guess, curr_pid);
		
        mq_send(guesses, (char *)&current_guess, sizeof(current_guess), 0);

        mq_receive(results, (char *)&correct, sizeof(int), NULL);
      }

      printf("Guesser: Correct! The number is %d. Pid %d \n", current_guess,
             curr_pid);
    } else {
      int secret_number = 1 + rand() % N;
      printf("Secret Keeper: The secret number is %d. Pid %d \n", secret_number,
             curr_pid);
	  mq_send(results, (char *)&secret_number, sizeof(secret_number), 0);

      while (!correct) {
        mq_receive(guesses, (char *)&guess, sizeof(int), NULL);

        if (guess == secret_number) {
          printf("Secret Keeper: Guesser guessed correctly! Pid %d \n",
                 curr_pid);
          correct = 1;

        } else {
          printf("Secret Keeper: Incorrect guess (%d). Pid %d \n", guess,
                 curr_pid);
        }
        mq_send(results, (char *)&correct, sizeof(correct), 0);
      }
    }
    is_guesser = !is_guesser;
  }
  mq_close(results);
  mq_unlink("/results");

  mq_close(guesses);
  mq_unlink("/guesses");

  return 0;
}

