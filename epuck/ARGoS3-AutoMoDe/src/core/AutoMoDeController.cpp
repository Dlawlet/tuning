/*
 * @file <src/core/AutoMoDeController.cpp>
 *
 * @author Antoine Ligot - <aligot@ulb.ac.be>
 *
 * @package ARGoS3-AutoMoDe
 *
 * @license MIT License
 */

#include "AutoMoDeController.h"
#include "../modules/AutoMoDeFsmUpdator.h"
#include <future>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>


namespace argos {

	/****************************************/
	/****************************************/

	AutoMoDeController::AutoMoDeController() {
		m_pcRobotState = new ReferenceModel1Dot2();
		m_unTimeStep = 0;
		m_strFsmConfiguration = "";
		m_bMaintainHistory = true; // for testing purposes default is false
		m_bPrintReadableFsm = false;
		m_strHistoryFolder = "./";
		m_bFiniteStateMachineGiven = false;
		NYF_old_neighbors_count = 0; // Declare and initialize NYF_old_neighbors_count
		NYF_reward = 0; // Declare and initialize NYF_reward	



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

		//m_unRobotID = atoi(GetId().substr(5, 6).c_str()); 
		// id always = 0 
		m_unRobotID = rand() % 1000; 
		m_pcRobotState->SetRobotIdentifier(m_unRobotID);

		/*
		 * If a FSM configuration is given as parameter of the experiment file, create a FSM from it
		 */
		if (m_strFsmConfiguration.compare("") != 0 && !m_bFiniteStateMachineGiven) {
			m_pcFsmBuilder = new AutoMoDeFsmBuilder();
			SetFiniteStateMachine(m_pcFsmBuilder->BuildFiniteStateMachine(m_strFsmConfiguration));
			if (m_bMaintainHistory) {
				m_pcFiniteStateMachine->SetHistoryFolder(m_strHistoryFolder);
				m_pcFiniteStateMachine->MaintainHistory();
			}
			if (m_bPrintReadableFsm) {
				std::cout << "Finite State Machine description: " << std::endl;
				std::cout << m_pcFiniteStateMachine->GetReadableFormat() << std::endl;
			}
		} else {
			LOGERR << "Warning: No finite state machine configuration found in .argos" << std::endl;
		}



		/*
		 *  Initializing sensors and actuators
		 */
		try{
			m_pcProximitySensor = GetSensor<CCI_EPuckProximitySensor>("epuck_proximity");
			m_pcLightSensor = GetSensor<CCI_EPuckLightSensor>("epuck_light");
			m_pcGroundSensor = GetSensor<CCI_EPuckGroundSensor>("epuck_ground");
			 m_pcRabSensor = GetSensor<CCI_EPuckRangeAndBearingSensor>("epuck_range_and_bearing");
			 m_pcCameraSensor = GetSensor<CCI_EPuckOmnidirectionalCameraSensor>("epuck_omnidirectional_camera");
		} catch (CARGoSException ex) {
			LOGERR<<"Error while initializing a Sensor!\n";
		}

		try{
			m_pcWheelsActuator = GetActuator<CCI_EPuckWheelsActuator>("epuck_wheels");
			m_pcRabActuator = GetActuator<CCI_EPuckRangeAndBearingActuator>("epuck_range_and_bearing");
			m_pcLEDsActuator = GetActuator<CCI_EPuckRGBLEDsActuator>("epuck_rgb_leds");
		} catch (CARGoSException ex) {
			LOGERR<<"Error while initializing an Actuator!\n";
		}

		/*
		 * Starts actuation.
		 */
		 InitializeActuation();


	}

	/****************************************/
	/****************************************/

	void AutoMoDeController::ControlStep() {
		/*
		 * 1. Update RobotDAO
		 */
		if(m_pcRabSensor != NULL){
			const CCI_EPuckRangeAndBearingSensor::TPackets& packets = m_pcRabSensor->GetPackets();
			//m_pcRobotState->SetNumberNeighbors(packets.size());
			m_pcRobotState->SetRangeAndBearingMessages(packets);
		}
		if (m_pcGroundSensor != NULL) {
			const CCI_EPuckGroundSensor::SReadings& readings = m_pcGroundSensor->GetReadings();
			m_pcRobotState->SetGroundInput(readings);
		}
		if (m_pcLightSensor != NULL) {
			const CCI_EPuckLightSensor::TReadings& readings = m_pcLightSensor->GetReadings();
			m_pcRobotState->SetLightInput(readings);
		}
		if (m_pcProximitySensor != NULL) {
			const CCI_EPuckProximitySensor::TReadings& readings = m_pcProximitySensor->GetReadings();
			m_pcRobotState->SetProximityInput(readings);
		}

		/*
		 * 2. Execute step of FSM
		 */
		m_pcFiniteStateMachine->ControlStep();

		/*
		 * 3. Update Actuators
		 */
		if (m_pcWheelsActuator != NULL) {
			m_pcWheelsActuator->SetLinearVelocity(m_pcRobotState->GetLeftWheelVelocity(),m_pcRobotState->GetRightWheelVelocity());
		}

		/*
		 * 4. Update variables and sensors
		 */
		if (m_pcRabSensor != NULL) {
			m_pcRabSensor->ClearPackets();
		}
		m_unTimeStep++;

		/*
		 * 5. Update the FSM if needed
		 */
		// pass all the necessary parameters to the FSM Updator
/* 		for (int i = 0; i < m_pcRobotState->GetNumberNeighbors(); i++){
			std::cout << "Neighbor " << i << " ID: " << m_pcRobotState->GetRangeAndBearingMessages()[i]->Data[0] << std::endl;
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

		}	

		// here we will try to retreive the score of current fsm and the history of the fsm
		// Retrieval of the score of the swarm driven by the Finite State Machine (Copied from the AutoMoDeMain.cpp)
		// Declare and initialize the variable "cSimulator"
		//CSimulator& cSimulator = CSimulator::GetInstance();
		//CoreLoopFunctions& cLoopFunctions = dynamic_cast<CoreLoopFunctions&> (cSimulator.GetLoopFunctions());
		//Real fObjectiveFunction = cLoopFunctions.GetObjectiveFunction();
		//std::cout << "Score " << fObjectiveFunction << std::endl;
	
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

	void AutoMoDeController::UpdateFSM(std::string NewFsmConfig){
		//Construct a new FSM using AutoMoDeFsmBuilder and vecNewConfigFSm
		AutoMoDeFiniteStateMachine* pcNewFiniteStateMachine = m_pcFsmBuilder->BuildFiniteStateMachine(NewFsmConfig);
		// Replace the current FSM with the new one 
		SetFiniteStateMachine(pcNewFiniteStateMachine);
		
	}
	

bool isFileLocked(const std::string& filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        return false; // File does not exist or cannot be opened
    }

    int lock_result = flock(fd, LOCK_EX | LOCK_NB);
    if (lock_result == 0) {
        flock(fd, LOCK_UN); // Unlock if we managed to lock it
        close(fd);
        return false; // File is not locked by another process
    } else {
        close(fd);
        return true; // File is locked by another process
    }
}

void AutoMoDeController::ExtractLogFile() {
    // Extract the log file from the AutoMoDeFiniteStateMachine
    std::string timestep = std::to_string(m_pcFiniteStateMachine->GetTimeStep());
    printf("TimeStep: %s\n", timestep.c_str());

    // Ensure that the FSM is maintaining history
    if (!m_pcFiniteStateMachine->GetMaintainHistoryFlag()) {
        std::cerr << "FSM is not maintaining history.\n";
        return;
    }

    // Retrieve the history
    AutoMoDeFsmHistory* histo = m_pcFiniteStateMachine->GetHistory();
    if (histo == nullptr) {
        std::cerr << "Failed to retrieve FSM history\n";
        return;
    }

    // Write the buffer to a new file
    printf("Robot ID: %d\n", m_unRobotID);
    std::string filename = "output_history_" + std::to_string(m_unRobotID) + ".txt";

    if (isFileLocked(filename)) {
        std::cerr << "File is currently locked by another process: " << filename << std::endl;
        return;
    }

    int fd = open(filename.c_str(), O_WRONLY | O_CREAT, 0666);
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

    for (const std::string& line : histo->GetBuffer()) { // Assuming GetBuffer() returns a vector<string>
        output_file << line << std::endl;
    }
    output_file.close();
    flock(fd, LOCK_UN);  // Unlock the file
    close(fd);           // Close the file descriptor
    printf("History successfully written to output_history_%d.txt\n", m_unRobotID);
}


	AutoMoDeController* AutoMoDeController::GetInstance() {
		// Return the instance of the controller
		return this;
	}

	REGISTER_CONTROLLER(AutoMoDeController, "automode_controller");
}
