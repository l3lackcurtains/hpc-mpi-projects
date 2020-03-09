#!/bin/bash

#BATCH --job-name=sort_act1_mp2525_p2_d1
#SBATCH --output=/scratch/mp2525/sort_act1_mp2525_p2_d1.txt	
#SBATCH --error=/scratch/mp2525/sort_act1_mp2525_p2_d1.err
#SBATCH --time=720:00
#SBATCH --mem=0
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=1
#SBATCH --exclusive

module load openmpi

mpicc -O3 sort_act1_mp2525_d1.c -lm -o sort_act1_mp2525_d1.out -std=c99

srun ./sort_act1_mp2525_d1.out
