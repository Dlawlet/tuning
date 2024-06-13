/*
 * @file <src/modules/AutoMoFsmUpdator.h>
 *
 * @author Florian Noussa-Yao - <fnoussay@ulb.be>
 *
 * @package ARGoS3-AutoMoDe
 *
 * @license MIT License
 */

#include <sstream> // Include the <sstream> header for stringstream
#include <vector> // Include the <vector> header for vector
#include <map>
#include <future>

namespace argos {
    class AutoMoDeController;
}



#ifndef AUTOMODE_FSM_UPDATOR_H
#define AUTOMODE_FSM_UPDATOR_H

class AutoMoDeFsmUpdator {
    public:
        int UpdateFsmLauncher(::argos::AutoMoDeController* instance, int m_unTimeStep, std::string* old_fsm);
        std::string New_fsm_generator(std::string *current_fsm);
        std::vector<float> generateNewParams(const std::vector<float>& old_params);
        std::string HeuristicOptimization(::argos::AutoMoDeController *instance, std::string *old_fsm);
        std::vector<float> extractFsmParams(std::string *fsm);
        float Objective(std::vector<float> individual);
        std::vector<float>  Genetic_Heuristic(std::string *old_fsm, std::vector<float> fsm_params);
        std::string Params_to_fsm(std::string *old_fsm, std::vector<float> new_params);
         std::shared_future<int>  UpdateFsmLauncherAsync(::argos::AutoMoDeController* instance, int m_unTimeStep, std::string* old_fsm);

        std::string *best_old_fsm;
        float best_old_fsm_objective = 24.0 ; 
        float old_reward = 0;
        float default_rate_reward = 0 ;
        std::vector<std::vector<float>> used_values = {};

        std::map<std::vector<float>, float> already_evaluated = {};

        // the following is a hard coded list of params changeable this need to be past as an argument when launching the program 
        std::vector<float> fsm_params;

};

#endif