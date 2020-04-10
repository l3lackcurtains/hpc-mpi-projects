#!/bin/bash
#SBATCH -C sl
#BATCH --job-name=kmeans_act1_mp2525_p12_k25
#SBATCH --output=/scratch/mp2525/kmeans_act1_mp2525_p12_k25.txt	
#SBATCH --error=/scratch/mp2525/kmeans_act1_mp2525_p12_k25.err
#SBATCH --time=30:00
#SBATCH --mem=0
#SBATCH --nodes=1
#SBATCH --ntasks=12
#SBATCH --exclusive

module load openmpi

mpicc -O3 kmeans_act1_mp2525.c -lm -o kmeans_act1_mp2525.out -std=c99

srun ./kmeans_act1_mp2525.out 5159737 2 25 ../iono_57min_5.16Mpts_2D.txt