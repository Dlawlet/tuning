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


namespace argos {

	/****************************************/
	/****************************************/

	AutoMoDeController::AutoMoDeController() {
		m_pcRobotState = new ReferenceModel1Dot2();
		m_unTimeStep = 0;
		m_strFsmConfiguration = "";
		m_bMaintainHistory = false;
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

		m_unRobotID = atoi(GetId().substr(5, 6).c_str());
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
		Reward(m_pcRobotState->GetNumberNeighbors(), m_pcRobotState->GetGroundReading());

		// if updater doesn't exist yet create it else pass 
		AutoMoDeController* instance = GetInstance();
		updator.UpdateFsmLauncher(instance, m_unTimeStep, &m_strFsmConfiguration);

		
		

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

	std::int32_t AutoMoDeController::Reward(int32_t neighbors, float ground) { // Modify the function signature to use int32_t as the parameter type and return type
		int version = 2;
		if (version == 1){
			if (neighbors >= NYF_old_neighbors_count) {
				NYF_reward += 1;
				NYF_old_neighbors_count = neighbors;
			}
			return NYF_reward;
		}
		else {
			// note black and white threshold isn't define in the fsm but in the Reference model of the censor, therefore may not optimisable right ?
			//note since the modification impact directly the controller it can saturate the program, the first issue spotter the time is not more coherent with reality time. 
			if (ground < 0.5){
				NYF_reward +=1 ;
			}
		}
	}
	AutoMoDeController* AutoMoDeController::GetInstance() {
		// Return the instance of the controller
		return this;
	}

	REGISTER_CONTROLLER(AutoMoDeController, "automode_controller");
}
