#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <sstream>
#include <future>
#include <cstdio>
#include <cmath> 
#include <string>
#include <iostream>
#include <unordered_map>
#include "../core/AutoMoDeController.h"
#include "AutoMoDeFsmUpdator.h"

std::shared_future<int>  AutoMoDeFsmUpdator::UpdateFsmLauncherAsync(::argos::AutoMoDeController* instance, int m_unTimeStep, std::string* old_fsm) {
    return std::async(std::launch::async, &AutoMoDeFsmUpdator::UpdateFsmLauncher, this, instance, m_unTimeStep, old_fsm);
}

void set_params_on_cmd() {
    // This function will contain code to initialize the first best old FSM and its fitness
}
    
int AutoMoDeFsmUpdator::UpdateFsmLauncher(AutoMoDeController* instance, int m_unTimeStep, std::string* old_fsm) {
    int timelapsed = 500;
    if (m_unTimeStep <= 15){
        return 0;
    }
    if (is_first) {
        //clear the file
        std::ofstream file;
        file.open(file_name, std::ios_base::trunc);
        file.close();
        is_first = false;
    }
    if (m_unTimeStep % timelapsed <= 15 ) {
        instance->ExtractLogFile();
        best_old_fsm = old_fsm;
        old_fsm_params = extractFsmParams(*old_fsm);
        //new_fsm_params = Genetic_Heuristic(old_fsm_params);
        new_fsm_params = simple_sampling(old_fsm_params);
        std::string new_fsm = Params_to_fsm(*old_fsm, new_fsm_params);
         // evaluate the new fsm and write the result into a file
        //1, evaluation of the new fsm
        std::vector <std::string> result;
        result.push_back(new_fsm);
        result.push_back(std::to_string(Objective(new_fsm_params)));
        //2, write the result into a file ( add not overwrite)
        std::ofstream file;
        file.open(file_name, std::ios_base::app);
        for (std::string line : result) {
            file << line << std::endl;
        }

        if (std::stoi(result[1]) > 10 && std::stoi(result[1]) > best_old_fsm_objective) {
            best_old_fsm_objective = std::stoi(result[1]);
            printf("The new best old fsm for robot number %d is: %s\n", bot_number, new_fsm.c_str());
            instance->UpdateFSM(new_fsm);
        }
    }
     return 0;
}

std::vector<float> AutoMoDeFsmUpdator::generateNewParams(const std::vector<float>& old_params) {
    std::vector<float> new_params;
    new_params.reserve(old_params.size());

    static std::random_device rd;  // Use a random_device to seed the generator for better randomness
    static std::default_random_engine generator(rd());
    
    for (size_t i = 0; i < old_params.size(); i++) {
        std::normal_distribution<float> distribution; // Move declaration here
        float min_val, max_val;
        if (old_params[i] <= 1) {
            distribution = std::normal_distribution<float>(old_params[i], 0.1);
            min_val = 0.0f;
            max_val = 1.0f;
        } else {
            distribution = std::normal_distribution<float>(old_params[i], 1.0);
            min_val = 1.0f;
            max_val = 10.0f;
        }
        float new_value;
        do {
            new_value = distribution(generator);
            if (new_value > 1) {
                new_value = round(new_value);
            }
        } while (new_value <= min_val || new_value >= max_val); // Ensure new value is clamped between min and max

        new_params.push_back(new_value);
    }

    
    return new_params;
}

std::vector<float> crossover(const std::vector<float>& parent1, const std::vector<float>& parent2) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::vector<float> child(parent1.size());
    std::uniform_int_distribution<> distrib(0, 1);
    for (size_t i = 0; i < parent1.size(); i++) {
        child[i] = distrib(gen) ? parent1[i] : parent2[i];
    }
    return child;
}


float AutoMoDeFsmUpdator::Objective(std::vector<float> individual) {
    if (already_evaluated.find(individual) != already_evaluated.end()) {
        printf(" \n********************************Already evaluated\n, value: %.2f\n", already_evaluated[individual]);
        return already_evaluated[individual];
    }

    std::string individual_fsm = Params_to_fsm(*best_old_fsm, individual);
    std::string command = "python3 /home/ubuntu/daremo/tuning/Off-policySwarmPerformanceEstimation-master/AutoMoDeLogAnalyzer.py /home/ubuntu/daremo/tuning/Off-policySwarmPerformanceEstimation-master/iraceTools/fsm_logs/6_fsm_log_no_comments.txt -pa --newfsm-config " + individual_fsm;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Couldn't start command." << std::endl;
        return 0;
    }

    char buffer[1024];
    float wis_value = -1.0;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (sscanf(buffer, "OIS Expected average performance with the proportional reward        : %f", &wis_value) == 1) {
            break;
        }
    }
    if (wis_value == -1.0) {
        printf("*************************************Error in the command: %s\n", command.c_str());
        printf("*************************************fsm: %s\n", individual_fsm.c_str());
        return 0;
    }

    pclose(pipe);
    already_evaluated[individual] = wis_value;
    return wis_value;
}

std::vector<float> AutoMoDeFsmUpdator::Genetic_Heuristic(const std::vector<float>& fsm_params) {
    printf("Starting genetic algorithm\n");

    // Initialize population
    std::vector<std::vector<float>> population(2);
    for (auto& individual : population) {
        individual = generateNewParams(fsm_params);
    }

    // Calculate fitness for the initial population
    std::vector<float> fitness(population.size());
    for (size_t i = 0; i < population.size(); ++i) {
        fitness[i] = Objective(population[i]);
    }

    // Sort population based on fitness
    std::vector<size_t> indices(population.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&fitness](size_t i1, size_t i2) {
        return fitness[i1] > fitness[i2];
    });

    // Create new population using crossover
    std::vector<std::vector<float>> new_population(population.size());
    for (size_t i = 0; i < population.size() - 1; ++i) {
        new_population[i] = crossover(population[indices[i]], population[indices[i + 1]]);
    }
    new_population.back() = population[indices[0]]; // Retain the best individual

    // Ensure uniqueness in the population
    for (size_t i = 0; i < new_population.size(); ++i) {
        for (size_t j = i + 1; j < new_population.size(); ++j) {
            if (new_population[i] == new_population[j]) {
                new_population[j] = generateNewParams(fsm_params);
            }
        }
    }

    // Check bounds for the parameters
    for (auto& individual : new_population) {
        for (size_t j = 0; j < individual.size(); ++j) {
            if ((individual[j] > 1 && fsm_params[j] <= 1) || (individual[j] <= 1 && fsm_params[j] > 1)) {
                individual = generateNewParams(fsm_params);
            }
        }
    }

    // Replace old population with new population
    population = new_population;

    // Recalculate fitness for the new population
    for (size_t i = 0; i < population.size(); ++i) {
        fitness[i] = Objective(population[i]);
    }

    // Sort population based on new fitness values
    std::sort(indices.begin(), indices.end(), [&fitness](size_t i1, size_t i2) {
        return fitness[i1] > fitness[i2];
    });

    // Check if the best individual improves the objective function value
    if (fitness[indices[0]] > best_old_fsm_objective) {
        best_old_fsm_objective = fitness[indices[0]];
        //printf("with objective function value: %.2f\n", fitness[indices[0]]);
        return population[indices[0]];
    } 
    else {
        //printf("No improvement in the objective function value\n");
        return fsm_params;
    }
}

std::string AutoMoDeFsmUpdator::Params_to_fsm(const std::string& old_fsm, const std::vector<float>& new_params) {
    // Create a new FSM from the old FSM
    std::stringstream ss2(old_fsm);
    std::string token2;
    std::string new_fsm = old_fsm;
    size_t i = 0;
    size_t pos = 0;

    // Replace parameters in the FSM
    while (i < new_params.size() && std::getline(ss2, token2, ' ')) {
        if (token2.find("p") != std::string::npos) {
            if (std::getline(ss2, token2, ' ')) {
                // Find the position of the token starting from the last position
                pos = new_fsm.find(token2, pos);
                if (pos != std::string::npos) {
                    std::stringstream new_param_ss;
                    new_param_ss << new_params[i];
                    new_fsm.replace(pos, token2.length(), new_param_ss.str());
                    // Move the position forward for the next search to avoid replacing the same token again
                    pos += new_param_ss.str().length();
                    i++;
                }
            }
        }
    }

    // print out new params and new fsm
    //printf("\n\n\n#************************************************************################################ the new fsm generated is: %s\n", new_fsm.c_str());
    //printf("\n\n\n########## the new params generated are: ");
    /* for (float gene : new_params) {
        printf("%.2f ", gene);
    } */

    return new_fsm;
}



std::vector<float> AutoMoDeFsmUpdator::extractFsmParams(const std::string& fsm) {
    std::vector<float> fsm_params;
    std::stringstream ss(fsm);
    std::string token;
    
    while (ss >> token) { // Use the stringstream directly
        if (token.find("p") != std::string::npos) {
            if (ss >> token) { // Try to get the next token
                try {
                    fsm_params.push_back(std::stof(token));
                } catch (const std::invalid_argument& e) {
                    // Handle the case where stof fails
                    std::cerr << "Invalid parameter: " << token << std::endl;
                } catch (const std::out_of_range& e) {
                    // Handle the case where stof result is out of range
                    std::cerr << "\n####### WARNING ###### Parameter out of range: " << token << std::endl;
                }
            }
        }
    }

    return fsm_params;
}

std::vector<float> AutoMoDeFsmUpdator::simple_sampling(const std::vector<float>& old_params) {
    std::vector<float> new_params;
    new_params.reserve(old_params.size());


    static std::random_device rd;  // Use a random_device to seed the generator for better randomness
    static std::default_random_engine generator(rd());
    
    for (size_t i = 0; i < old_params.size(); i++) {
        std::normal_distribution<float> distribution; // Move declaration here
        float min_val, max_val;
        if (old_params[i] <= 1) {
            distribution = std::normal_distribution<float>(old_params[i], 0.1);
            min_val = 0.0f;
            max_val = 1.0f;
        } else {
            distribution = std::normal_distribution<float>(old_params[i], 1.0);
            min_val = 1.0f;
            max_val = 10.0f;
        }
        float new_value;
        do {
            new_value = distribution(generator);
            if (new_value > 1) {
                new_value = round(new_value);
            }
        } while (new_value <= min_val || new_value >= max_val); // Ensure new value is clamped between min and max

        new_params.push_back(new_value);
    }
    return new_params;
}   