/*
 * Ex #1: Comparation between series search and binary search.
 * ==========================================================
 * Written by: Jacob Bondar.
 *
 * This program performs two types of searches (binary and linear) on
 *  arrays filled with random values. It creates child processes for
 *  each search type, measures the time taken for each search, and
 *  writes the results (number of matches and time taken) to the
 *  father. The program runs multiple rounds
 *  and calculates overall search performance.
 *
 * Input: An integer that represents the seed.
 *
 * Output: Run time of the series search, binary search and the main.
 */

 //-------------- include section ---------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>

//-------------- const section -----------------------------------------

const int NUM_OF_ROUNDS = 10;
const int VALUES_IN_ARR = 100000;
const int NUM_OF_CHILDREN = 2;
const int CORRECTION_NUMBER = 1000000;

//-------------- prototypes section ------------------------------------

void child_get_ready(const int pipe_sons_dad[]);
void do_father(const int pipe_sons_dad[], float total_time_main);
void do_father(const int pipe_sons_dad[], float total_time_main);
void receive_time(float* total_time, int* counter);
void insertValuesInArrs(int binary_arr[], int series_arr[]);
bool valid_fork(pid_t status);
void sort(int arr[]);
pid_t create_child();
void binary_search(const int arr[], const int pipe_sons_dad[]);
void series_search(const int arr[], const int pipe_sons_dad[]);
void create_child_and_search(const int arr_s[], const int arr_b[],
    const int pipe_sons_dad[]);
float get_total_time_and_wait(int is_binary);
int compare(const void* a, const void* b);

//-------------- main --------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        perror("Enter valid file name and a number/n");
        exit(EXIT_FAILURE);
    }

    int seed = atoi(argv[1]);
    srand(seed);

    int binary_arr[VALUES_IN_ARR], series_arr[VALUES_IN_ARR];
    float total_time_main = 0;

    int pipe_sons_dad[2];
    if (pipe(pipe_sons_dad) == -1)
    {
        perror("Can't pipe \n");
        exit(EXIT_FAILURE);
    }

    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    for (int round = 0; round < NUM_OF_ROUNDS; round++)
    {
        insertValuesInArrs(binary_arr, series_arr);
        sort(binary_arr);

        create_child_and_search(series_arr, binary_arr, pipe_sons_dad);
    }

    gettimeofday(&t1, NULL);

    total_time_main = (double)(t1.tv_usec - t0.tv_usec) /
        CORRECTION_NUMBER + (double)(t1.tv_sec - t0.tv_sec);

    do_father(pipe_sons_dad, total_time_main);

    exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* The function creates a child process and performs a search based on
 *  the input flag.
 * The function receives: an array of integers, a file pointer,
 *  a pointer to a double, and an integer.
 * The function returns: void.
 */
void create_child_and_search(const int arr_s[], const int arr_b[],
    const int pipe_sons_dad[])
{
    pid_t series_child = create_child();
    if (series_child == 0)
    {
        child_get_ready(pipe_sons_dad);
        series_search(arr_s, pipe_sons_dad);
    }

    pid_t binary_child = create_child();
    if (binary_child == 0)
    {
        child_get_ready(pipe_sons_dad);
        binary_search(arr_b, pipe_sons_dad);
    }
    wait(NULL);
    wait(NULL);
}

//----------------------------------------------------------------------

/* The function performs a binary search on an array and writes results
 *  to the stdout, which goes to the father.
 * The function receives: an array of integers and pipe.
 * The function returns: void.
 */
void binary_search(const int arr[], const int pipe_sons_dad[])
{
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    srand(time(NULL));
    unsigned int counter = 0;
    float t_time = 0;

    for (int round = 0; round < VALUES_IN_ARR; round++)
    {

        int low = 0, high = VALUES_IN_ARR - 1;

        int random_number = rand() % (VALUES_IN_ARR * NUM_OF_ROUNDS);

        while (low < high)
        {
            int mid = (low + high) / 2;
            if (random_number == arr[mid])
            {
                counter++;
                break;
            }
            else if (random_number < arr[mid]) high = mid - 1;
            else low = mid + 1;
        }

    }

    gettimeofday(&t1, NULL);

    t_time = (double)(t1.tv_usec - t0.tv_usec) / CORRECTION_NUMBER +
        (double)(t1.tv_sec - t0.tv_sec);

    printf("b %u %f ", counter, t_time);

    close(pipe_sons_dad[1]);
    exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* The function performs a linear search on an array and writes results
 *  to the stdout, which is the father.
 * The function receives: an array of integers and pipe.
 * The function returns: void.
 */
void series_search(const int arr[], const int pipe_sons_dad[])
{
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    srand(time(NULL));
    unsigned int counter = 0;
    float t_time = 0;

    for (int round = 0; round < VALUES_IN_ARR; round++)
    {
        int random_number = rand() % (VALUES_IN_ARR * NUM_OF_ROUNDS);

        for (int index = 0; index < VALUES_IN_ARR; index++)
        {
            if (arr[index] == random_number)
            {
                counter++;
                break;
            }
        }
    }
    gettimeofday(&t1, NULL);

    t_time = (double)(t1.tv_usec - t0.tv_usec) / CORRECTION_NUMBER +
        (double)(t1.tv_sec - t0.tv_sec);

    printf("s %u %f ", counter, t_time);

    close(pipe_sons_dad[1]);
    exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* The function creates a child process.
 * The function receives: no parameters.
 * The function returns: a process ID (pid_t).
 */
pid_t create_child()
{
    pid_t child = fork();

    if (!valid_fork(child))
    {
        fputs("Can't fork\n", stderr);
        exit(EXIT_FAILURE);
    }
    return child;
}

//----------------------------------------------------------------------

/* The function sorts an array of integers using the qsort function.
 * The function receives: an array of integers (`arr[]`).
 * The function returns: void (no return value).
*/
void sort(int arr[])
{
    qsort(arr, VALUES_IN_ARR, sizeof(int), compare);
}

//----------------------------------------------------------------------

/* The comparison function for qsort. This function compares two
 *  integers.
 * The function receives: two pointers to the elements being compared.
 * The function returns: an integer indicating the comparison result.
*/
int compare(const void* a, const void* b)
{
    return (*(int*)a - *(int*)b);
}

//----------------------------------------------------------------------

/* The function checks if a fork was successful.
 * The function receives: a process ID (pid_t).
 * The function returns: a boolean value.
 */
bool valid_fork(pid_t status)
{
    if (status < 0) return false;
    return true;
}

//----------------------------------------------------------------------

/* The function inserts random values into two arrays.
 * The function receives: 2 arrays of integers.
 * The function returns: void.
 */
void insertValuesInArrs(int binary_arr[], int series_arr[])
{
    for (int index = 0; index < VALUES_IN_ARR; index++)
    {
        int random_number = rand() % (VALUES_IN_ARR + 1);
        binary_arr[index] = random_number;
        series_arr[index] = random_number;
    }
}

//----------------------------------------------------------------------

/* The function reads from the stdout, and updated the values.
 * The function receives: 2 intereges by pointers.
 * The function returns: void.
 */
void receive_time(float* total_time, int* counter)
{
    float time;
    scanf("%f", &time);
    scanf("%f", &time);
    (*total_time) += time;
    (*counter)++;
}

//----------------------------------------------------------------------

/* The function recieves the information from the children, and prints
 *  them.
 * The function receives: A pipe and a float value.
 * The function returns: void.
 */
void do_father(const int pipe_sons_dad[], float total_time_main)
{
    dup2(pipe_sons_dad[0], STDIN_FILENO); // writes to stdout
    close(pipe_sons_dad[0]);
    close(pipe_sons_dad[1]);

    float total_time_s = 0, total_time_b = 0;
    char c;
    for (int counter = 0; counter < NUM_OF_ROUNDS * NUM_OF_CHILDREN;)
    {
        scanf("%c", &c);
        if (c == 'b')
        {
            receive_time(&total_time_b, &counter);
        }
        else if (c == 's')
        {
            receive_time(&total_time_s, &counter);
        }
    }

    printf("%.4f %.4f \n%.4f\n", (total_time_s / NUM_OF_ROUNDS),
        (total_time_b / NUM_OF_ROUNDS), total_time_main);
    close(pipe_sons_dad[0]);
}

//----------------------------------------------------------------------

/* The function changes the stdout of the child to the father, and
 *  closes the pipe.
 * The function receives: A pipe.
 * The function returns: void.
 */
void child_get_ready(const int pipe_sons_dad[])
{
    dup2(pipe_sons_dad[1], STDOUT_FILENO);
    close(pipe_sons_dad[1]);
    close(pipe_sons_dad[0]);
}
