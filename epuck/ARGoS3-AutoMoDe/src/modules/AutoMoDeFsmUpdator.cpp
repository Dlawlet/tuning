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

    void AutoMoDeFsmUpdator::UpdateFsmLauncher(AutoMoDeController* instance, int m_unTimeStep){
        if (m_unTimeStep % 100 == 0){
            //Update the FSM
            instance->UpdateFSM("--nstates 3 --s0 0 --rwm0 2 --n0 2 --n0x0 0 --c0x0 0 --p0x0 0.47 --n0x1 0 --c0x1 0 --p0x1 0.16 --s1 1 --n1 2 --n1x0 1 --c1x0 2 --p1x0 0.73 --n1x1 1 --c1x1 1 --p1x1 0.87 --s2 4 --att2 3.67 --n2 4 --n2x0 1 --c2x0 3 --p2x0 2 --w2x0 2.75 --n2x1 1 --c2x1 3 --p2x1 3 --w2x1 0.36 --n2x2 1 --c2x2 4 --p2x2 1 --w2x2 11.97 --n2x3 1 --c2x3 0 --p2x3 0.18");
        }
        else if (m_unTimeStep % 150 == 0){
            //Update the FSM
            instance->UpdateFSM("--nstates 1 --s0 1"); 
        }
    }

    