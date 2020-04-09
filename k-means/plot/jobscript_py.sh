#!/bin/bash
#SBATCH -C sl
#SBATCH --job-name=kmean_plot
#SBATCH --output=/scratch/mp2525/kmean_plot.txt
#SBATCH --error=/scratch/mp2525/kmean_plot.err
#SBATCH --time=720:00
#SBATCH --mem=0
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1

module load anaconda3

srun python ./kmean_plot.py