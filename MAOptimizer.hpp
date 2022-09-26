#ifndef MAOPTIMIZER_H
#define MAOPTIMIZER_H
#include <iostream>
#include "init.hpp"
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <deque>
#include <queue>
#include <cmath>

class MAOptimizer
{

public:
    Population population;
    std::vector<float> asset1; // loaded close data for asset 1 (market)
    std::vector<float> asset1Open;
    unsigned lenAsset1;        // how many lines of data
    std::vector<float> asset2; // loaded data for asset 2 (inter-market)
    std::vector<float> asset2Open;
    unsigned lenAsset2; // how many lines of data
    unsigned numGenerations;

    MAOptimizer(unsigned numGenerations, int lenPop,
                float mutationProb, std::string asset1Path,
                std::string asset2Path); // constructor to be implemented

    /**
     * @brief Calculates the moving average from a deque
     * 
     * @param data vector of data to avg
     * @return float moving average from data
     */
    float getMA(std::deque<float> data);

    /**
     * @brief Calculates the return of a trade and stores information in referenced trade object
     * 
     * @param t trade to calculate for 
     * @param exitPrice Price at which trade is being exited
     */
    float calculateReturn(Trade t, float exitPrice);

    /**
     * @brief Calculate mean of vector
     * 
     * @param vec Vector to find mean of
     * @return float average of elements in vector
     */
    float mean(std::vector<float> vec);

    /**
     * @brief Calculates covariance between vectors
     * sum [x - meanx * y - meany] / n - 1
     * 
     * @param v1 
     * @param v2 
     * @return float 
     */
    float covariance(std::vector<float> v1, std::vector<float> v2);

    /**
     * @brief Calculates variance of vector
     * 
     * @param vec Input vector
     * @return float variance of vector elements
     */
    float var(std::vector<float> vec);

    /**
     * @brief Standard deviation of elements in vector
     * 
     * @param vec 
     * @return float 
     */
    float std(std::vector<float> vec);

    /**
     * @brief Calculates the correlation coefficient between two vectors
     * 
     * @param v1 Vector 1
     * @param v2 Vector 2
     * @return float Correlation coefficient
     */
    float cor(std::vector<float> v1, std::vector<float> v2);

    /**
     * @brief Calls Genome constructor
     * 
     * @return Genome object
     */
    Genome generateGenome();

    /**
     * @brief Calls generateGenome function for "len" number of times and creates Population object
     * which gets appended to and eventually inserted as class variable in here
     * 
     * @param len how many genomes in population
     */
    void generatePopulation(unsigned len);

    /**
     * @brief Reads from 2 CSV files and loads them into the vector class variables
     * 
     * @param asset1Path path
     * @param asset2Path path
     */
    void readAssetCSV(std::string asset1Path, std::string asset2Path);

    /**
     * @brief updates Genome obj's fitness value (asset return given genomes optim values)
     * 
     * @param genome genome to evaluate
     */
    void fitness(Genome genome);

    /**
     * @brief selects the two best performing (parents)
     * 
     * @return Population containing two best performing genomes
     */
    Population selectionPair();

    /**
     * @brief Combines or swaps an element between the two genomes
     * 
     * @param a Genome 1
     * @param b Genome 2
     * @return std::vector<Genome> the modified genomes 
     */
    std::vector<Genome> singlePointCrossover(Genome a, Genome b);

    /**
     * @brief Mutates the given genome altering its optim values
     * 
     * @param genome Genome to mutate
     * @param probability probability of mutation
     * @return Genome mutated genome
     */
    void mutate(Genome genome, unsigned probability);

    /**
     * @brief Runs the appropriate functions and writes to memory the data as it's proccessed
     * 
     */
    void Evolve(int lenPop, float mutationProb);
};

#endif