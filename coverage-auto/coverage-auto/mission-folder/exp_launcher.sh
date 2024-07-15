#!/bin/bash

# Random_seed list for the experiment
RANDOM_SEED_LIST=(7676 134524 226452 334353 449872 543342 636439 74765 93925383 103539682)

# Read the ARGoS configuration file
CONFIG_FILE="fora_launch.argos"
TEMP_SEED_FILE="temp_seed.argos"

# Iterate over the random_seed values
for RANDOM_SEED in "${RANDOM_SEED_LIST[@]}"; do
    # Read the ARGoS configuration file
    CONFIG_CONTENT=$(cat $CONFIG_FILE)
    # Replace the placeholder with the actual random_seed value
    CONFIG_CONTENT=${CONFIG_CONTENT//PLACEHOLDER_SEED/$RANDOM_SEED}

    # print ot terminal
    echo "Running experiment with random seed = $RANDOM_SEED"

    # Write the updated configuration into temporary file
    echo "$CONFIG_CONTENT" > $TEMP_SEED_FILE

    # Run the ARGoS experiment
    argos3 -c $TEMP_SEED_FILE 
done

# Remove the temporary file
rm $TEMP_SEED_FILE
