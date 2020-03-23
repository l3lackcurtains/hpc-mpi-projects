#!/bin/bash

#BATCH --job-name=range_act2_mp2525_n2_p12
#SBATCH --output=/scratch/mp2525/range_act2_mp2525_n2_p12.txt	
#SBATCH --error=/scratch/mp2525/range_act2_mp2525_n2_p12.err
#SBATCH --time=120:00
#SBATCH --mem=0
#SBATCH --nodes=2
#SBATCH --ntasks=12
#SBATCH --ntasks-per-node=6
#SBATCH --cpus-per-task=1
#SBATCH --exclusive

module load openmpi

mpic++ -O3 range_act2_mp2525.cpp -lm -o range_act2_mp2525.out

srun ./range_act2_mp2525.out 2000000 100000