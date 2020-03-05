#!/bin/bash

#BATCH --job-name=sort_act1_mp2525
#SBATCH --output=/scratch/mp2525/sort_act1_mp2525.txt	
#SBATCH --error=/scratch/mp2525/sort_act1_mp2525.err
#SBATCH --time=20:00
#SBATCH --mem=40000
#SBATCH --nodes=1
#SBATCH --ntasks=1 
#SBATCH --cpus-per-task=1
#SBATCH --exclusive

module load openmpi

mpicc -O3 sort_act1_mp2525.c -lm -o sort_act1_mp2525.out -std=c99

srun ./sort_act1_mp2525.out
