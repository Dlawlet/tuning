/*
 * @file <src/core/AutoMoDeFsmHistory.cpp>
 *
 * @author Antoine Ligot - <aligot@ulb.ac.be>
 *
 * @package ARGoS3-AutoMoDe
 *
 * @license MIT License
 */

#include "AutoMoDeFsmHistory.h"


namespace argos {

	/****************************************/
	/****************************************/

	AutoMoDeFsmHistory::AutoMoDeFsmHistory(const std::string& str_path) {
		m_strPath = str_path;
		OpenFile();
	}

	/****************************************/
	/****************************************/

	AutoMoDeFsmHistory::AutoMoDeFsmHistory(AutoMoDeFsmHistory* pc_fsm_history) {
		m_strPath = pc_fsm_history->GetPath();
		OpenFile();
	}

	/****************************************/
	/****************************************/

	AutoMoDeFsmHistory::~AutoMoDeFsmHistory() {
            CloseFile();
  	}

	/****************************************/
	/****************************************/

	void AutoMoDeFsmHistory::OpenFile() {
		m_ofHistoryFile.open(m_strPath.c_str(), std::ofstream::out | std::ofstream::trunc);
		if(m_ofHistoryFile.fail()) {
			THROW_ARGOSEXCEPTION("Error opening file \"" << m_strPath);
		}
	}

	/****************************************/
	/****************************************/

	void AutoMoDeFsmHistory::CloseFile() {
        	
		for(std::vector<std::string>::iterator it = m_buffer.begin(); it != m_buffer.end(); ++it)
		{
			std::cout << *it << std::endl;
			m_ofHistoryFile << *it << std::endl;
		}
//	     m_ofHistoryFile.close();
	}
	/****************************************/
	/****************************************/

	void AutoMoDeFsmHistory::AddTimeStep(UInt32 un_time_step, AutoMoDeBehaviour* pc_current_state, std::map<AutoMoDeCondition*, bool> map_transition_status) {
		std::stringstream ssInput;
		ssInput << "--t " << un_time_step << " "<< "--s" << pc_current_state->GetIndex() << " " << pc_current_state->GetIdentifier() << " ";
		//ssInput << "--s" << pc_current_state->GetIndex() << " " << pc_current_state->GetIdentifier() << " ";
               
		std::map<AutoMoDeCondition*, bool>::iterator it;
                bool first = true;
                int t_active = 0;
		for (it = map_transition_status.begin(); it != map_transition_status.end(); ++it) {
			if((it->second) && first) { //priting only the first condition that is active (the one choosen by the controller)
			    ssInput << "--c" << (it->first)->GetIndex() << " "  << (it->first)->GetIdentifier() << " " << (it->second) << " " << (it->first)->GetPbTransition() << " ";
                            t_active++;
                            first = false;
			} else if((it->second)){
                            t_active++; 
			    //ssInput << "0 ";
			}
		}
		if(t_active > 0) {
               		ssInput << "--a " << t_active << " "; //printing number of active transitions
		}
                        ssInput << "--n " << pc_current_state->GetRobotDAO()->GetNumberNeighbors() << " "; //Printing number of neighbors
                        ssInput << "--f " << pc_current_state->GetRobotDAO()->GetGroundReading() << " "; //Printing ground floor reading
		//m_ofHistoryFile << ssInput.str() << std::endl;
		m_buffer.push_back(ssInput.str());
	}

	/****************************************/
	/****************************************/

	void AutoMoDeFsmHistory::AddTimeStep(UInt32 un_time_step, AutoMoDeBehaviour* pc_current_state) {
		std::stringstream ssInput;
		ssInput << "--t " << un_time_step << " ";
		ssInput << "--s" << pc_current_state->GetIndex() << " " << pc_current_state->GetIdentifier() << " ";
		m_buffer.push_back(ssInput.str());
		//m_ofHistoryFile << ssInput.str() << std::endl;
	}

	/****************************************/
	/****************************************/

	const std::string& AutoMoDeFsmHistory::GetPath() const{
		return m_strPath;
	}
}
