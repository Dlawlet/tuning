#include "AutoMoDeController.h"
#include "../modules/AutoMoDeFsmUpdator.h"
#include <future>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <iostream>
#include <string>

#include <argos3/core/simulator/simulator.h>
#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/loop_functions.h>

//#include <argos3/demiurge/loop-functions/CoreLoopFunctions.h>


namespace argos {

    /****************************************/
    /****************************************/

    AutoMoDeController::AutoMoDeController() 
     {
        m_pcRobotState = new ReferenceModel1Dot2();
        m_unTimeStep = 0;
        m_strFsmConfiguration = "";
        m_bMaintainHistory = true;
        m_bPrintReadableFsm = false;
        m_strHistoryFolder = "./";
        m_bFiniteStateMachineGiven = false;
        //m_pcLoopFunctions = nullptr;
        NYF_old_neighbors_count = 0;
        NYF_reward = 0;
    }

    /****************************************/
    /****************************************/

    AutoMoDeController::~AutoMoDeController() {
       delete m_pcRobotState;
		if (m_strFsmConfiguration.compare("") != 0) {
			delete m_pcFsmBuilder;
		}
	}
    

    /****************************************/
    /****************************************/

    void AutoMoDeController::Init(TConfigurationNode& t_node) {
        // Parsing parameters
        try {
            GetNodeAttributeOrDefault(t_node, "fsm-config", m_strFsmConfiguration, m_strFsmConfiguration);
            GetNodeAttributeOrDefault(t_node, "history", m_bMaintainHistory, m_bMaintainHistory);
            GetNodeAttributeOrDefault(t_node, "hist-folder", m_strHistoryFolder, m_strHistoryFolder);
            GetNodeAttributeOrDefault(t_node, "readable", m_bPrintReadableFsm, m_bPrintReadableFsm);
        } catch (CARGoSException& ex) {
            THROW_ARGOSEXCEPTION_NESTED("Error parsing <params>", ex);
        }

        m_unRobotID = rand() % 1000;
        m_pcRobotState->SetRobotIdentifier(m_unRobotID);

        if (m_strFsmConfiguration.compare("") != 0 && !m_bFiniteStateMachineGiven) {
            m_pcFsmBuilder = new AutoMoDeFsmBuilder();
            SetFiniteStateMachine(m_pcFsmBuilder->BuildFiniteStateMachine(m_strFsmConfiguration));
            if (m_bMaintainHistory) {
                m_pcFiniteStateMachine->SetHistoryFolder(m_strHistoryFolder);
                m_pcFiniteStateMachine->MaintainHistory();
            }
            if (m_bPrintReadableFsm) {
                //std::cout << "Finite State Machine description: " << std::endl;
                //std::cout << m_pcFiniteStateMachine->GetReadableFormat() << std::endl;
            }
        } else {
            LOGERR << "Warning: No finite state machine configuration found in .argos" << std::endl;
        }

        try {
            m_pcProximitySensor = GetSensor<CCI_EPuckProximitySensor>("epuck_proximity");
            m_pcLightSensor = GetSensor<CCI_EPuckLightSensor>("epuck_light");
            m_pcGroundSensor = GetSensor<CCI_EPuckGroundSensor>("epuck_ground");
            m_pcRabSensor = GetSensor<CCI_EPuckRangeAndBearingSensor>("epuck_range_and_bearing");
            m_pcCameraSensor = GetSensor<CCI_EPuckOmnidirectionalCameraSensor>("epuck_omnidirectional_camera");
        } catch (CARGoSException ex) {
            LOGERR << "Error while initializing a Sensor!\n";
        }

        try {
            m_pcWheelsActuator = GetActuator<CCI_EPuckWheelsActuator>("epuck_wheels");
            m_pcRabActuator = GetActuator<CCI_EPuckRangeAndBearingActuator>("epuck_range_and_bearing");
            m_pcLEDsActuator = GetActuator<CCI_EPuckRGBLEDsActuator>("epuck_rgb_leds");
        } catch (CARGoSException ex) {
            LOGERR << "Error while initializing an Actuator!\n";
        }

        /*
		 * Starts actuation.
		 */
        InitializeActuation();

        //delete all output files in current folder
        std::string command = "rm -f output_history_*.txt fsm_history_*.txt objectiveval*.txt estimated_fsms.txt Glogfile.txt";
        system(command.c_str());

    }

    /****************************************/
    /****************************************/

    void AutoMoDeController::ControlStep() {
		/*
		 * 1. Update RobotDAO
		 */
        //printf("new control step for robot id: %d\n", m_unRobotID);
		if(m_pcRabSensor != NULL){
            //printf("rab sensor is not null\n");
			const CCI_EPuckRangeAndBearingSensor::TPackets& packets = m_pcRabSensor->GetPackets();
			//m_pcRobotState->SetNumberNeighbors(packets.size());
			m_pcRobotState->SetRangeAndBearingMessages(packets);

		}
		if (m_pcGroundSensor != NULL) {
            //printf("ground sensor is not null\n");
			const CCI_EPuckGroundSensor::SReadings& readings = m_pcGroundSensor->GetReadings();
			m_pcRobotState->SetGroundInput(readings);
		}
		if (m_pcLightSensor != NULL) {
            //printf("light sensor is not null\n");
			const CCI_EPuckLightSensor::TReadings& readings = m_pcLightSensor->GetReadings();
			m_pcRobotState->SetLightInput(readings);
		}
		if (m_pcProximitySensor != NULL) {
            //printf("proximity sensor is not null\n");
			const CCI_EPuckProximitySensor::TReadings& readings = m_pcProximitySensor->GetReadings();
			m_pcRobotState->SetProximityInput(readings);
		}

		/*
		 * 2. Execute step of FSM
		 */
        //printf("executing step of FSM\n");
		m_pcFiniteStateMachine->ControlStep();

		/*
		 * 3. Update Actuators
		 */
        //printf("updating actuators\n");
		if (m_pcWheelsActuator != NULL) {
            //printf("wheels actuator is not null\n");
			m_pcWheelsActuator->SetLinearVelocity(m_pcRobotState->GetLeftWheelVelocity(),m_pcRobotState->GetRightWheelVelocity());
		}

		/*
		 * 4. Update variables and sensors
		 */
        //printf("updating variables and sensors\n");
		if (m_pcRabSensor != NULL) {
            //printf("rab sensor is not null\n");
			m_pcRabSensor->ClearPackets();
		}
		m_unTimeStep++;

        /* if (m_pcLoopFunctions) {
            Real fObjectiveFunction = m_pcLoopFunctions->GetObjectiveFunction();
            std::cout << "Score " << fObjectiveFunction << std::endl;
            printf("Debug: Objective Function Score: %f\n", fObjectiveFunction);
        } */

		/*
		 * 5. Update the FSM if needed
		 */
		// pass all the necessary parameters to the FSM Updator
/* 		for (int i = 0; i < m_pcRobotState->GetNumberNeighbors(); i++){
			//std::cout << "Neighbor " << i << " ID: " << m_pcRobotState->GetRangeAndBearingMessages()[i]->Data[0] << std::endl;
		} this for loop is just to show how to access the data of the neighbors 
		to set data for the neighbors to access it just use the following code: */

		//evaluate reward base on number of neighbors
		//Reward(m_pcRobotState->GetNumberNeighbors(), m_pcRobotState->GetGroundReading());

		// if updater doesn't exist yet create it else pass 
		AutoMoDeController* instance = GetInstance();
		int timeStep = m_unTimeStep;
		std::string fsmConfiguration = m_strFsmConfiguration;
		//updator.UpdateFsmLauncher(instance, timeStep, &fsmConfiguration);

		// ...

		if (!isRunning) {
			future = updator.UpdateFsmLauncherAsync(instance, m_unTimeStep, &m_strFsmConfiguration);
			isRunning = true;
		}
		if (future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
			// The result is ready
			try {
				future.get();  // This will not block because we know the result is ready
				isRunning = false;  // Reset the flag
			} catch (...) {
				// Handle any exceptions thrown by UpdateFsmLauncher
			}
		} else {
			// The result is not ready yet, continue with the rest of the program
                //}
            // wait until the future is ready then get the result
            try{
            future.wait();
            future.get();}
            catch (...){}

		}


		// here we will try to retreive the score of current fsm and the history of the fsm
		// Retrieval of the score of the swarm driven by the Finite State Machine (Copied from the AutoMoDeMain.cpp)
		// Declare and initialize the variable "cSimulator"
		//CSimulator& cSimulator = CSimulator::GetInstance();
		//CoreLoopFunctions& cLoopFunctions = dynamic_cast<CoreLoopFunctions&> (cSimulator.GetLoopFunctions());
		//Real fObjectiveFunction = cLoopFunctions.GetObjectiveFunction();
		////std::cout << "Score " << fObjectiveFunction << std::endl;
	
	}

    /****************************************/
    /****************************************/

    void AutoMoDeController::Destroy() {}

    /****************************************/
    /****************************************/

    void AutoMoDeController::Reset() {
        m_pcFiniteStateMachine->Reset();
        m_pcRobotState->Reset();
		// Restart actuation.
        InitializeActuation();
    }

    /****************************************/
    /****************************************/

    void AutoMoDeController::SetFiniteStateMachine(AutoMoDeFiniteStateMachine* pc_finite_state_machine) { 
    /* if (m_pcFiniteStateMachine != nullptr) {
        // destroy the old FSM with the destructor
        std::cout << "Destroying old FSM" << std::endl;
        delete m_pcFiniteStateMachine;
    } */
    m_pcFiniteStateMachine = pc_finite_state_machine;
    m_pcFiniteStateMachine->SetRobotDAO(m_pcRobotState);
    m_pcFiniteStateMachine->Init();
    m_bFiniteStateMachineGiven = true;
}

    /****************************************/
    /****************************************/

    void AutoMoDeController::SetHistoryFlag(bool b_history_flag) {
        if (b_history_flag) {
            m_pcFiniteStateMachine->MaintainHistory();
        }
    }

    /****************************************/
    /****************************************/

    void AutoMoDeController::InitializeActuation() {
		/*
		 * Constantly send range-and-bearing messages containing the robot integer identifier.
		 */
        if (m_pcRabActuator != NULL) {
            UInt8 data[4];
            data[0] = m_unRobotID;
            data[1] = 0;
            data[2] = 0;
            data[3] = 0;
            m_pcRabActuator->SetData(data);
        }
    }

    void AutoMoDeController::UpdateFSM(std::string  NewFsmConfig) {
        //std::lock_guard<std::mutex> guard(fsmMutex);
        //std::cout << "robot id: " << m_unRobotID << " UpdateFSM called with new FSM config: " << NewFsmConfig << std::endl;
        std::cout << "robot id: " << m_unRobotID << " UpdateFSM called with new FSM config: " << std::endl;
        /* AutoMoDeFiniteStateMachine* pcNewFiniteStateMachine = m_pcFsmBuilder->BuildFiniteStateMachine(NewFsmConfig);
        if (pcNewFiniteStateMachine == nullptr) {
            std::cerr << "Failed to build new FSM. Received nullptr." << std::endl;
            return;
        }
        //std::cout << "New FSM  built." << std::endl;
        try {
            //std::cout << "################################################################################################################################################################################################Attempting to set new FSM." << std::endl;
            SetFiniteStateMachine(pcNewFiniteStateMachine);
            //std::cout << "FSM successfully updated." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Exception caught while setting new FSM: " << e.what() << std::endl;
            delete pcNewFiniteStateMachine;
        } catch (...) {
            std::cerr << "Unknown error caught while setting new FSM." << std::endl;
            delete pcNewFiniteStateMachine;
        }
     */
    /* Model from init : */
    /*
    if (m_strFsmConfiguration.compare("") != 0 && !m_bFiniteStateMachineGiven) {
            m_pcFsmBuilder = new AutoMoDeFsmBuilder();
            SetFiniteStateMachine(m_pcFsmBuilder->BuildFiniteStateMachine(m_strFsmConfiguration));
            if (m_bMaintainHistory) {
                m_pcFiniteStateMachine->SetHistoryFolder(m_strHistoryFolder);
                m_pcFiniteStateMachine->MaintainHistory();
            }
            if (m_bPrintReadableFsm) {
                //std::cout << "Finite State Machine description: " << std::endl;
                //std::cout << m_pcFiniteStateMachine->GetReadableFormat() << std::endl;
            }
        } else {
            LOGERR << "Warning: No finite state machine configuration found in .argos" << std::endl;
        }*/
    /* Then sweatable version for the update: */
        //delete m_pcFsmBuilder; // delete the old builder

        //m_pcFsmBuilder = new AutoMoDeFsmBuilder();
        //printf("old fsm config: %s\n", m_strFsmConfiguration.c_str());
        try {
            if (m_pcFiniteStateMachine == nullptr) {
            throw std::runtime_error("m_pcFiniteStateMachine is null");
            }
            //std::cout << "################################################################################################################################################################################################Attempting to set new FSM." << std::endl;
            //SetFiniteStateMachine(m_pcFsmBuilder->BuildFiniteStateMachine(NewFsmConfig));
            // print out the m_vecBehaviours and m_vecConditions
            //std::cout << "Behaviours: " << m_pcFiniteStateMachine->GetBehaviours().size() << std::endl;
            for (int i = 0; i < m_pcFiniteStateMachine->GetBehaviours().size(); i++) {
                //std::cout << "Behaviour " << i << ": " << m_pcFiniteStateMachine->GetBehaviours()[i]->GetLabel() << std::endl;
            }
            //std::cout << "Conditions: " << m_pcFiniteStateMachine->GetConditions().size() << std::endl;
            for (int i = 0; i < m_pcFiniteStateMachine->GetConditions().size(); i++) {
                //std::cout << "Condition " << i << ": " << m_pcFiniteStateMachine->GetConditions()[i]->GetLabel() << std::endl;
            }
            //std::cout << "Current Behaviour Index: " << m_pcFiniteStateMachine->GetCurrentBehaviourIndex() << std::endl;
            int curr = m_pcFiniteStateMachine->GetCurrentBehaviourIndex();

                    if (m_pcFsmBuilder == nullptr) {
                throw std::runtime_error("m_pcFsmBuilder is null");
            }
            AutoMoDeFiniteStateMachine* NewAutoFsmConfig = m_pcFsmBuilder->BuildFiniteStateMachine(NewFsmConfig);
            if (NewAutoFsmConfig == nullptr) {
                throw std::runtime_error("Failed to build new FSM");
            }
            //std::cout << "New FSM built." << std::endl;
            //std::cout << "Behaviours: " << NewAutoFsmConfig->GetBehaviours().size() << std::endl;
            for (int i = 0; i < NewAutoFsmConfig->GetBehaviours().size(); i++) {
                //std::cout << "Behaviour " << i << ": " << NewAutoFsmConfig->GetBehaviours()[i]->GetLabel() << std::endl;
            }
            //std::cout << "Conditions: " << NewAutoFsmConfig->GetConditions().size() << std::endl;
            for (int i = 0; i < NewAutoFsmConfig->GetConditions().size(); i++) {
                //std::cout << "Condition " << i << ": " << NewAutoFsmConfig->GetConditions()[i]->GetLabel() << std::endl;
            }
            NewAutoFsmConfig->m_unCurrentBehaviourIndex = curr;
            //std::cout << "Current Behaviour Index: " << NewAutoFsmConfig->GetCurrentBehaviourIndex() << std::endl;
            //std::cout << "Current behaviour: " << NewAutoFsmConfig->GetBehaviours()[NewAutoFsmConfig->GetCurrentBehaviourIndex()]->GetLabel() << std::endl;
            // set the current behaviour index to the same as the old one
            
            if (NewAutoFsmConfig->GetCurrentBehaviourIndex() < 0 || NewAutoFsmConfig->GetCurrentBehaviourIndex() >= NewAutoFsmConfig->GetBehaviours().size()) {
                    throw std::out_of_range("Current Behaviour Index is out of range");
                }
            m_pcFiniteStateMachine = NewAutoFsmConfig;
            m_pcFiniteStateMachine->SetRobotDAO(m_pcRobotState);
            m_pcFiniteStateMachine->Init();
            m_pcFiniteStateMachine->SetHistoryFolder(m_strHistoryFolder);
            m_pcFiniteStateMachine->MaintainHistory();
            // set m_pcCurrentBehaviour = m_vecBehaviours.at(m_unCurrentBehaviourIndex);
            m_pcFiniteStateMachine->m_pcCurrentBehaviour = m_pcFiniteStateMachine->m_vecBehaviours[(m_pcFiniteStateMachine->m_unCurrentBehaviourIndex)];
            //				m_vecCurrentConditions = GetOutgoingConditions();
            m_pcFiniteStateMachine->m_vecCurrentConditions = m_pcFiniteStateMachine->GetOutgoingConditions();
            m_pcFiniteStateMachine->m_bEnteringNewState = false;//print the robot number
            //m_pcFiniteStateMachine->ControlStep();
            //std::cout << "Robot-number: " << m_unRobotID << std::endl;
            //std::cout << "FSM successfully updated." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Exception caught while setting new FSM: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown error caught while setting new FSM." << std::endl;
        }
    }

    bool isFileLocked(const std::string& filename) {
        int fd = open(filename.c_str(), O_RDONLY);
        if (fd == -1) {
            return false;
        }

        int lock_result = flock(fd, LOCK_EX | LOCK_NB);
        if (lock_result == 0) {
            flock(fd, LOCK_UN);
            close(fd);
            return false;
        } else {
            close(fd);
            return true;
        }
    }

    void AutoMoDeController::ExtractLogFile() {
        std::string timestep = std::to_string(m_pcFiniteStateMachine->GetTimeStep());
        //printf("called extractlog TimeStep: %s\n", timestep.c_str());

        if (!m_pcFiniteStateMachine->GetMaintainHistoryFlag()) {
            std::cerr << "FSM is not maintaining history.\n";
            return;
        }

        AutoMoDeFsmHistory* histo = m_pcFiniteStateMachine->GetHistory();
        if (histo == nullptr) {
            std::cerr << "Failed to retrieve FSM history\n";
            return;
        }

        //printf("Robot ID: %d\n", m_unRobotID);
        std::string filename = "output_history_" + std::to_string(m_unRobotID) + ".txt";

        if (isFileLocked(filename)) {
            std::cerr << "File is currently locked by another process: " << filename << std::endl;
            return;
        }

        int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1) {
            std::cerr << "Failed to open the file: " << filename << std::endl;
            return;
        }

        if (flock(fd, LOCK_EX) == -1) {
            std::cerr << "Failed to lock the file: " << filename << std::endl;
            close(fd);
            return;
        }

        std::ofstream output_file(filename, std::ios::app);
        if (!output_file.is_open()) {
            std::cerr << "Failed to open the file: " << filename << std::endl;
            flock(fd, LOCK_UN);
            close(fd);
            return;
        }

        std::vector<std::string> m2_buffer = histo->GetBuffer();
        //std::cout << "Buffer size: " << m2_buffer.size() << std::endl;
        // first we write on first the fsm configuration
        output_file << "--fsm-config " << m_unRobotID<< " " << m_strFsmConfiguration << std::endl;
        for (std::vector<std::string>::iterator it = m2_buffer.begin(); it != m2_buffer.end(); ++it) {
            output_file << *it << std::endl;
        }
        output_file.close();
        flock(fd, LOCK_UN);
        close(fd);
        //printf("History successfully written to output_history_%d.txt\n", m_unRobotID);
    }

    void AutoMoDeController::linkSimulator(argos::CSimulator& cSimulator) {
        //m_pcLoopFunctions = dynamic_cast<CoreLoopFunctions*>(&cSimulator.GetLoopFunctions());
        std::cout << "Linked simulator" << std::endl;
    }


    AutoMoDeController* AutoMoDeController::GetInstance() {
        return this;
    }

    REGISTER_CONTROLLER(AutoMoDeController, "automode_controller");
}
