#!/bin/bash

#BATCH --job-name=distance_matrix_metrics        
#SBATCH --output=/scratch/mp2525/distance_matrix_metrics.txt	
#SBATCH --error=/scratch/mp2525/distance_matrix_metrics.err
#SBATCH --time=02:00
#SBATCH --mem=2000 
#SBATCH --nodes=1
#SBATCH --ntasks=12
#SBATCH --cpus-per-task=1

module load openmpi

mpicc -O3 distance_matrix_metrics.c -lm -o distance_matrix_metrics.out -std=c99

srun distance_matrix_metrics.out 10000 90 MSD_year_prediction_normalize_0_1_100k.txt
