#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>

using namespace std;

// Function declarations
float Objective(std::string individual_fsm);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " fsm-config" << std::endl;
        return 1;
    }

    // load all the arguments into a string 
    std::string fsm_to_eval = "";
    for (int i = 1; i < argc; i++) {
        fsm_to_eval += argv[i];
        fsm_to_eval += " ";
    }

    float objective = Objective(fsm_to_eval);
    std::cout << objective << std::endl;

    return 0;
}

float Objective(std::string individual_fsm) {

    std::string log_file_path = "./Glogfile.txt";
    std::string command = "python3 /home/ubuntu/daremo/tuning/Off-policySwarmPerformanceEstimation-master/AutoMoDeLogAnalyzer.py " + log_file_path + " -pa --newfsm-config " + individual_fsm;

    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Couldn't start command." << std::endl;
        exit(1);
    }

    char buffer[1024]; 

    // we will extract and save all 4 values    
    float wis_value = -1.0;
    float ois_value = -1.0;
    float wisp_value = -1.0;
    float oisp_value = -1.0;

    while (!feof(pipe)) {
        if (fgets(buffer, 1024, pipe) != NULL) {
            std::string line(buffer);
            if (line.find("WIS Expected average performance of the pruned FSM") != std::string::npos) {
                wis_value = std::stof(line.substr(line.find(":") + 1));
            } else if (line.find("OIS Expected average performance of the pruned FSM") != std::string::npos) {
                ois_value = std::stof(line.substr(line.find(":") + 1));
            } else if (line.find("WIS Expected average performance with the proportional reward") != std::string::npos) {
                wisp_value = std::stof(line.substr(line.find(":") + 1));
            } else if (line.find("OIS Expected average performance with the proportional reward") != std::string::npos) {
                oisp_value = std::stof(line.substr(line.find(":") + 1));
            }
        }
    }
        

    pclose(pipe);

    if (wis_value == -1.0) {
        std::cerr << "Error in the command: " << command << std::endl;
        exit(1);
    }

    // save individual_fsm in a text file if it  is not yet present in the file
    std::string fsm_file_path = "./fsm_configs.txt";
    std::ifstream file(fsm_file_path);
    std::string line;
    bool already_present = false;
    while (std::getline(file, line)) {
        if (line == individual_fsm) {
            already_present = true;
            break;
        }
    }
    file.close();

    if (!already_present) {
        std::ofstream file(fsm_file_path, std::ios_base::app);
        file << individual_fsm << std::endl;
        // save the 4 estimated values in a csv file 
        std::string csv_file_path = "./off-policy_estimated_values.csv";
        std::ofstream csv_file(csv_file_path, std::ios_base::app);
        csv_file << individual_fsm << "," << wis_value << "," << ois_value << "," << wisp_value << "," << oisp_value << std::endl;
        file.close();
    }

    return wis_value;
}
