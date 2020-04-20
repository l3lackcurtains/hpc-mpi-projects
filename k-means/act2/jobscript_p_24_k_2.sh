#!/bin/bash
#SBATCH -C sl
#BATCH --job-name=kmeans_act1_mp2525_p24_k2
#SBATCH --output=/scratch/mp2525/kmeans_act1_mp2525_p24_k2.txt
#SBATCH --error=/scratch/mp2525/kmeans_act1_mp2525_p24_k2.err
#SBATCH --time=05:00
#SBATCH --mem=60000
#SBATCH --nodes=2
#SBATCH --ntasks=24
#SBATCH --ntasks-per-node=12
#SBATCH --exclusive

module load openmpi

mpicc -O3 kmeans_act1_mp2525.c -lm -o kmeans_act1_mp2525.out -std=c99

srun ./kmeans_act1_mp2525.out 5159737 2 2 ../iono_57min_5.16Mpts_2D.txt