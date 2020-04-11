#!/bin/bash
#SBATCH -C sl
#BATCH --job-name=b_kmeans_act1_mp2525_p32_k2
#SBATCH --output=/scratch/mp2525/kmeans_act1_mp2525_p32_i3_k2.txt
#SBATCH --error=/scratch/mp2525/kmeans_act1_mp2525_p32_i3_k2.err
#SBATCH --time=30:00
#SBATCH --mem=0
#SBATCH --nodes=2
#SBATCH --ntasks=32
#SBATCH --ntasks-per-node=16
#SBATCH --exclusive

module load openmpi

mpicc -O3 kmeans_act1_mp2525.c -lm -o kmeans_act1_mp2525.out -std=c99

srun ./kmeans_act1_mp2525.out 5159737 2 2 ../iono_57min_5.16Mpts_2D.txt