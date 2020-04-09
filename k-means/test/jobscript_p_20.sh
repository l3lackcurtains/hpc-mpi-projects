#!/bin/bash
#SBATCH -C sl
#BATCH --job-name=kmeans_out
#SBATCH --output=/scratch/mp2525/kmeans_out.txt
#SBATCH --error=/scratch/mp2525/kmeans_out.err
#SBATCH --time=20:00
#SBATCH --mem=40000
#SBATCH --nodes=1
#SBATCH --ntasks=20
#SBATCH --exclusive

module load openmpi

mpicc -O3 kmeans_out.c -lm -o kmeans_out.out -std=c99

srun ./kmeans_out.out 5159737 2 10 ../iono_57min_5.16Mpts_2D.txt