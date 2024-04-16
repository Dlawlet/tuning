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

namespace argos {
    class AutoMoDeController;
}



#ifndef AUTOMODE_FSM_UPDATOR_H
#define AUTOMODE_FSM_UPDATOR_H

class AutoMoDeFsmUpdator {
    public:
        void UpdateFsmLauncher(::argos::AutoMoDeController* instance, int m_unTimeStep, std::string* old_fsm);
        std::string New_fsm_generator(std::string *current_fsm);
        std::vector<float> generateNewParams(const std::vector<float>& old_params);
        std::string HeuristicOptimization(::argos::AutoMoDeController *instance, std::string *old_fsm);
        std::string *best_old_fsm;
        float old_reward = 0;
        float default_rate_reward = 0 ;
        std::vector<std::vector<float>> used_values = {};

        // the following is a hard coded list of params changeable this need to be past as an argument when launching the program 
        std::vector<float> fsm_params = {10,0.41,0.29,0.05,0.4,0.33};

};

#endif