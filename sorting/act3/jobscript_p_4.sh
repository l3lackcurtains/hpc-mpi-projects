#!/bin/bash

#BATCH --job-name=sort_act3_mp2525_p4
#SBATCH --output=/scratch/mp2525/sort_act3_mp2525_p4.txt	
#SBATCH --error=/scratch/mp2525/sort_act3_mp2525_p4.err
#SBATCH --time=360:00
#SBATCH --mem=0
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=1
#SBATCH --exclusive

module load openmpi

mpicc -O3 sort_act3_mp2525.c -lm -o sort_act3_mp2525.out -std=c99

srun ./sort_act3_mp2525.out
