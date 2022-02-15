/* ******** SOURCE: GEEKS FOR GEEKS ******** */
/* https://www.geeksforgeeks.org/detect-cycle-in-a-graph/ */

#ifndef GRAPH_H
#define GRAPH_H
// A C++ Program to detect cycle in a graph
#include<bits/stdc++.h>
using namespace std;

class Graph
{
    int V;    // No. of vertices
    list<int> *adj;    // Pointer to an array containing adjacency lists
    bool isCyclicUtil(int v, bool visited[], bool *rs);  // used by isCyclic()
public:
    Graph(){}        // ADDED BY ME CONSTRUCTOR
    Graph(int V);   // Constructor
    void addEdge(int v, int w);   // to add an edge to graph
    void removeEdge(int v);   // ADDED BY ME REMOVEEDGE
    bool isCyclic();    // returns true if there is a cycle in this graph
};

#endif /* GRAPH_H */