#!/bin/bas
# Path to the AutoMoDe software:
EXE=/home/ubuntu/daremo/tuning/epuck/ARGoS3-AutoMoDe/bin/automode_main

IRACE_OUT="$1"
INSTANCE="$2"
#SEED=$RANDOM 
SEED=2024

# In case of error, we print the current time:
error() {
    echo "`TZ=UTC date`: error: $@" >&2
    exit 1
}

if [ ! -x "${EXE}" ]; then
    error "${EXE}: not found or not executable (pwd: $(pwd))"
fi

extract_and_execute() {
    awk '/Best configurations as commandlines/{flag=1} flag' "$IRACE_OUT" | tail -n +2 | while read -r id config_params; do
        echo "Executing command for ID: $id"
        OUTPUT=$($EXE -n -c $INSTANCE --seed $SEED -t --fsm-config ${config_params})
        # Extract the score from the AutoMoDe (i.e. and ARGoS) output
	SCORE=$(echo ${OUTPUT} | grep -o -E 'Score [-+0-9.e]+' | cut -d ' ' -f2)
	if ! [[ "$SCORE" =~ ^[-+0-9.e]+$ ]] ; then
   	 	error "Output is not a number"
	fi


	# Saving the FSM configuration
	FSM_LOG="fsm_logs/${id}_fsm_log.txt"
	echo --fsm-config ${id} ${config_params} >> $FSM_LOG
	#echo " Score $SCORE" >> $FSM_LOG
	echo "$OUTPUT" >> $FSM_LOG

	# Print score!
	echo "-$SCORE"
    done
}

extract_and_execute

exit 0

