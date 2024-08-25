import os
import re
import sys
import random
import fcntl
from time import sleep


def output_join(output_folder, swarm_objective_function_value, nmber_robots):
    """
    This function browses the output folder and joins robot outputs with the swarm objective function value to generate a log file usable by the OFFpolicy Estimator.

    Args:
        output_folder (str): The path to the output folder.
        swarm_objective_function_value (float): The value of the swarm objective function.

    Returns:
        None
    """
    print("in output")
    # List all files in the output folder
    files = os.listdir(output_folder)
    # Define the log file name
    log_file = os.path.join(output_folder, "Glogfile.txt")

    # Select a random robot output file
    robot_files = [file for file in files if re.match(r"output_history_\d+.txt", file)]
    while not robot_files or len(robot_files) != nmber_robots:
        print("No robot output files found.")
        sleep(10)
        robot_files = [file for file in files if re.match(r"output_history_\d+.txt", file)]

    random_file = random.choice(robot_files)

    # Open the log file and lock it to prevent other processes from writing or reading it
    with open(log_file, "w") as f:
        fcntl.flock(f, fcntl.LOCK_EX)
        try:
            print("Writing to", log_file)
            # Open the randomly selected robot output file and copy the first line to the log file
            with open(os.path.join(output_folder, random_file), "r") as f_robot:
                f.write(f_robot.readline())
            # Write the score on the next line
            f.write("Score " + str(swarm_objective_function_value) + "\n")
            # Write all robot outputs in the log file, excluding the first line
            for file in robot_files:
                # Open the robot output file    
                with open(os.path.join(output_folder, file), "r") as f_robot:
                    # Skip the first line
                    next(f_robot)
                    # Copy the rest of the file to the log file
                    for line in f_robot:
                        f.write(line)
        finally:
            fcntl.flock(f, fcntl.LOCK_UN) # Unlock the log file
            # delete all the output files
            for file in robot_files:
                    os.remove(os.path.join(output_folder, file)) 


if __name__ == "__main__":
    timestep = int(sys.argv[1])
    nmber_robots = int(sys.argv[2])
    file_objective_function_value = "objectiveval.txt"
    # first we need to erase the estimated_fsm.txt file, [ need to do this in a better way]
    with open("estimated_fsms.txt", "w") as f:
        f.write("")
    #
    
    print("in output_join.py, timestep:", timestep)
    # Open the objectiveval.txt file to get the swarm objective function value
    with open(file_objective_function_value, "r") as f:
        # The objective function value is the second value of the line containing the int timestep
        lines = f.readlines()
        if timestep < len(lines):
            swarm_objective_function_value = float(lines[timestep].split()[1])
        else:
            #print(f"Timestep {timestep} is out of range.")
            sys.exit(1)

    #print("swarm_objective_function_value:", swarm_objective_function_value)
    output_join("./", swarm_objective_function_value, nmber_robots)
