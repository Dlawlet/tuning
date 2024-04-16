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


    void AutoMoDeFsmUpdator::UpdateFsmLauncher(AutoMoDeController* instance, int m_unTimeStep, std::string* old_fsm){
        // Every 100 time steps, we will run an heuristic optimization process to update the fsm.
        // The optimization process will use as objective function the number of time steps the robot has been in the state 0.
        // The heuristic optimization process will change the parameters of the fsm i other to maximize the objective function value.
        int timelapsed = 30;
        float rate_reward;
        if (m_unTimeStep % timelapsed == 0){
            float delta_reward = instance->NYF_reward - old_reward;
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
            }
        }

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
        fsm_params = new_params;

        //print(new_params) on single line 
        printf("\nnew_params\n");
        for (size_t i = 0; i < new_params.size(); i++) {
            printf("%.2f , ", new_params[i]);
        }

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


    std::string AutoMoDeFsmUpdator::HeuristicOptimization(AutoMoDeController* instance, std::string* old_fsm){
        std::string new_fsm = "";
        // The optimization process will use as objective function the number of time steps the robot has been in the state 0.


        return new_fsm;
    }

    