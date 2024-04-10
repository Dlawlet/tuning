/*
 * @file <src/modules/AutoMoFsmUpdator.h>
 *
 * @author Florian Noussa-Yao - <fnoussay@ulb.be>
 *
 * @package ARGoS3-AutoMoDe
 *
 * @license MIT License
 */

#include "../core/AutoMoDeController.h"

#ifndef AUTOMODE_FSM_UPDATOR_H
#define AUTOMODE_FSM_UPDATOR_H

class AutoMoDeFsmUpdator {
        public:
            void UpdateFsmLauncher(AutoMoDeController* instance, int m_unTimeStep);
    };

#endif