#!/bin/bash

#BATCH --job-name=sort_act2_mp2525
#SBATCH --output=/scratch/mp2525/sort_act2_mp2525.txt	
#SBATCH --error=/scratch/mp2525/sort_act2_mp2525.err
#SBATCH --time=05:00
#SBATCH --mem=4000
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --cpus-per-task=1
#SBATCH --exclusive

module load openmpi

mpicc -O3 sort_act2_mp2525.c -lm -o sort_act2_mp2525.out -std=c99

srun ./sort_act2_mp2525.out
