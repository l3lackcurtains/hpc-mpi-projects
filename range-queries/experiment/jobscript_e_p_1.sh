#!/bin/bash
#SBATCH -C sl
#BATCH --job-name=range_exp_mp2525_p1
#SBATCH --output=/scratch/mp2525/range_exp_mp2525_p1.txt	
#SBATCH --error=/scratch/mp2525/range_exp_mp2525_p1.err
#SBATCH --time=20:00
#SBATCH --mem=0
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --exclusive

module load openmpi

mpic++ -O3 range_exp_mp2525.cpp -lm -o range_exp_mp2525.out

srun ./range_exp_mp2525.out 2000000 100000
