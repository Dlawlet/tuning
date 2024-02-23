#!/bin/bash

# source setup 
source setup.sh

# List of directories to build
build_dirs=("epuck/argos3-epuck/build"  "epuck/demiurge-epuck-dao/build" "epuck/experiments-loop-functions/build" "epuck/ARGoS3-AutoMoDe/build") 

# Function to run a command and log if it fails
run_cmd() {
     # Run the command passed to this function, suppress stdout, keep stderr
    output=$("$@" > /dev/null 2>&1)
    # Capture the exit status of the command
    local status=$?
    if [ $status -ne 0 ]; then
        # If command fails, print stderr and exit
        echo "Error: Command failed - '$*'" >&2
        # Optionally, echo the output here if it's useful for debugging
        echo $output
        exit $status
    fi
}

# Loop through each directory
for dir in "${build_dirs[@]}"; do
    echo "Building in $dir..."

    # Navigate to the directory
    cd "$dir" || { echo "Failed to enter $dir"; continue; }

    # Run cmake, make, and make install commands
    # Check if the directory is the special case
    if [ "$dir" == "epuck/argos3-epuck/build" ]; then
        run_cmd cmake ../src -DCMAKE_INSTALL_PREFIX=$ARGOS_INSTALL_PATH/argos3-dist
    else
        # Default case: Run cmake in the current directory
        run_cmd cmake .. -DCMAKE_INSTALL_PREFIX=$ARGOS_INSTALL_PATH/argos3-dist
    fi
    
    run_cmd make
    if [ "$dir" != "epuck/ARGoS3-AutoMoDe/build" ]; then 
    	run_cmd sudo make install 
    fi


    # Navigate back to the original directory
    cd - > /dev/null

    echo "Finished building in $dir"
done

echo "All specified builds are complete."

