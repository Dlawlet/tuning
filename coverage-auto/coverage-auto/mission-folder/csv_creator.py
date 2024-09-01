"""
This file is responsible of updating the off-policy_estimated_values.csv file by adding the results of multiple run on argos and their median values.
"""

import subprocess
import pandas as pd
import numpy as np
import os
import sys
import math 


if __name__ == "__main__":
    # open csv file and read it
    csv_file = "./off-policy_estimated_values.csv"
    df = pd.read_csv(csv_file)
    RANDOM_SEED_LIST=[7676, 134524, 226452, 334353, 449872, 543342, 636439, 74765, 93925383, 103539682]
    # create 11 blank columns for runs and median 
    for seed in RANDOM_SEED_LIST:
        df[str(seed)] = np.nan
    df["median"] = np.nan
    """ CSV HEAD =
    fsm,wis,ois,wisp,iosp
    --nstates 4 --s0 3 --n0 1 --n0x0 0 --c0x0 4 --p0x0 8 --w0x0 3.5 --s1 0 --rwm1 1 --n1 2 --n1x0 1 --c1x0 0 --p1x0 0.559 --n1x1 0 --c1x1 2 --p1x1 0.018 --s2 2 --n2 2 --n2x0 0 --c2x0 2 --p2x0 0.396 --n2x1 2 --c2x1 2 --p2x1 0.782 --s3 3 --n3 4 --n3x0 0 --c3x0 2 --p3x0 0.177 --n3x1 0 --c3x1 4 --p3x1 5 --w3x1 11.23 --n3x2 1 --c3x2 5 --p3x2 0.704 --n3x3 0 --c3x3 4 --p3x3 7 --w3x3 1.02 ,47.55,0.009,46.171,0.009
    --nstates 4 --s0 3 --n0 1 --n0x0 0 --c0x0 4 --p0x0 7 --w0x0 3.5 --s1 0 --rwm1 1 --n1 2 --n1x0 1 --c1x0 0 --p1x0 0.16 --n1x1 0 --c1x1 2 --p1x1 0.513 --s2 2 --n2 2 --n2x0 0 --c2x0 2 --p2x0 0.318 --n2x1 2 --c2x1 2 --p2x1 0.829 --s3 3 --n3 4 --n3x0 0 --c3x0 2 --p3x0 0.126 --n3x1 0 --c3x1 4 --p3x1 4 --w3x1 11.23 --n3x2 1 --c3x2 5 --p3x2 0.964 --n3x3 0 --c3x3 4 --p3x3 6 --w3x3 1.02 ,46.733,1.15147e+06,45.96,1.13243e+06
    """
    # for each fsm in column fsm of the csv file call the function to update the csv file
    for i in range(len(df)):
        fsm = df.iloc[i,0]
        med_list = []
        #print(fsm)
        for seed in RANDOM_SEED_LIST:
            # call the function to update the csv file
            #os.system("bash launcher.sh " + fsm)
            # launch the bash file and retrieve the result echo
            result = subprocess.check_output(["bash", "launcher.sh", str(seed), fsm]).decode("utf-8")
            #print(int(result[-4:-1]))
            # update the csv file with the result
            df.loc[i,str(seed)] = int(result[-4:-1])
            med_list.append(int(result[-4:-1]))

        # calculate the median of the 10 runs
        median = np.median(med_list)
        # update the csv file with the median
        df.loc[i,"median"] = median

    # save the updated csv file
    df.to_csv("model_data.csv", index=False)

    print(df.head())

