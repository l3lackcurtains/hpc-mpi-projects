#!/bin/bash
#SBATCH -C sl
#BATCH --job-name=kmeans_act1_mp2525_p20
#SBATCH --output=/scratch/mp2525/kmeans_act1_mp2525_p20.txt	
#SBATCH --error=/scratch/mp2525/kmeans_act1_mp2525_p20.err
#SBATCH --time=15:00
#SBATCH --mem=40000
#SBATCH --nodes=1
#SBATCH --ntasks=20
#SBATCH --exclusive

module load openmpi

mpicc -O3 kmeans_act1_mp2525.c -lm -o kmeans_act1_mp2525.out -std=c99

srun ./kmeans_act1_mp2525.out 5159737 2 10 ../iono_57min_5.16Mpts_2D.txt