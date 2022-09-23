import networkx as nx
import matplotlib.pyplot as plt

n_nodes = 5
n_edges = 15
G = nx.gnm_random_graph(n_nodes, n_edges, seed=None, directed=True)

curId = 0
f = open("graph.txt", "w")
f.write(str(n_nodes)+'\n')
for i in range(n_nodes):
    s = str(i)+' '+str(len(G.in_edges(i)))+' '+' '.join(str(x[0]) for x in G.in_edges(i))+' '+str(len(G.in_edges(i))/2)+'\n'
    f.write(s)
f.close()

nx.draw(G, with_labels=True, font_weight='bold')
plt.show()
