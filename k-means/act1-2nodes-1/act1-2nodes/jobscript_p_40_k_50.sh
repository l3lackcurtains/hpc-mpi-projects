#!/bin/bash
#SBATCH -C sl
#BATCH --job-name=a_kmeans_act1_mp2525_p40_k50_i1
#SBATCH --output=/scratch/mp2525/kmeans_act1_mp2525_p40_k50_i1.txt	
#SBATCH --error=/scratch/mp2525/kmeans_act1_mp2525_p40_k50_i1.err
#SBATCH --time=30:00
#SBATCH --mem=0
#SBATCH --nodes=2
#SBATCH --ntasks=40
#SBATCH --ntasks-per-node=20
#SBATCH --exclusive

module load openmpi

mpicc -O3 kmeans_act1_mp2525.c -lm -o kmeans_act1_mp2525.out -std=c99

srun ./kmeans_act1_mp2525.out 5159737 2 50 ../iono_57min_5.16Mpts_2D.txt