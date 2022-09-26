#ifndef INIT_H
#define INIT_
#include <vector>

class Genome
{
public:
    unsigned lowerBound;
    unsigned upperBound;
    std::vector<unsigned> optim; // elements to be optimized
    float fitnessValue;          // cumulative % return

    Genome();
};

class Population
{
public:
    std::vector<Genome> pop; // collection of genomes
};

class Trade
{
public:
    std::string symbol;
    std::string direction; // "SHORT" or "LONG"
    float fillPrice;
    float exitPrice;
    float netReturn;
    float pctReturn;

    Trade(std::string direction, float fillPrice);
};

#endif