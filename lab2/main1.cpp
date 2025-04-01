#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SIG_GUESS SIGRTMIN
#define SIG_CORRECT SIGUSR1
#define SIG_INCORRECT SIGUSR2

volatile int guess = 0;
volatile int correct = 0;
pid_t guesser_pid;

void result_handler(int sig) { correct = (sig == SIG_CORRECT); }

int main(int argc, char *argv[]) {
  int N = atoi(argv[1]);
  srand(time(NULL));

  volatile pid_t pid = fork();

  pid_t curr_pid = getpid();

  bool is_guesser = false;
  if (pid == 0) {
    is_guesser = true;
    pid = getppid();
  }

  sigset_t guessor_mask;
  sigemptyset(&guessor_mask);
  sigaddset(&guessor_mask, SIG_CORRECT);
  sigaddset(&guessor_mask, SIG_INCORRECT);
  sigprocmask(SIG_BLOCK, &guessor_mask, nullptr);
  int feed_sig;

  sigset_t keeper_mask;
  sigemptyset(&keeper_mask);
  sigaddset(&keeper_mask, SIG_GUESS);
  sigprocmask(SIG_BLOCK, &keeper_mask, nullptr);
  siginfo_t value;

  for (int i = 1; i < 11; ++i) {
    printf("Round #%d. Pid %d \n", i, curr_pid);
    correct = 0;

    sleep(2);

    if (is_guesser) {
      int attempt = 0;
      int current_guess;

      while (!correct) {
        attempt++;
        current_guess = attempt;
        printf("Guesser: Attempt %d: Guessing %d. Pid %d \n", attempt,
               current_guess, curr_pid);

        union sigval sv;
        sv.sival_int = current_guess;
        sigqueue(pid, SIG_GUESS, sv);

        sigwait(&guessor_mask, &feed_sig);
        result_handler(feed_sig);
      }

      printf("Guesser: Correct! The number is %d. Pid %d \n", current_guess,
             curr_pid);
    } else {
      int secret_number = 1 + rand() % N;
      printf("Secret Keeper: The secret number is %d. Pid %d \n", secret_number,
             curr_pid);

      while (1) {
        int sig = sigwaitinfo(&keeper_mask, &value);
        int guess = value.si_value.sival_int;

        if (guess == secret_number) {
          printf("Secret Keeper: Guesser guessed correctly! Pid %d \n",
                 curr_pid);
          kill(pid, SIG_CORRECT);
          break;
        } else {
          printf("Secret Keeper: Incorrect guess (%d). Pid %d \n", guess,
                 curr_pid);
          kill(pid, SIG_INCORRECT);
        }
      }
    }
    is_guesser = !is_guesser;
  }

  return 0;
}

