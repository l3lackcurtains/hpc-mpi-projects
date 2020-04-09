import matplotlib.pyplot as plt
import pandas as pd 

data = pd.read_csv("../kmeans_output.txt", delimiter = ',', names =['cluster', 'x', 'y'])
centroids = pd.read_csv("../centroids.txt", delimiter = ',', names =['x', 'y'])

x = data['x'].to_numpy()
y = data['y'].to_numpy()
color = data['cluster']
plt.scatter(x, y, c=color)

centroids_x = centroids['x'].to_numpy()
centroids_y = centroids['y'].to_numpy()

plt.scatter(centroids_x, centroids_y, s=2, c=None, edgecolors="black", marker="o", linewidth=15)

plt.title('Kmean')
plt.xlabel('x')
plt.ylabel('y')
plt.savefig('scatter_plot.png')