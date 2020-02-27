#!/bin/bash

#BATCH --job-name=distance_act1_mp2525        
#SBATCH --output=/scratch/mp2525/distance_act1_mp2525.txt	
#SBATCH --error=/scratch/mp2525/distance_act1_mp2525.err
#SBATCH --time=02:00
#SBATCH --mem=2000 
#SBATCH --nodes=1
#SBATCH --ntasks=8 
#SBATCH --cpus-per-task=1
#SBATCH --exclusive

module load openmpi

mpicc -O3 distance_act1_mp2525.c -lm -o distance_act1_mp2525.out -std=c99

srun -n1 /usr/bin/perf stat -B -e cache-references,cache-misses  distance_act1_mp2525.out 10000 90 ../../MSD_year_prediction_normalize_0_1_100k.txt
