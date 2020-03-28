#!/bin/bash
#SBATCH -C sl
#BATCH --job-name=range_act1_mp2525_p1
#SBATCH --output=/scratch/mp2525/range_act1_mp2525_p1.txt	
#SBATCH --error=/scratch/mp2525/range_act1_mp2525_p1.err
#SBATCH --time=15:00
#SBATCH --mem=20000
#SBATCH --nodes=1
#SBATCH --ntasks=1 
#SBATCH --cpus-per-task=1
#SBATCH --exclusive

module load openmpi

mpicc -O3 range_act1_mp2525.c -lm -o range_act1_mp2525.out -std=c99

srun ./range_act1_mp2525.out 2000000 100000
