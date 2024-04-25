/*
 * @file <src/modules/AutoMoFsmUpdator.cpp>
 *
 * @author Florian Noussa-Yao - <fnoussay@ulb.be>
 *
 * @package ARGoS3-AutoMoDe
 *
 * @license MIT License
 */

#include "AutoMoDeFsmUpdator.h"
#include <sstream> // Include the <sstream> header for stringstream
#include <vector> // Include the <vector> header for vector 
#include <random>
#include <algorithm>
#include <cstdio>
#include "../core/AutoMoDeController.h"
#include <cstdio>
#include <iostream>


std::shared_future<int>  AutoMoDeFsmUpdator::UpdateFsmLauncherAsync(::argos::AutoMoDeController* instance, int m_unTimeStep, std::string* old_fsm){
    return std::async(std::launch::async, &AutoMoDeFsmUpdator::UpdateFsmLauncher, this, instance, m_unTimeStep, old_fsm);
}


int AutoMoDeFsmUpdator::UpdateFsmLauncher(AutoMoDeController* instance, int m_unTimeStep, std::string* old_fsm){
    // Every 100 time steps, we will run an heuristic optimization process to update the fsm.
    // The optimization process will use as objective function the number of time steps the robot has been in the state 0.
    // The heuristic optimization process will change the parameters of the fsm i other to maximize the objective function value.
    printf("UpdateFsmLauncher started\n");
    int timelapsed = 30;
    if (m_unTimeStep % timelapsed == 0){
        printf("UpdateFsmLauncher checkpoint1\n");
        /* float delta_reward = instance->NYF_reward - old_reward;
        old_reward = instance->NYF_reward;
        rate_reward = delta_reward / timelapsed;
        printf("\n \n delta_reward: %.2f\n", delta_reward);
        printf("rate_reward: %.2f\n", rate_reward);
        if (rate_reward >= default_rate_reward){
            default_rate_reward = rate_reward;
            best_old_fsm = old_fsm;
            instance->UpdateFSM(*old_fsm);
        }
        else {
            old_fsm = best_old_fsm;
            //print("best_old-fsm") for checking
            printf("best fsm to date = %s\n",old_fsm->c_str());
            std::string newfsm=New_fsm_generator(old_fsm);
            instance->UpdateFSM(newfsm); 
        } */
        //fsm_params = extractFsmParams(old_fsm);
        fsm_params = Genetic_Heuristic(old_fsm, fsm_params);
        std::string new_fsm = Params_to_fsm(old_fsm, fsm_params);
        printf("finished\n");
        instance->UpdateFSM(new_fsm);
        
    }
    printf("UpdateFsmLauncher finished\n");
    return 0;
}
std::string AutoMoDeFsmUpdator::New_fsm_generator(std::string* old_fsm){
    // Implementation of the New_fsm_generator function

    // randomly change the params by others. this need to be change with a more sophisticated method such as a genetic algorithm or a hill climbing algorithm
    
    //print( params_values)
    printf("params_values\n");
    for (size_t i = 0; i < fsm_params.size(); i++) {
        printf("%.2f,", fsm_params[i]);
    }
    // then change the values of the params while keeping a tack of previously used values
    std::vector<float> new_params; // Declare the new_params vector
    std::vector<std::vector<float>> used_values; // Change the type of used_values to std::vector<std::vector<float>>

    //part to change to heuristic 
    do {
        new_params = generateNewParams(fsm_params);
    } while (std::find(used_values.begin(), used_values.end(), new_params) != used_values.end());

    used_values.push_back(new_params); // Store new params to avoid repetition later

    // add new-params to used_values
    used_values.push_back(new_params);
    fsm_params = new_params; // need to change this to make sure fsm_params used is actually the one belonging to the old_best_fsm

    //print(new_params) on single line 
    printf("\nnew_params\n");
    for (size_t i = 0; i < new_params.size(); i++) {
        printf("%.2f , ", new_params[i]);
    }

    // change new params in best_old fsm 
    std::string new_fsm = Params_to_fsm(old_fsm, new_params);
    ///print(new_fsm)
    //printf("%s\n", new_fsm.c_str());
    // The optimization process will use as objective function the number of time steps the robot has been in the state 0.
    return new_fsm;

}

std::vector<float> AutoMoDeFsmUpdator::generateNewParams(const std::vector<float>& old_params) {
        std::vector<float> new_params;
        new_params.reserve(old_params.size());

        // Initialize a random number generator
        static std::random_device rd;  // Use a random_device to seed the generator for better randomness
        static std::default_random_engine generator(rd());
        float min_val = 0.0f;
        float max_val = 0.0f;
        for (size_t i = 0; i < old_params.size(); i++) {
            // Create a normal distribution centered around the old parameter value
            std::normal_distribution<float> distribution(old_params[i], 0.1);
            // ATTENTION need to handle value of C0x0 like params since the following lead to a switching from config else to if reciprocally 
            if (old_params[i] <= 1){
                min_val = 0.0f;
                max_val = 1.0f;
            }
            else {
                min_val = 1.0f;
                max_val = 10.0f;
            }
            float new_value;
            // Generate a new value within bounds
            do {
                new_value = distribution(generator);
            } while (new_value <= min_val || new_value >= max_val); // Ensure new value is clamped between 0 and 1

            new_params.push_back(new_value);
        }

        return new_params;
    }

// Genetic algorithm utilities
void mutate(std::vector<float>& individual, float mutationRate) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<> d(0, 1);

    for (auto& gene : individual) {
        if (std::generate_canonical<double, 10>(gen) < mutationRate) {
            gene += d(gen);
            // Ensure bounds are maintained
            if (gene < 0) gene = 0;
            if (gene > 10) gene = 10; // Adjust this if necessary
        }
    }
}

std::vector<float> crossover(const std::vector<float>& parent1, const std::vector<float>& parent2) {
    static std::random_device rd; // this is a random device
    static std::mt19937 gen(rd()); // this is a random number generator
    std::vector<float> child(parent1.size());
    std::uniform_int_distribution<> distrib(0, 1);
    for (size_t i = 0; i < parent1.size(); i++) {
        child[i] = distrib(gen) ? parent1[i] : parent2[i]; // Randomly select a gene from either parent
    }
    return child;
}


float AutoMoDeFsmUpdator::Objective(std::vector<float> individual) {
     // Objective function to maximize

    if (already_evaluated.find(individual) != already_evaluated.end()) {
        return already_evaluated[individual];
    }
    // Prepare the command
    std::string command = "python3 /home/ubuntu/daremo/tuning/Off-policySwarmPerformanceEstimation-master/AutoMoDeLogAnalyzer.py /home/ubuntu/daremo/tuning/Off-policySwarmPerformanceEstimation-master/iraceTools/fsm_logs/1-100_90_fsm_log.txt -pa --newfsm-config --nstates 2 --s0 0 --rwm0 57 --n0 4 --n0x0 0 --c0x0 0 --p0x0 0.35 --n0x1 0 --c0x1 0 --p0x1 0.69 --n0x2 0 --c0x2 1 --p0x2 0.82 --n0x3 0 --c0x3 5 --p0x3 0.66 --s1 0 --rwm1 58 --n1 2 --n1x0 0 --c1x0 3 --p1x0 5 --w1x0 12.39 --n1x1 0 --c1x1 5 --p1x1 0.79";

    // Open the pipe
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Couldn't start command." << std::endl;
        return 0;
    }

    // Read the output
    char buffer[128];
    float wis_value = 0.0;
    while (fgets(buffer, sizeof buffer, pipe) != NULL) {
        // Look for the pattern "WIS : <value>"
        if (sscanf(buffer, "OIS Expected average performance with the proportional reward        : %f", &wis_value) == 1) {
            break;
        }
    }

    // Close the pipe
    pclose(pipe);

    already_evaluated[individual] = wis_value;

    return wis_value;
}

std::vector<float>  AutoMoDeFsmUpdator::Genetic_Heuristic(std::string* old_fsm, std::vector<float> fsm_params){
    // Genetic algorithm to optimize the parameters of the FSM
    // Initialize the population
    std::vector<std::vector<float>> population(5);
    for (auto& individual : population) {
        individual = generateNewParams(fsm_params);
    }

    // Evaluate the population
    std::vector<float> fitness(population.size());
    for (size_t i = 0; i < population.size(); i++) {
        fitness[i] = Objective(population[i]);
    }

    // Sort the population by fitness
    std::vector<size_t> indices(population.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&fitness](size_t i1, size_t i2) { return fitness[i1] > fitness[i2]; });


    // Select the best individuals
    std::vector<std::vector<float>> new_population(population.size());
    for (size_t i = 0; i < population.size(); i++) {
        new_population[i] = population[indices[i]];
    } 

    // Crossover
    for (size_t i = 0; i < population.size()-1; i++) {
        new_population[i] = crossover(new_population[i], new_population[i + 1]);
    }

    printf("After crossover: \n");
    for (size_t i = 0; i < new_population.size(); i++) {
        printf("Individual %zu: ", i);
        for (size_t j = 0; j < new_population[i].size(); j++) {
            printf("%.2f, ", new_population[i][j]);
        }
        printf("\n");
    }
    // Mutate
    for (size_t i = 0; i < population.size(); i++) {
        mutate(new_population[i], 0.1);
    }

    // Update the population
    population = new_population;

    // Evaluate the population
    for (size_t i = 0; i < population.size(); i++) {
        fitness[i] = Objective(population[i]);
    }

    // Sort the population by fitness
    std::sort(indices.begin(), indices.end(), [&fitness](size_t i1, size_t i2) { return fitness[i1] > fitness[i2]; });


    // print the best individual
    printf("Best individual: ");
    for (size_t i = 0; i < population[indices[0]].size(); i++) {
        printf("%.2f, ", population[indices[0]][i]);
    }
    printf("\n");
    
    // Return the best individual
    return population[indices[0]];

}

std::string AutoMoDeFsmUpdator::Params_to_fsm(std::string* old_fsm, std::vector<float> new_params){
    // change new params in best_old fsm 
    std::stringstream ss2(*old_fsm);
    std::string token2;
    std::string new_fsm = *old_fsm; 
    size_t i = 0;
    while (i < new_params.size()) {
        std::getline(ss2, token2, ' ');
        if (token2.find("p") != std::string::npos){
            //we change the next token which is the value of the parameter
            std::getline(ss2, token2, ' ');
            size_t pos = new_fsm.find(token2);
            if (pos != std::string::npos) {
                std::stringstream new_param_ss;
                new_param_ss << new_params[i];
                new_fsm.replace(pos, token2.length(), new_param_ss.str());
                i++;
            }
        }
        
    }
    return new_fsm;
}

std::vector<float> AutoMoDeFsmUpdator::extractFsmParams(std::string* fsm) {
    std::vector<float> fsm_params;
    std::stringstream ss(*fsm);
    std::string token;
    while (std::getline(ss, token, ' ')) {
        if (token.find("p") != std::string::npos) {
            std::getline(ss, token, ' ');
            fsm_params.push_back(std::stof(token));
        }
    }
    return fsm_params;
}
    