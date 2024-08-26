#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <sstream>
#include <future>
#include <cstdio>
#include <cmath> 
#include <string>
#include <unordered_map>
#include <sys/file.h>
#include <fstream>
#include <unistd.h>  // for sleep function
#include <fcntl.h>   // for file control options
#include <mutex>
#include <limits>

#include "../core/AutoMoDeController.h"
#include "AutoMoDeFsmUpdator.h"

std::shared_future<int>  AutoMoDeFsmUpdator::UpdateFsmLauncherAsync(::argos::AutoMoDeController* instance, int m_unTimeStep, std::string* old_fsm) {
    return std::async(std::launch::async, &AutoMoDeFsmUpdator::UpdateFsmLauncher, this, instance, m_unTimeStep, old_fsm);
}

void set_params_on_cmd() {
    // This function will contain code to initialize the first best old FSM and its fitness
}

int AutoMoDeFsmUpdator::UpdateFsmLauncher(AutoMoDeController* instance, int m_unTimeStep, std::string* old_fsm) {
    int timelapsed = 1800;
    int SwarmSize = 20 ;
    if (is_first) {
        // Clear the file
        std::ofstream file;
        file.open(file_name, std::ios_base::trunc);
        file.close();
        is_first = false;

    }
    //printf("The time step is: %d\n", m_unTimeStep);
    // /!\ This is a temporary fix to avoid the 1over2 skipping in time steps
    // Define the interval around timu
    int interval = 4;  // The interval range around timu
    int timu = timelapsed - 10;

    // Check if m_unTimeStep is within the interval around timu
    if (m_unTimeStep > 10 && m_unTimeStep % timu >= timu - interval && m_unTimeStep % timu <= timu + interval) {
        if (!hasExtracted) {
            instance->ExtractLogFile();
            hasExtracted = true;
        }
    } else {
        hasExtracted = false;  // Reset the flag when outside the interval
    }

    if (m_unTimeStep > 10 && m_unTimeStep % timelapsed >= timelapsed - interval && m_unTimeStep % timelapsed <= timelapsed + interval) {
        if (!hasEvaluated){
            best_old_fsm = old_fsm;
            old_fsm_params = extractFsmParams(*old_fsm);
            new_fsm_params = simple_sampling(old_fsm_params);
            std::string new_fsm = Params_to_fsm(*old_fsm, new_fsm_params);
            std::vector <std::string> result;
            result.push_back(new_fsm);
            result.push_back(std::to_string(Objective(new_fsm_params))); // Ensure Objective is thread-safe
            std::cout <<"almost done with the estimated fsm" <<  bot_number << std::endl;
            // check if file is ready and not locked by another process then write to it
            while (!is_file_ready(file_name)) {
                std::cout << "Waiting for estimated_fsms.txt to be available..." <<  bot_number << std::endl;
                sleep(1); // Wait for 1 second before trying again
            }
            std::ofstream file;
            file.open(file_name, std::ios_base::app);
            for (const std::string& line : result) {
                file << line << std::endl;
            }
            file.close();
            std::cout << "finished writing to the file" <<  bot_number << std::endl;
    /*         
            std::ofstream file;
            file.open(file_name, std::ios_base::app);
            for (const std::string& line : result) {
                file << line << std::endl;
            }
            file.close();
            std::cout << "finished writing to the file" << std::endl; */

            /* int objective;
            try {
                objective = std::stoi(result[1]);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid argument: " << e.what() << std::endl;
            } catch (const std::out_of_range& e) {
                std::cerr << "Out of range: " << e.what() << std::endl;
                // then print result[1] to see what is causing the error and set objective to infinity
                std::cerr << "Objective value: " << result[1] << std::endl;
                objective = std::numeric_limits<int>::max();
            } */
            /* if (objective > 0 && objective > best_old_fsm_objective) {
                    //std::lock_guard<std::mutex> lock(m_Mutex); // Lock the mutex to ensure thread safety
                    best_old_fsm_objective = objective;
                    //printf("The new best old fsm for robot number %d is: %s\n", bot_number, new_fsm.c_str());
                    instance->UpdateFSM(new_fsm);
                } */
            hasEvaluated = true;
            return 0;
        }
    } else {
        if (hasEvaluated && !hasUpdated) {
        hasEvaluated = false;

        // Check the number of lines in the file
        std::ifstream estimated_file(file_name);
        if (!estimated_file.is_open()) {
            std::cerr << "Failed to open the file: " << file_name << std::endl;
            return 0; 
        }

        std::string line;
        int line_count = 0;
        while (std::getline(estimated_file, line)) {
            line_count++;
        }
        estimated_file.close();
        //std::cout << "Estimated file line count: " << line_count << std::endl;

        while (line_count != 2 * SwarmSize) {
            sleep(1);
            estimated_file.open(file_name);
            if (!estimated_file.is_open()) {
                std::cerr << "Failed to open the file: " << file_name << std::endl;
                return 0; 
            }

            line_count = 0;
            while (std::getline(estimated_file, line)) {
                line_count++;
            }
            estimated_file.close();
            //std::cout << "Estimated file line count: " << line_count << " Waiting for: " << 2 * SwarmSize << std::endl;
        }

        estimated_file.open(file_name);
        if (!estimated_file.is_open()) {
            std::cerr << "Failed to open the file: " << file_name << std::endl;
            return 0; 
        }

        std::string best_fsm;
        float best_value = -std::numeric_limits<float>::infinity(); // Start with a very low value

        std::string current_fsm;
        bool fsm_next = false; // Flag to indicate the next line should be read as FSM

        while (std::getline(estimated_file, line)) {
            if (line.find("--nstates") != std::string::npos) {
                current_fsm = line;
                fsm_next = true;
            } else if (fsm_next) {
                std::stringstream ss(line);
                float value;
                if (ss >> value) {
                    if (value > best_value) {
                        best_value = value;
                        best_fsm = current_fsm;
                    }
                    //std::cout << "FSM: " << current_fsm << " Value: " << value << std::endl;
                } else {
                    //std::cerr << "Failed to convert to float: " << line << std::endl;
                }
                fsm_next = false;
            } else {
                std::cerr << "Unexpected line format: " << line << std::endl;
            }
        }
        //std::cout << "The best FSM is: " << best_fsm << " with value: " << best_value << std::endl;
        std::cout << "The best FSM  with value: " << best_value << std::endl;
        estimated_file.close();

        // Open the objectiveval.txt file and read the value corresponding to obj value
        std::ifstream obj_file("objectiveval.txt");
        if (!obj_file.is_open()) {
            std::cerr << "Failed to open the file: objectiveval.txt" << std::endl;
            return 0; 
        }
 
       // get the value at the m_unTimeStep line of the file 
        int actual_value = 0;
        int line_number = 0;
        while (std::getline(obj_file, line)) {
            if (line_number == m_unTimeStep) {
                std::stringstream ss(line); // a line is made of two values separated by a space we will get the second value
                std::string token;
                std::getline(ss, token, ' ');
                std::getline(ss, token, ' ');
                actual_value = std::stoi(token);    
                std::cout << "The actual value is: " << actual_value << std::endl;
                break;
            }
            line_number++;
        }

        if (best_value > actual_value+10) {
            //instance->UpdateFSM(best_fsm); // Assuming instance is defined elsewhere
        }
        
        hasUpdated = true;
    }
        else {
        hasUpdated = false;
        }
        hasEvaluated = false;
    }
    // then the folliwng code will be apply once on the next time step
    /* if  (hasEvaluated && !hasUpdated){
        // now we will retreive the best fsm from the all the evaluated ones, and update the controller with it 
            // 1. verify if the Estimated_fms.txt file contains 2x the number of robot as the number of lines
            // 2. read the file and get the fsm (odd line starting with 1) associated with the highest estimated value (even line starting with 2)
            // 3. apply the fsm to the controller

            std::ifstream estimated_file(file_name);
            std::string line;
            int line_count = 0;
            while (std::getline(estimated_file, line)) {
                line_count++;
            }
            estimated_file.close();
            std::cout << "estimated file line count: " << line_count << std::endl;

            while (line_count != 2*SwarmSize) {
                sleep(1);
                estimated_file.open(file_name);
                line_count = 0;
                while (std::getline(estimated_file, line)) {
                    line_count++;
                }
                estimated_file.close();
                std::cout << "estimated file line count: " << line_count << " Waiting : " << 2*SwarmSize << std::endl;
            }

            estimated_file.open(file_name);
            std::string best_fsm;
            float best_value = 0;
            while (std::getline(estimated_file, line)) {
                if (line.find("Estimated") != std::string::npos) { // 
                    std::string fsm;
                    float value;
                    std::getline(estimated_file, fsm);
                    std::getline(estimated_file, line);
                    value = std::stof(line);
                    if (value > best_value) {
                        best_value = value;
                        best_fsm = fsm;
                    }
                    std::cout << "fsm: " << fsm << " value: " << value << std::endl;
                }
                std::cout << "Not in the if statement" << std::endl;
            }
            std::cout << "The best fsm is: " << best_fsm << " with value: " << best_value << std::endl;
            estimated_file.close();
            if (best_value > 10) {
                instance->UpdateFSM(best_fsm);
            }
            hasUpdated = true;
    } else {
        hasUpdated = false;
    } */
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


bool AutoMoDeFsmUpdator::is_file_ready(const std::string& filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    //int fd = access(filename.c_str(),F_OK) ;
    if (fd == -1) {
        std::cout << "File do not exist" << std::endl ;
        return false; // File does not exist
    }
    
    int lock_result = flock(fd, LOCK_EX | LOCK_NB);
    if (lock_result == 0) {
        flock(fd, LOCK_UN); // Unlock if we managed to lock it
        close(fd);
        return true; // File is not locked by another process
    } else {
        close(fd);
        std::cout << "File is locked " << std::endl ;
        return false; // File is locked by another process
    }
}
float AutoMoDeFsmUpdator::Objective(std::vector<float> individual) {
    if (already_evaluated.find(individual) != already_evaluated.end()) {
        //printf(" \n********************************Already evaluated\n, value: %.2f\n", already_evaluated[individual]);
        return already_evaluated[individual];
    }

    std::string individual_fsm = Params_to_fsm(*best_old_fsm, individual);
    //std::string command = "python3 /home/students/daremo/tuning/Off-policySwarmPerformanceEstimation-master/AutoMoDeLogAnalyzer.py /home/students/daremo/tuning/Off-policySwarmPerformanceEstimation-master/iraceTools/fsm_logs/6_fsm_log_no_comments.txt -pa --newfsm-config " + individual_fsm;
    std::string log_file_path = "/home/students/daremo/tuning/coverage-auto/coverage-auto/mission-folder/Glogfile.txt";
    std::string command = "python3 /home/students/daremo/tuning/Off-policySwarmPerformanceEstimation-master/AutoMoDeLogAnalyzer.py "+ log_file_path + " -pa --newfsm-config " + individual_fsm;

    // Check if the glogfile exists and isn't locked by another process
    while (!is_file_ready(log_file_path)) {
        std::cout << "Waiting for Glogfile.txt to be available..." << std::endl;
        sleep(1); // Wait for 1 second before trying again
    }
    
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
        //printf("*************************************fsm: %s\n", individual_fsm.c_str());
        return 0;
    }

    pclose(pipe);
    std::cout << "Objective function value: " << wis_value << std::endl;
    already_evaluated[individual] = wis_value;
    return wis_value;
}

std::vector<float> AutoMoDeFsmUpdator::Genetic_Heuristic(const std::vector<float>& fsm_params) {
    //printf("Starting genetic algorithm\n");

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
        ////printf("with objective function value: %.2f\n", fitness[indices[0]]);
        return population[indices[0]];
    } 
    else {
        ////printf("No improvement in the objective function value\n");
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
    ////printf("\n\n\n#************************************************************################################ the new fsm generated is: %s\n", new_fsm.c_str());
    ////printf("\n\n\n########## the new params generated are: ");
    /* for (float gene : new_params) {
        //printf("%.2f ", gene);
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
        if (old_params[i] < 1) {
            distribution = std::normal_distribution<float>(old_params[i], 0.1);
            min_val = 0.0f;
            max_val = 1.0f;
        }
        // if old params - 1 just keep the same value
        else if (old_params[i] == 1) {
            new_params.push_back(1);
            continue;
        } 
        else {
            distribution = std::normal_distribution<float>(old_params[i], 1.0);
            min_val = 2.0f;
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
