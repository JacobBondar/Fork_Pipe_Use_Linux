# Fork_Pipe_Use_Linux

File: series_binary_search.c 

Ex #1: Comparation between series search and binary search.
==========================================================
Written by: Jacob Bondar.

This program performs two types of searches (binary and linear) on 
 arrays filled with random values. It creates child processes for 
 each search type, measures the time taken for each search, and 
 writes the results (number of matches and time taken) to the 
 father. The program runs multiple rounds
 and calculates overall search performance.

Compile: gcc -Wall series_binary_search.c –o series_binary_search
Run: ./series_binary_search <seed value>

Input: An integer that represents the seed.
 
Output: Run time of the series search, binary search and the main.

----------------------------------------------------------------------

File: duel_children.c 

Ex #2:  duel for the higher value.
==========================================================
Written by: Jacob Bondar.

This program simulates a competitive game between two child processes
 managed by a parent process using communication through pipes and
 signals. Each child generates random numbers, which are sent to the
 parent for comparison. The parent determines the winner of each
 round, updates the scores, and sends the results back to the 
 children. The game continues until one child achieves 120 win or 
 both of then achieve 100 same values. at which point the parent 
 declares the winner and terminates both child processes.

Compile: gcc -Wall duel_children.c –o duel_children
Run: ./duel_children <seed value>

Input: An integer that represents the seed.
 
Output: The winner (if there is one), and the children summary.
