#!/bin/bash

# Random_seed list for the experiment
RANDOM_SEED_LIST=(7676 134524 226452 334353 449872 543342 636439 74765 93925383 103539682)

# Read the ARGoS configuration file
#CONFIG_FILE="fora_launch.argos" # for original data -- don't forget to update the loopfunction call to py to eanable update
CONFIG_FILE="pseudo-fora_launch.argos" # for Pseudo-reality data
TEMP_SEED_FILE="temp_seed.argos"
OUTPUT_FILE="automated_test_result.txt"
ERROR_FILE="temp_error.log"

# Clear the output file if it already exists
> $OUTPUT_FILE

# Iterate over the random_seed values
for RANDOM_SEED in "${RANDOM_SEED_LIST[@]}"; do
    # Read the ARGoS configuration file
    CONFIG_CONTENT=$(cat $CONFIG_FILE)
    # Replace the placeholder with the actual random_seed value
    CONFIG_CONTENT=${CONFIG_CONTENT//PLACEHOLDER_SEED/$RANDOM_SEED}

    # Print to terminal
    echo "Running experiment with random seed = $RANDOM_SEED"

    # Write the updated configuration into a temporary file
    echo "$CONFIG_CONTENT" > $TEMP_SEED_FILE

    # Run the ARGoS experiment and capture stderr into a temporary error file
    argos3 -c $TEMP_SEED_FILE 2> $ERROR_FILE

    # Strip ANSI color codes and get the last "Obj" line
    last_obj_line=$(grep --text "Obj " $ERROR_FILE | sed 's/\x1B\[[0-9;]*[a-zA-Z]//g' | tail -n 1)

    # Append the last "Obj" line to the output file
    echo "$last_obj_line" >> $OUTPUT_FILE
done

# Remove the temporary files
rm $TEMP_SEED_FILE
rm $ERROR_FILE

