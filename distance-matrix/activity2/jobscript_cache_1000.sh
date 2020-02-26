#!/bin/bash

#BATCH --job-name=distance_matrix_metrices_cache.c       
#SBATCH --output=/scratch/mp2525/distance_matrix_metrices_cache.c.txt	
#SBATCH --error=/scratch/mp2525/distance_matrix_metrices_cache.c.err
#SBATCH --time=02:00
#SBATCH --mem=2000 
#SBATCH --nodes=1
#SBATCH --ntasks=20 
#SBATCH --cpus-per-task=1

module load openmpi

mpicc -O3 distance_matrix_metrices_cache.c.c -lm -o distance_matrix_metrices_cache.c.out -std=c99

srun distance_matrix_metrices_cache.c.out 10000 90 1000 MSD_year_prediction_normalize_0_1_100k.txt
