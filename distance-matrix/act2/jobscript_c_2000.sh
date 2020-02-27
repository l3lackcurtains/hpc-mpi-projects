#!/bin/bash

#BATCH --job-name=distance_act2_mp2525       
#SBATCH --output=/scratch/mp2525/distance_act2_mp2525.txt	
#SBATCH --error=/scratch/mp2525/distance_act2_mp2525.err
#SBATCH --time=02:00
#SBATCH --mem=2000 
#SBATCH --nodes=1
#SBATCH --ntasks=20 
#SBATCH --cpus-per-task=1
#SBATCH --exclusive

module load openmpi

mpicc -O3 distance_act2_mp2525.c -lm -o distance_act2_mp2525.out -std=c99

srun distance_act2_mp2525.out 100000 90 2000 MSD_year_prediction_normalize_0_1_100k.txt
