// find a path between two vertices using breadth-first traversal
// only allow edges whose weight is less than "max"
int findPath (Graph g, Vertex src, Vertex dest, int max, int *path)
{
	assert (g != NULL);
	int *visited = malloc(g->nV * sizeof(int));
	for (int i = 0; i < g->nV; i++) {
		visited[i] = -1;
	}

	Queue q = newQueue();
	QueueJoin(q, src);
	visited[src] = src;
	int found = FALSE;
	while (found == FALSE && !QueueIsEmpty(q)) {
		Vertex v = QueueLeave(q);
		if (v == dest) {
			found = TRUE;
		} else {
			for (int i = 0; i < g->nV; i++) {
				if (g->edges[v][i] < max && visited[i] == -1) {
					visited[i] = v;
					QueueJoin(q, i);
				} 
			}
		}
	}

	if (found == TRUE) {
		int i = 0;
		Vertex v;
		int temp[g->nV];
		for (v = dest; v != src; v = visited[v]) {
			temp[i] = v;
			i++;
		}
		temp[i] = v;
		int j = 0;
		while (i >= 0) {
			path[j] = temp[i];
			i--;
			j++;
		}
		free(visited);
		return j;
	}
	free(visited);
	return 0;
}