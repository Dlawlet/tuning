#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <map>
#include <thread>
#include <future>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <unordered_map>
#include <sys/file.h>
#include <unistd.h> // for sleep function
#include <fcntl.h>  // for file control options
#include <mutex>
#include <limits>

using namespace std;

// Function declarations
std::vector<std::string> run_script(const std::string &central_fsm);
std::vector<float> extractFsmParams(const std::string &fsm);
std::vector<float> simple_sampling(const std::vector<float> &old_params);
std::string Params_to_fsm(const std::string &old_fsm, const std::vector<float> &new_params);
float Objective(const std::vector<float> &new_params);
bool is_file_ready(const std::string &filename);

// Global variables:
std::string central_fsm = "--nstates 4 --s0 3 --n0 1 --n0x0 0 --c0x0 4 --p0x0 5 --w0x0 3.5 --s1 0 --rwm1 1 --n1 2 --n1x0 1 --c1x0 0 --p1x0 0.55 --n1x1 0 --c1x1 2 --p1x1 0.43 --s2 2 --n2 2 --n2x0 0 --c2x0 2 --p2x0 0.09 --n2x1 2 --c2x1 2 --p2x1 1 --s3 3 --n3 4 --n3x0 0 --c3x0 2 --p3x0 0.43 --n3x1 0 --c3x1 4 --p3x1 7 --w3x1 11.23 --n3x2 1 --c3x2 5 --p3x2 0.48 --n3x3 0 --c3x3 4 --p3x3 2 --w3x3 1.02";
std::vector<float> central_fsm_params;
//init time counter 
auto start = std::chrono::high_resolution_clock::now();


int best_iteration = 0;
float max_objective = 0.0f;
std::map<std::vector<float>, float> already_evaluated = {};
int bot_number = 20;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <iterations>" << std::endl;
        return 1;
    }

    int iterations;
    try {
        iterations = std::stoi(argv[1]);
    } catch (const std::invalid_argument &e) {
        std::cerr << "Invalid argument: " << argv[1] << std::endl;
        return 1;
    }

    for (int it = 0; it < iterations; it++) {
        std::cout << "#############################################################################################" << std::endl;
        std::cout << "Iteration " << it + 1 << std::endl;

        
        central_fsm_params = extractFsmParams(central_fsm);

        bool new_max = false;

        // Launch bot_number scripts in parallel
        std::vector<std::future<std::vector<std::string>>> futures;
        for (int i = 0; i < bot_number; i++) {
            futures.push_back(std::async(std::launch::async, run_script, std::ref(central_fsm)));
        }

        // Gather results into a vector
        // Each future will return a vector of 2 values: the fsm and its objective
        std::vector<std::vector<std::string>> results;
        for (auto &fut : futures) {
            results.push_back(fut.get());
        }

        // Find the element in result with the highest objective
        for (const auto &result : results) {
            float objective = std::stof(result[1]);
            if (objective > max_objective) {
                max_objective = objective;
                central_fsm = result[0];
                best_iteration = it+1;
                new_max = true;

            }
        }

        if (new_max) {
            std::cout << "New max objective: " << max_objective << std::endl;
            std::cout << central_fsm << std::endl;
        } else {
            std::cout << "Current max objective: " << max_objective << std::endl;
        }

        // free memory and cpu ressources for the next iteration
        central_fsm_params.clear();

        // Explicitly deallocating resources
        futures.clear();   // Clear futures to free any associated resources
        results.clear();   // Clear results to free memory used by the vector

        // Optional: Add a sleep to give some idle time to the CPU, especially if the iterations are intensive
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    } 

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Total time: " << elapsed.count() << "s" << std::endl;
    

    // at the end of the iterations, print the best FSM and its objective
    std::cout << "#############################################################################################" << std::endl;
    std::cout << "Best FSM: " << central_fsm << std::endl;
    std::cout << "Best objective: " << max_objective << std::endl;
    std::cout << "Best iteration: " << best_iteration << std::endl;

    return 0;
}

std::vector<std::string> run_script(const std::string &central_fsm) {
    // print out the extracted parameters
    //std::cout << "Extracted parameters: " << "length: " << central_fsm_params.size() << std::endl;
    /* for (const auto &param : central_fsm_params) {
        std::cout << param << " ";
    } */
    //std::cout << std::endl;
    std::vector<float> new_fsm_params;
    new_fsm_params = simple_sampling(central_fsm_params);
    // print out the new params 
    //std::cout << "New parameters: ";
    /* for (const auto &param : new_fsm_params) {
        std::cout << param << " ";
    } */
    //std::cout << std::endl;
    std::string new_fsm = Params_to_fsm(central_fsm, new_fsm_params);
    std::vector<std::string> result;
    result.push_back(new_fsm);
    result.push_back(std::to_string(Objective(new_fsm_params)));
    new_fsm_params.clear();
    return result;
}

std::vector<float> simple_sampling(const std::vector<float> &old_params) {
    std::vector<float> new_params;
    new_params.reserve(old_params.size());
    static std::random_device rd;
    static std::default_random_engine generator(rd());

    for (size_t i = 0; i < old_params.size(); i++) {
        std::normal_distribution<float> distribution;
        float min_val, max_val;
        if (old_params[i] < 1) {
            distribution = std::normal_distribution<float>(old_params[i], 0.1);
            min_val = 0.0f;
            max_val = 1.0f;
        } else if (old_params[i] == 1) {
            new_params.push_back(1);
            continue;
        } else {
            distribution = std::normal_distribution<float>(old_params[i], 1.0);
            min_val = 1.7f;
            max_val = 10.0f;
        }

        float new_value;
        do {
            new_value = distribution(generator);
            if (new_value > 1) {
                new_value = round(new_value);
            }
        } while (new_value <= min_val || new_value >= max_val);

        new_params.push_back(new_value);
    }
    return new_params;
}

std::vector<float> extractFsmParams(const std::string &fsm) {
    std::vector<float> fsm_params;
    std::stringstream ss(fsm);
    std::string token;

    while (ss >> token) {
        if (token.find("p") != std::string::npos) {
            if (ss >> token) {
                try {
                    fsm_params.push_back(std::stof(token));
                } catch (const std::invalid_argument &e) {
                    std::cerr << "Invalid parameter: " << token << std::endl;
                } catch (const std::out_of_range &e) {
                    std::cerr << "Parameter out of range: " << token << std::endl;
                }
            }
        }
    }
    return fsm_params;
}

float Objective(const std::vector<float> &individual) {
    if (already_evaluated.find(individual) != already_evaluated.end()) {
        std::cout << "Already evaluated, value:" << already_evaluated[individual] << std::endl;
        return already_evaluated[individual];
    }

    std::string individual_fsm = Params_to_fsm(central_fsm, individual);
    std::string log_file_path = "./Glogfile.txt";
    std::string command = "python3 /home/ubuntu/daremo/tuning/Off-policySwarmPerformanceEstimation-master/AutoMoDeLogAnalyzer.py " + log_file_path + " -pa --newfsm-config " + individual_fsm;

    while (!is_file_ready(log_file_path)) {
        std::cout << "Waiting for Glogfile.txt to be available..." << std::endl;
        sleep(1);
    }

    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Couldn't start command." << std::endl;
        return 0;
    }

    char buffer[1024];
    float wis_value = -1.0;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (sscanf(buffer, "OIS Expected average performance with the proportional reward : %f", &wis_value) == 1) {
            break;
        }
    }

    pclose(pipe);

    if (wis_value == -1.0) {
        std::cerr << "Error in the command: " << command << std::endl;
        return 0;
    }

    //std::cout << "Objective function value: " << wis_value << std::endl;
    already_evaluated[individual] = wis_value;
    return wis_value;
}

bool is_file_ready(const std::string &filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cout << "File does not exist" << std::endl;
        return false;
    }

    int lock_result = flock(fd, LOCK_EX | LOCK_NB);
    if (lock_result == 0) {
        flock(fd, LOCK_UN);
        close(fd);
        return true;
    } else {
        close(fd);
        std::cout << "File is locked" << std::endl;
        return false;
    }
}

std::string Params_to_fsm(const std::string &old_fsm, const std::vector<float> &new_params) {
    std::stringstream ss2(old_fsm);
    std::string token2;
    std::string new_fsm = old_fsm;
    size_t i = 0;
    size_t pos = 0;

    // First, count the number of probability values in old_fsm
    int prob_count = 0;
    std::stringstream temp_ss(old_fsm);
    while (temp_ss >> token2) {
        if (token2.find("--p") != std::string::npos) {
            prob_count++;
        }
    }

    // Check if the number of new parameters matches the number of probability values
    /* if (new_params.size() != prob_count) {
        std::cerr << "Error: Number of parameters does not match the number of probability values." << std::endl;
        std::cerr << "Number of parameters: " << new_params.size() << ", Number of probability values: " << prob_count << std::endl;
        // Halt the program
        exit(1);
    } */

    // Replace the probability values with new_params
    while (i < new_params.size() && std::getline(ss2, token2, ' ')) {
        if (token2.find("--p") != std::string::npos) {
            std::string param_token;
            if (std::getline(ss2, param_token, ' ')) {
                size_t start_pos = new_fsm.find(token2, pos);
                if (start_pos != std::string::npos) {
                    // Ensure that we only replace the value following the "--p" token
                    size_t value_start_pos = start_pos + token2.length() + 1;
                    size_t value_end_pos = new_fsm.find(' ', value_start_pos);
                    if (value_end_pos == std::string::npos) {
                        value_end_pos = new_fsm.length();
                    }
                    std::string current_value = new_fsm.substr(value_start_pos, value_end_pos - value_start_pos);
                    
                    // Only replace if the current value matches the old value in the token
                    if (current_value == param_token) {
                        std::stringstream new_param_ss;
                        new_param_ss << new_params[i];
                        new_fsm.replace(value_start_pos, current_value.length(), new_param_ss.str());
                        pos = value_start_pos + new_param_ss.str().length();
                        i++;
                    }
                }
            }
        }
    }

    // Check if the only differences are the probability values
    std::stringstream old_fsm_ss(old_fsm);
    std::stringstream new_fsm_ss(new_fsm);
    std::string old_token, new_token;
    std::vector<std::string> differences;

    /* while (old_fsm_ss >> old_token && new_fsm_ss >> new_token) {
        if (old_token != new_token) {
            differences.push_back("Old: " + old_token + " New: " + new_token);
        }
    }

    if (!differences.empty()) {
        std::cerr << "Error: Differences found between old and new FSM other than probability values." << std::endl;
        std::cerr << "Number of differences: " << differences.size() << std::endl;
        std::cerr << "Differences: " << std::endl;
        for (const auto &diff : differences) {
            std::cerr << diff << std::endl;
        }
        std::cerr << "Old FSM: " << old_fsm << std::endl;
        std::cerr << "New FSM: " << new_fsm << std::endl;
        std::cerr << "Parameters List: ";
        for (const auto &param : new_params) {
            std::cerr << param << " ";
        }
        std::cerr << std::endl;
    } */

    return new_fsm;
}

