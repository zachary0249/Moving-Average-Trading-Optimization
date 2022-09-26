#include "MAOptimizer.hpp"
using namespace std;

float MAOptimizer::getMA(deque<float> data)
{
    float avg = 0;
    for (int i = 0; i < data.size(); i++)
    {
        avg += data[i];
    }
    return avg / data.size();
}

Genome::Genome()
{
    this->optim.reserve(2);
    this->lowerBound = 1;
    this->upperBound = 200;
    // setting the MA lengths to be rand number between lower and upper bound
    this->optim[0] = this->lowerBound + (rand() % (this->upperBound - this->lowerBound + 1));
    this->optim[1] = this->lowerBound + (rand() % (this->upperBound - this->lowerBound + 1));
    this->fitnessValue = 0; // init to 0
}

Trade::Trade(string direction, float fillPrice)
{
    this->direction = direction;
    this->fillPrice = fillPrice;
}

float MAOptimizer::calculateReturn(Trade t, float exitPrice)
{
    t.netReturn = exitPrice - t.fillPrice;
    t.pctReturn = (exitPrice - t.fillPrice) / t.fillPrice;
    return t.pctReturn;
}

float MAOptimizer::mean(vector<float> vec)
{
    float sum = 0.;
    for (unsigned i = 0; i < vec.size(); i++)
    {
        sum += vec[i];
    }
    return sum / vec.size();
}

float MAOptimizer::var(vector<float> vec)
{
    float var = 0.;
    float mean1 = mean(vec);
    for (unsigned i = 0; i < vec.size(); i++)
    {
        var += powf(vec[i] - mean1, 2);
    }
    return var / (vec.size() - 1);
}

float MAOptimizer::std(vector<float> vec)
{
    float variance = var(vec);
    return sqrtf(variance);
}

float MAOptimizer::cor(vector<float> v1, vector<float> v2)
{
    float cov = covariance(v1, v2);
    float std1 = std(v1);
    float std2 = std(v2);
    return cov / (std1 * std2);
}

float MAOptimizer::covariance(vector<float> v1, vector<float> v2)
{
    if (v1.size() == v2.size())
    {
        float sum = 0.;
        float mean1 = mean(v1);
        float mean2 = mean(v2);
        for (unsigned i = 0; i < v1.size(); i++)
        {
            sum += (v1[i] - mean1) * (v2[i] - mean2);
        }
        return sum / (v1.size() - 1); // sample calculation ie n - 1 rather than just n
    }
    else
    {
        cout << "Vectors aren't the same length." << endl;
        exit(0);
    }
}

MAOptimizer::MAOptimizer(unsigned numGenerations, int lenPop,
                         float mutationProb, string asset1Path,
                         string asset2Path)
{
    this->numGenerations = numGenerations;
    this->readAssetCSV(asset1Path, asset2Path);

    this->Evolve(lenPop, mutationProb);
}

Genome MAOptimizer::generateGenome()
{
    return Genome();
}

void MAOptimizer::generatePopulation(unsigned len)
{
    for (int i = 0; i < len; i++)
    {
        this->population.pop.push_back(this->generateGenome());
    }
}

void MAOptimizer::readAssetCSV(string asset1Path, string asset2Path)
{
    /*
    Requires the CSV data be in format [close, open] for both assets
    */

    string line, asset1Close, asset1Open, asset2Open, asset2Close;
    int nlines = 0;           // number of lines in file
    ifstream fin(asset1Path); // open file
    if (fin.is_open())        // if file is open
    {
        // ignore first line
        getline(fin, line);
        while (!fin.eof())
        {
            getline(fin, asset1Close, ',');
            this->asset1.push_back(stof(asset1Close));
            getline(fin, asset1Open, ',');
            this->asset1Open.push_back(stof(asset1Open));
            nlines++;
        }
        fin.close();
        this->lenAsset1 = --nlines;
        cout << nlines << " entries successfully loaded. \n"
             << endl;
    }

    // reading asset #2
    nlines = 0;                // number of lines in file
    ifstream fin2(asset2Path); // open file
    if (fin2.is_open())        // if file is open
    {
        // ignore first line
        getline(fin2, line);

        while (!fin2.eof())
        {

            getline(fin2, asset2Close, ',');
            this->asset2.push_back(stof(asset2Close));
            getline(fin2, asset2Open, ',');
            this->asset2Open.push_back(stof(asset2Open));
            nlines++;
        }
        fin2.close();
        this->lenAsset2 = --nlines;
        cout << nlines << " entries successfully loaded. \n"
             << endl;
    }
}

/**
 * @brief Calculates a moving average for each asset based on the genomes optum values
 * 
 * @param genome Genome to access 
 */
void MAOptimizer::fitness(Genome genome)
{
    // reset genomes fitness value if it isn't 0
    if (genome.fitnessValue != 0.)
    {
        genome.fitnessValue = 0.;
    }
    queue<Trade> trades;
    float markInd, interInd; // prices relative to MA for asset1 and asset2 respectively
    deque<float> asset1MA;   // moving average
    deque<float> asset2MA;   // moving average

    if (this->asset1.size() != this->asset2.size())
        cout << "Sizes of two data sets aren't equal." << endl;
    else
    {
        for (int i = 0; i < this->asset1.size(); i++)
        {
            // adding to MA queue
            asset1MA.push_back(this->asset1[i]);
            asset2MA.push_back(this->asset2[i]);
            // keeping MA size under their threshold
            if (asset1MA.size() > genome.optim[0])
                asset1MA.pop_front();
            if (asset2MA.size() > genome.optim[1])
                asset2MA.pop_front();

            // evaluating performance if MA allow for it
            if (asset1MA.size() != genome.optim[0] ||
                asset2MA.size() != genome.optim[1])
                continue;
            else
            {
                // getting price in relation to MAs
                markInd = asset1[i] - this->getMA(asset1MA);
                interInd = asset2[i] - this->getMA(asset2MA);

                // trade entry / exit logic
                // if theres an open position: close it if it's in opposite direction
                // and open position
                float correlation = cor(this->asset1, this->asset2);

                // for positive correlation between assets
                if (correlation > 0.)
                {
                    if (interInd > 0 && markInd < 0) // buy market asset (close short) positive correlation
                    {
                        if (!trades.empty() && trades.front().direction == "SHORT")
                        {
                            genome.fitnessValue += calculateReturn(trades.front(), asset1Open[i + 1]);
                            trades.pop();
                            trades.push(Trade("LONG", asset1Open[i + 1])); // buys at next bar's open
                        }
                        else if (trades.empty())
                        {
                            trades.push(Trade("LONG", asset1Open[i + 1])); // buys at next bar's open
                        }
                    }
                    else if (interInd < 0 && markInd > 0) // sell market asset
                    {
                        if (!trades.empty() && trades.front().direction == "LONG")
                        {
                            genome.fitnessValue += calculateReturn(trades.front(), asset1Open[i + 1]);
                            trades.pop();
                            trades.push(Trade("SHORT", asset1Open[i + 1])); // sells at next bar's open
                        }
                        else if (trades.empty())
                        {
                            trades.push(Trade("SHORT", asset1Open[i + 1])); // sells at next bar's open
                        }
                    }
                }
                else if (correlation < 0.)
                {
                    if (interInd < 0 && markInd < 0) // buy market asset (close short) positive correlation
                    {
                        if (!trades.empty() && trades.front().direction == "SHORT")
                        {
                            genome.fitnessValue += calculateReturn(trades.front(), asset1Open[i + 1]);
                            trades.pop();
                            trades.push(Trade("LONG", asset1Open[i + 1])); // buys at next bar's open
                        }
                        else if (trades.empty())
                        {
                            trades.push(Trade("LONG", asset1Open[i + 1])); // buys at next bar's open
                        }
                    }
                    else if (interInd > 0 && markInd > 0) // sell market asset
                    {
                        if (!trades.empty() && trades.front().direction == "LONG")
                        {
                            genome.fitnessValue += calculateReturn(trades.front(), asset1Open[i + 1]);
                            trades.pop();
                            trades.push(Trade("SHORT", asset1Open[i + 1])); // sells at next bar's open
                        }
                        else if (trades.empty())
                        {
                            trades.push(Trade("SHORT", asset1Open[i + 1])); // sells at next bar's open
                        }
                    }
                }
                else if (correlation == 0.)
                {
                    cout << "0 correlation between assets." << endl;
                    return;
                }
            }
        }
    }
}

Population MAOptimizer::selectionPair()
{
    // initialize vector of fitness values
    vector<float> fitValues;
    for (unsigned j = 0; j < this->population.pop.size(); j++)
    {
        fitValues.push_back(this->population.pop[j].fitnessValue);
    }

    int maxdex, max2dex;
    float maxv = max(fitValues[0], fitValues[1]), max2v = min(fitValues[0], fitValues[1]);
    if (maxv == fitValues[0])
    {
        maxdex = 0, max2dex = 1;
    }
    else
    {
        maxdex = 1, max2dex = 0;
    }

    // finding index and value of two greatest fitness values
    for (unsigned i = 0; i < fitValues.size(); i++)
    {
        if (fitValues[i] > maxv)
        {
            max2v = maxv;
            max2dex = maxdex;
            maxv = fitValues[i];
            maxdex = i;
        }
        else if (fitValues[i] > max2v && fitValues[i] < maxv)
        {
            max2v = fitValues[i];
            max2dex = i;
        }
    }
    // pushing the two genomes with max fitness value to population and returning
    Population p;
    p.pop.push_back(this->population.pop[maxdex]);
    p.pop.push_back(this->population.pop[max2dex]);
    return p;
}

vector<Genome> MAOptimizer::singlePointCrossover(Genome a, Genome b)
{
    // swaps random element of genome optim values
    int rdex = rand() % 2; // random index either 0 or 1
    int altdex = (rdex == 0) ? 1 : 0;
    int tmp;
    tmp = a.optim[rdex];
    a.optim[rdex] = b.optim[altdex];
    b.optim[altdex] = tmp;
    vector<Genome> modified;
    modified.push_back(a);
    modified.push_back(b);
    return modified;
}

void MAOptimizer::mutate(Genome genome, unsigned probability)
{
    // randomly selects an optim value and randomizes it
    int rdex = rand() % 2;
    float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    if (r <= probability)
    {
        genome.optim[rdex] = genome.lowerBound + (rand() % (genome.upperBound - genome.lowerBound + 1));
    }
}

void MAOptimizer::Evolve(int lenPop, float mutationProb)
{
    cout << "Genetic Optimization initialized..."
         << "\n\n";
    generatePopulation(lenPop);

    Genome offspring1, offspring2;
    for (unsigned i = 0; i < this->numGenerations; i++)
    {
        cout << "Gen: " << i + 1 << "Population % Return: " << endl;
        for (unsigned j = 0; j < lenPop; j++)
        {
            cout << this->population.pop[j].fitnessValue << " ";
        }
        cout << "\n\n";

        Population nextGen = selectionPair();

        for (unsigned z = 0; z < static_cast<int>(this->population.pop.size() / 2) - 1; z++)
        {
            vector<Genome> offspring = singlePointCrossover(nextGen.pop[0], nextGen.pop[1]);
            offspring1 = offspring[0], offspring2 = offspring[1];

            mutate(offspring1, mutationProb);
            mutate(offspring2, mutationProb);
            nextGen.pop.push_back(offspring1);
            nextGen.pop.push_back(offspring2);
        }

        if (this->population.pop.size() == nextGen.pop.size())
        {
            cout << "Population and next gen are same size!" << endl;
            this->population = nextGen;
        }
        else
        {
            cout << "Population and next gen aren't same size!!!" << endl;
            exit(0);
        }
    }

    cout << "Finished " << this->numGenerations << " generations." << endl;
}

int main()
{
    MAOptimizer opt(10, 5, 0.5, "/home/zacharyhayden/Documents/programming/c++/MAOptimizer/src/DIA.csv",
                    "/home/zacharyhayden/Documents/programming/c++/MAOptimizer/src/oil.csv");
}