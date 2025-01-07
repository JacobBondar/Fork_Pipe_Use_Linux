/*
 * Ex #2: A duel for the higher value.
 * ==========================================================
 * Written by: Jacob Bondar.
 *
 * This program simulates a competitive game between two child processes
 *  managed by a parent process using communication through pipes and
 *  signals. Each child generates random numbers, which are sent to the
 *  parent for comparison. The parent determines the winner of each
 *  round, updates the scores, and sends the results back to the
 *  children. The game continues until one child achieves 120 win or
 *  both of then achieve 100 same values. at which point the parent
 *  declares the winner and terminates both child processes.
 *
 * Input: An integer that represents the seed.
 *
 * Output: The winner (if there is one), and the children summary.
 */

 //-------------- include section ---------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

//-------------- const section -----------------------------------------
const int EQUAL = 0;
const int HIGHER = 1;
const int LOWER = -1;
const int WON = 120;
const int FINISED = 100;
const int DIVIDE = 10;

//-------------- prototypes section ------------------------------------

void create_pipes_and_run(int seed);
void do_dad(pid_t first_child, pid_t second_child,
    const int pipe_sons_dad[], const int pipe_dad_first_son[],
    const int pipe_dad_second_son[]);
void print_killed(int id, int minus_one_count, int zero_count,
    int one_count, const int pipe_sons_dad[], const int pipe_dad_son[]);
void update_counters_after_read(int num_got, int* zero_count,
    int* one_count, int* minus_one_count);
void print_won(pid_t first_child, pid_t second_child, int num_child);
void swap(int* first_num, int* second_num);
void update_values(int* won, int* first_result, int* second_result);
void check_fork(pid_t child);
void kill_children(int sig);
void do_child(const int pipe_dad_son[], const int pipe_son_dad[],
    int seed, int id);
bool finish = false;

//-------------- main --------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        puts("Not enough arguments!");
        exit(EXIT_FAILURE);
    }

    // Using sigaction for signal
    struct sigaction act;
    act.sa_handler = kill_children;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);

    int seed = atoi(argv[1]);

    create_pipes_and_run(seed);

    exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* The function implements the actions of a child process, including
 *  sending random values to the parent and responding to instructions.
 * The function receives: two pipes for communication, a random seed,
 *  and the child's ID.
 * The function returns: void.
 */
void do_child(const int pipe_dad_son[], const int pipe_sons_dad[],
    int seed, int id)
{
    close(pipe_sons_dad[0]); // can't read
    close(pipe_dad_son[1]); // can't write
    srand(seed + id);

    int zero_count = 0, minus_one_count = 0, one_count = 0, num_got = 0;

    while (true)
    {
        int random_number = rand() % 10;
        // The father needs to know which son sent the message, 
        // so for the child with the id = 1, we will multiply by 10.
        random_number += id * 10;

        if (write(pipe_sons_dad[1], &random_number,
            sizeof(random_number)) != sizeof(random_number))
        {
            perror("Invalid arguments! \n");
            exit(EXIT_FAILURE);
        }

        // blocking the  signal
        // calling the SIGUSR1 can interrupt the call, and the read()
        // will return -1. So we make sure the signal is handled only
        // after the read()
        sigset_t block_mask;
        sigemptyset(&block_mask);
        sigaddset(&block_mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &block_mask, NULL);

        if (read(pipe_dad_son[0], &num_got, sizeof(num_got))
            != sizeof(num_got))
        {
            perror("Invalid arguments! \n");
            exit(EXIT_FAILURE);
        }
        sigprocmask(SIG_UNBLOCK, &block_mask, NULL); // free signal

        update_counters_after_read(num_got, &zero_count, &one_count,
            &minus_one_count);

        if (finish || zero_count == FINISED)
        {
            print_killed(id, minus_one_count, zero_count, one_count,
                pipe_sons_dad, pipe_dad_son);
        }
    }
}

//----------------------------------------------------------------------

/* The function checks if a fork operation was successful.
 * The function receives: a child process ID.
 * The function returns: void.
 */
void check_fork(pid_t child)
{
    if (child < 0)
    {
        perror("Can't fork");
        exit(EXIT_FAILURE);
    }
}

//----------------------------------------------------------------------

/* The function swaps the values of two integers.
 * The function receives: two integer pointers.
 * The function returns: void.
 */
void swap(int* first_num, int* second_num)
{
    int temp;
    temp = *first_num;
    *first_num = *second_num;
    *second_num = temp;
}

//----------------------------------------------------------------------

/* The function updates the values of the result and win counters for
 *  the children based on the game outcome.
 * The function receives: pointers to the win counter and the results
 *  of the two children.
 * The function returns: void.
 */
void update_values(int* won, int* first_result, int* second_result)
{
    (*won)++;
    *first_result = LOWER;
    *second_result = HIGHER;
}

//----------------------------------------------------------------------

/* The function prints a message when a child process wins and sends
 *  a termination signal to both child processes.
 * The function receives: two child process IDs and the ID of the
 *  winning child.
 * The function returns: void.
 */
void print_won(pid_t first_child, pid_t second_child, int num_child)
{
    printf("Child #%d won\n", num_child);
    kill(first_child, SIGUSR1);
    kill(second_child, SIGUSR1);
    finish = true;
}

//----------------------------------------------------------------------

/* The function updates counters based on the value received from the
 *  parent process.
 * The function receives: the received value and pointers to three
 *  counters for `-1`, `0`, and `1`.
 * The function returns: void.
 */
void update_counters_after_read(int num_got, int* zero_count,
    int* one_count, int* minus_one_count)
{
    switch (num_got)
    {
    case 0:
    {
        (*zero_count)++;
        break;
    }

    case 1:
    {
        (*one_count)++;
        break;
    }

    case -1:
    {
        (*minus_one_count)++;
        break;
    }
    }
}

//----------------------------------------------------------------------

/* The function handles the actions of the parent process, including
 *  reading data from child processes and determining the winner.
 * The function receives: two child process IDs and three pipes for
 *  communication.
 * The function returns: void.
 */
void do_dad(pid_t first_child, pid_t second_child,
    const int pipe_sons_dad[], const int pipe_dad_first_son[],
    const int pipe_dad_second_son[])
{
    int first_value = 0, second_value = 0, first_won = 0,
        second_won = 0, first_result = 0, second_result = 0;
    while (!finish)
    {
        first_result = 0, second_result = 0;
        if (read(pipe_sons_dad[0], &first_value, sizeof(first_value))
            != sizeof(first_value))
        {
            perror("Invalid arguments! \n");
            exit(EXIT_FAILURE);
        }

        if (read(pipe_sons_dad[0], &second_value, sizeof(second_value))
            != sizeof(second_value))
        {
            perror("Invalid arguments! \n");
            exit(EXIT_FAILURE);
        }

        // first_num - {0,..,9} second_num - {10,..., 19}
        // we want 10 to have leftover, so we need to add 1 to the value
        if ((second_value + 1) % DIVIDE == 0)
        {
            swap(&first_value, &second_value);
        }
        second_value /= DIVIDE;

        if (second_value > first_value)
        {
            update_values(&second_won, &first_result, &second_result);
        }

        else if (second_value < first_value)
        {
            update_values(&first_won, &second_result, &first_result);
        }

        if (first_won == WON)
        {
            print_won(first_child, second_child, 0);

        }
        else if (second_won == WON)
        {
            print_won(first_child, second_child, 1);
        }

        // write to children
        if (write(pipe_dad_first_son[1], &first_result,
            sizeof(first_result)) != sizeof(first_result))
        {
            perror("Invalid arguments! \n");
            exit(EXIT_FAILURE);
        }

        if (write(pipe_dad_second_son[1], &second_result,
            sizeof(second_result)) != sizeof(second_result))
        {
            perror("Invalid arguments! \n");
            exit(EXIT_FAILURE);
        }
    }

    close(pipe_dad_second_son[1]);
    close(pipe_dad_first_son[1]);
    close(pipe_sons_dad[0]);

    int status;
    waitpid(first_child, &status, 0);
    waitpid(second_child, &status, 0);
}

//----------------------------------------------------------------------

/* The function prints a message when a child is killed and closes the
 * pipes associated with it.
 * The function receives: an integer child ID, counters for each
 * possible read value, and two pipes.
 * The function returns: void.
 */
void print_killed(int id, int minus_one_count, int zero_count,
    int one_count, const int pipe_sons_dad[], const int pipe_dad_son[])
{
    printf("Child #%d was killed: %d %d %d\n", id, minus_one_count,
        zero_count, one_count);
    close(pipe_sons_dad[1]);
    close(pipe_dad_son[0]);
    exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* The function handles SIGUSR1 signals by setting a global flag to
 * indicate termination.
 * The function receives: the signal number.
 * The function returns: void.
 */
void kill_children(int sig)
{
    finish = true;
}

//----------------------------------------------------------------------

/* The function creates the connections, and starts the comparation.
 * The function receives: the seed number.
 * The function returns: void.
 */
void create_pipes_and_run(int seed)
{
    // Create pipes
    int pipe_dad_first_son[2]; // dad to first son
    int pipe_dad_second_son[2]; // dad to second son
    int pipe_sons_dad[2]; // sons to dad

    if (pipe(pipe_dad_first_son) == -1 || pipe(pipe_sons_dad) == -1)
    {
        perror("Can't pipe");
        exit(EXIT_FAILURE);
    }

    int id = 0;
    pid_t first_child = fork();
    check_fork(first_child);

    if (first_child == 0)
    {
        do_child(pipe_dad_first_son, pipe_sons_dad, seed, id);
    }

    // only dad enters here
    if (pipe(pipe_dad_second_son) == -1)
    {
        perror("Can't pipe");
        exit(EXIT_FAILURE);
    }

    id = 1;
    pid_t second_child = fork();
    check_fork(second_child);

    if (second_child == 0)
    {
        close(pipe_dad_first_son[0]);
        close(pipe_dad_first_son[1]);
        do_child(pipe_dad_second_son, pipe_sons_dad, seed, id);
    }

    close(pipe_dad_second_son[0]); // cant read
    close(pipe_dad_first_son[0]); // cant read
    close(pipe_sons_dad[1]); // cant write

    do_dad(first_child, second_child, pipe_sons_dad,
        pipe_dad_first_son, pipe_dad_second_son);
}
