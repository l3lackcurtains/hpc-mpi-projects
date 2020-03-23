#!/bin/bash

#BATCH --job-name=range_act1_mp2525_p12
#SBATCH --output=/scratch/mp2525/range_act1_mp2525_p12.txt	
#SBATCH --error=/scratch/mp2525/range_act1_mp2525_p12.err
#SBATCH --time=120:00
#SBATCH --mem=0
#SBATCH --nodes=1
#SBATCH --ntasks=12 
#SBATCH --cpus-per-task=1
#SBATCH --exclusive

module load openmpi

mpicc -O3 range_act1_mp2525.c -lm -o range_act1_mp2525.out -std=c99

srun ./range_act1_mp2525.out 2000000 100000
