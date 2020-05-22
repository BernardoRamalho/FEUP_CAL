#pragma once
#ifndef GRAPH_H_
#define GRAPH_H_

#include <unordered_map>
#include <queue>
#include <list>
#include <limits>
#include <algorithm>
#include <cmath>
#include <chrono>
#include "MutablePriorityQueue.h"
#include "Vertex.h"
using namespace std;
using namespace std::chrono;

class Graph {
private:
    int offsetX;    // for Graph Viewer
    int offsetY;    // for Graph Viewer

    vector<Vertex*> vertexSet;
    unordered_map<int, Vertex*> vertexIndexes;    //search for id and return vertex (much faster)

    vector<vector<double>> minDistance;       // used for floyd Warshall algorithm
    vector<vector<Vertex*>> next;             // used for floyd Warshall algorithm
    void dfsVisit(Vertex *origin) const;      // pre processing

    const static int infinite = 99999999;

public:
    Vertex* findVertex(const int &id) const;
    bool addVertex(const int &id, const int &x, const int &y);
    void addPointOfInterest(Vertex* vertex);
    bool addEdge(const int &id, const int &origin, const int &dest);

    void setOffsetX(int x);
    void setOffsetY(int y);
    int getOffsetX();
    int getOffsetY();

    int getNumVertex() const;
    vector<Vertex*> getVertexSet() const;

    // pre processing
    bool preProcess(int origin);

    // dijkstra
    Vertex* djikstraInitCentral(const int origin);
    Vertex* dijkstraInit(const int origin);
    Vertex* dijkstraBackwardsInit(const int dest);
    bool dijkstraOriginal(const int origin);
    bool dijkstra(const int origin, const int dest, unordered_set<int> &processedEdges);
    int getPathTo(const int dest, vector<Edge> &edges) const;
    int getPathFromCentralTo(const int dest, vector<Edge> &edges) const;

    // dijkstra related
    double heuristicDistance(Vertex *origin, Vertex *dest);
    bool dijkstraOrientedSearch(const int origin, const int dest, unordered_set<int> &processedEdges) ;
    bool dijkstraBidirectional(const int origin, const int dest, unordered_set<int> &processedEdges, unordered_set<int> &processedEdgesInv);

    // all pairs
    void floydWarshallShortestPath();
    vector<int> getfloydWarshallPath(const int origin, const int dest) const;
};

/**************** Pre processing ************/
void Graph::dfsVisit(Vertex *origin) const {
    origin->visited = true;

    for(auto edge : origin->adj)
        if(!edge.dest->visited)
            dfsVisit(edge.dest);

    for(auto edge : origin->invAdj)
        if(!edge.origin->visited)
            dfsVisit(edge.origin);
}

bool Graph::preProcess(int origin) {
    cout << "\bOriginal graph node total: " << vertexSet.size() << endl;
    auto orig = findVertex(origin);
    if (orig == nullptr) return false;

    for(auto vertex : vertexSet)
        vertex->visited = false;

    dfsVisit(orig);

    set<int> removed;
    // deletes nodes
    for(auto it = vertexSet.begin(); it != vertexSet.end(); it++) {
        if(!(*it)->visited) {
            vertexIndexes.erase((*it)->getId());
            removed.insert((*it)->getId());
            it = vertexSet.erase(it);
            it--;
        }
    }

    // deletes outgoing edges of the deleted nodes (no need to delete from invAdj as Vertex is a pointer)
    for(auto vertex : vertexSet) {
        for(auto it = vertex->adj.begin(); it != vertex->adj.end(); it++)
            if(removed.find(it->getDest()->getId()) != removed.end()) {
                it = vertex->adj.erase(it);
                it--;
            }
    }

    cout << "Pre processed graph node total: " << vertexSet.size() << endl;
    return true;
}

/**************** Usual operations ************/

/**
 * @brief finds a vertex in the graph
 * @param id - id of the vertex that we are looking for
 * @return - a pointer to the vertex corresponding to the id given
 */
Vertex* Graph::findVertex(const int &id) const {
    auto it = vertexIndexes.find(id);
    return it == vertexIndexes.end() ? nullptr : it->second;
}

/**
 * @brief Adds a vertex to the graph
 * @param id - id of the vertex to add
 * @param x - coordinate in the x axis of the vertex
 * @param y - coordinate in the y axis of the vertex
 * @return - true if it added successfully
 */
bool Graph::addVertex(const int &id, const int &x, const int &y) {
    if (findVertex(id) != nullptr) return false;

    auto vertex = new Vertex(id, x, y);
    vertexSet.push_back(vertex);
    vertexIndexes.insert(pair<int, Vertex*>(id, vertex));

    return true;
}

/**
 * @brief Adds an edge to the graph
 * @param id - id of the edge to add
 * @param origin - id of the vertex where the edge begins
 * @param dest - id of the vertex where the edge ends
 * @return - true if it added successfully
 */
bool Graph::addEdge(const int &id, const int &origin, const int &dest) {
    auto v1 = findVertex(origin);
    auto v2 = findVertex(dest);
    if (v1 == nullptr || v2 == nullptr)
        return false;

    v1->addEdge(id, v2, v1->pos.euclideanDistance(v2->pos));

    return true;
}

void Graph::setOffsetX(int x) {
    this->offsetX = x;
}

void Graph::setOffsetY(int y) {
    this->offsetY = y;
}

int Graph::getOffsetX() {
    return offsetX;
}

int Graph::getOffsetY() {
    return offsetY;
}

int Graph::getNumVertex() const {
    return vertexSet.size();
}

vector<Vertex*> Graph::getVertexSet() const {
    return vertexSet;
}

/**************** Dijkstra ************/

/**
 * @brief Initializes all the camps of the vertexes related to the central with the default values
 * @param origin - id of the vertex that represents the origin of the graph
 * @return a pointer to the vertex that represents the origin of the graph
 */
Vertex* Graph::djikstraInitCentral(const int origin) {
    for(auto vertex : vertexSet) {
        vertex->visited = false;
        vertex->distCentral = infinite;
        vertex->pathCentral = NULL;
        vertex->edgePathCentral = Edge();
        vertex->queueIndex = 0;
    }

    auto start = findVertex(origin);
    start->distCentral = 0;

    return start;
}

/**
 * @brief Initializes all the camps of the vertexes with the default values
 * @param origin - id of the vertex that represents the origin of the graph
 * @return a pointer to the vertex that represents the origin of the graph
 */
Vertex* Graph::dijkstraInit(const int origin) {
    for(auto vertex : vertexSet) {
        vertex->visited = false;
        vertex->invVisited = false;
        vertex->dist = infinite;
        vertex->path = nullptr;
        vertex->edgePath = Edge();
        vertex->invDist = infinite;
        vertex->invPath = nullptr;
        vertex->invEdgePath = Edge();
        vertex->heuristicValue = infinite;
        vertex->invHeuristicValue = infinite;
        vertex->queueIndex = 0;
        vertex->invQueueIndex = 0;
        vertex->inv = false;
    }

    auto start = findVertex(origin);
    start->dist = 0;
    start->heuristicValue = 0;

    return start;
}

/**
 * @brief Initializes all the camps of the vertexes with the default values related to the invGraph
 * @param dest
 * @return
 */
Vertex* Graph::dijkstraBackwardsInit(const int dest){
    auto final = findVertex(dest);
    final->invDist = 0;
    final->invHeuristicValue = 0;

    return final;
}

/**
 * @brief Runs the dijkstra algorithm to find the shortest path to all the vertixes
 * @param origin - int representing the id of the origin of the graph
 * @return return true if it runned successfully
 */

bool Graph::dijkstraOriginal(const int origin)  {
    // Initializes the vertex variables based on the origin node
    Vertex* start = djikstraInitCentral(origin);

    if(start == nullptr) return false;

    // Initialize the priority queue and insert the first vertex
    MutablePriorityQueue<Vertex> minQueue;
    minQueue.insert(start);

    // Iterate over the priority queue
    while(!minQueue.empty()) {
        // From the queue extract the vertex that has the minimum distance from the origin point
        auto min = minQueue.extractMin();
        min->visited = true;

        // Iterate over all the edges that start in the min vertex
        for(auto edge : min->adj) {
            auto elem = edge.dest;

            // For each child of the min vertex, if the distance to the central is bigger then the
            // distance of the new path, then this is the new best path
            if(elem->distCentral > min->distCentral + edge.weight) {
                elem->distCentral = min->distCentral + edge.weight;
                elem->pathCentral = min;
                elem->edgePathCentral = edge;

                // if elem is not in queue, insert it, otherwise, update the queue with the new path
                if(elem->queueIndex == 0) minQueue.insert(elem);
                else minQueue.decreaseKey(elem);
            }
        }

        // Since our graph is bidirectional we iterate over all the edges that end in the min vertex
        for(auto edge : min->invAdj) {
            auto elem = edge.origin;

            // For each father of the min vertex, if the distance to the central is bigger then the
            // distance of the new path, then this is the new best path
            if(elem->distCentral > min->distCentral + edge.weight) {
                elem->distCentral = min->distCentral + edge.weight;
                elem->pathCentral = min;
                elem->edgePathCentral = edge;

                // if elem is not in queue, insert it, otherwise, update the queue with the new path
                if(elem->queueIndex == 0) minQueue.insert(elem);
                else minQueue.decreaseKey(elem);
            }
        }
    }

    return true;
}

/**
 * @brief Runs dijkstra algorithm to find the best path between two points
 * @param origin - integer representing the id of starting node
 * @param dest - integer representing the id of destination node
 * @param processedEdges - set that stores the id of th edges that are processed
 * @return - true if it runs successfully
 */
bool Graph::dijkstra(const int origin, const int dest, unordered_set<int> &processedEdges)  {
    auto start_time = high_resolution_clock::now();
    // Initialize all the vertex and find the origin and destination
    auto start = dijkstraInit(origin);
    auto final = findVertex(dest);

    if(start == nullptr || final == nullptr)
        return false;

    int i = 0;
    MutablePriorityQueue<Vertex> minQueue; // Create the priority queue and insert the first node
    minQueue.insert(start);

    while(!minQueue.empty()) {
        // Extract the lowest cost vertex
        auto min = minQueue.extractMin();
        min->visited = true;

        // If min equals to the final vertex then end
        if(min == final)
            break;

        // Run though all the neighbours
        for(auto edge : min->adj) {
            auto elem = edge.dest;

            if(elem->visited) continue;
            processedEdges.insert(edge.getId());

            // Relax the adjacent node
            if(elem->dist > min->dist + edge.weight) {
                elem->dist = min->dist + edge.weight;
                elem->path = min;
                elem->edgePath = edge;

                // if elem is not in queue (old dist(w) was infinite)
                if(elem->queueIndex == 0) minQueue.insert(elem);
                else minQueue.decreaseKey(elem);
            }
        }
        for(auto edge : min->invAdj) {
            auto elem = edge.origin;

            if(elem->visited) continue;
            processedEdges.insert(edge.getId());

            if(elem->dist > min->dist + edge.weight) {
                elem->dist = min->dist + edge.weight;
                elem->path = min;
                elem->edgePath = edge;

                // if elem is not in queue (old dist(w) was infinite)
              if(elem->queueIndex == 0) minQueue.insert(elem);
              else minQueue.decreaseKey(elem);
            }
        }
        i++;
    }
    auto stop_time = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop_time - start_time);

    cout << endl;
    cout << "Dijkstra iterations: " << i << endl;
    cout << "Dijkstra path cost: " << final->dist << endl << endl;

    return true;
}

int Graph::getPathTo(const int dest, vector<Edge> &edges) const {
    Vertex *final = findVertex(dest);

    if(final == nullptr || (final->path == nullptr && final->invPath == nullptr))
        return false;

    int dist = final->dist;

    while(final->path != nullptr) {
        edges.push_back(final->getEdgePath());
        final = final->path;

    }
    
    reverse(edges.begin(), edges.end());

    return dist;
}

int Graph::getPathFromCentralTo(const int dest, vector<Edge> &edges) const {
    Vertex *final = findVertex(dest);

    if(final == nullptr || final->pathCentral == nullptr)
        return false;

    int dist = final->distCentral;
    while(final->pathCentral != nullptr) {
        edges.push_back(final->edgePathCentral);
        final = final->pathCentral;
    }

    reverse(edges.begin(), edges.end());

    return dist;
}

/**************** Optimizing Dijkstra ************/

double Graph::heuristicDistance(Vertex *origin, Vertex *dest) {
    return origin->getPosition().euclideanDistance(dest->getPosition());
}

// Uses a heuristic to optimize dijkstra(A*)
bool Graph::dijkstraOrientedSearch(const int origin, const int dest, unordered_set<int> &processedEdges) {
    auto start = dijkstraInit(origin);
    auto final = findVertex(dest);

    if(start == nullptr || final == nullptr)
        return false;

    int i = 0;
    double cost;
    MutablePriorityQueue<Vertex> minQueue;
    minQueue.insert(start);

    while(!minQueue.empty()) {
        auto min = minQueue.extractMin();
        min->visited = true;

        if(min->getId() == final->getId())
            break;

        for(auto edge : min->adj) {
            auto elem = edge.dest;
            int weight = edge.weight;

            if(elem->visited) continue;
            processedEdges.insert(edge.getId());

            if(min->dist + weight < elem->dist){
                elem->path = min;
                elem->edgePath = edge;
                elem->dist = min->dist + weight;
                elem->heuristicValue = elem->dist + heuristicDistance(elem, final);

                // if elem is not in queue (old dist(w) was infinite)
                if(elem->queueIndex == 0) minQueue.insert(elem);
                else minQueue.decreaseKey(elem);
                cost = elem->dist;
            }
        }

        for(auto edge : min->invAdj) {
            auto elem = edge.origin;
            int weight = edge.weight;

            if(elem->visited) continue;
            processedEdges.insert(edge.getId());

            if(min->dist + weight  < elem->dist) {
                elem->path = min;
                elem->edgePath = edge;
                elem->dist = min->dist + weight;
                elem->heuristicValue = elem->dist + heuristicDistance(elem, final);

                // if elem is not in queue (old dist(w) was infinite)
                if(elem->queueIndex == 0) minQueue.insert(elem);
                else minQueue.decreaseKey(elem);
            }
        }
        i++;
    }

    cost = final->dist;

    cout << endl;
    cout << "A* iterations: " << i << endl;
    cout << "A* path cost: " << cost << endl << endl;

    return true;
}

// Upgrades the optimization using a* with bidirectional search
bool Graph::dijkstraBidirectional(const int origin, const int dest, unordered_set<int> &processedEdges, unordered_set<int> &processedEdgesInv) {
    auto start = dijkstraInit(origin);
    auto final = dijkstraBackwardsInit(dest);

    processedEdges.clear();
    processedEdgesInv.clear();

    if(start == nullptr || final == nullptr) return false;

    MutablePriorityQueue<Vertex> forwardMinQueue;
    forwardMinQueue.setInv(false);
    forwardMinQueue.insert(start);

    MutablePriorityQueue<Vertex> backwardMinQueue;
    backwardMinQueue.setInv(true);
    backwardMinQueue.insert(final);

    vector<int> processed;
    vector<int> backward_processed;

    Vertex *forwardMin = nullptr;
    Vertex *backwardMin = nullptr;
    Vertex *middle_vertex = nullptr;

    int i=0;
    double cost;
    // strict alternation between forward and backward search
    while(!forwardMinQueue.empty() && !backwardMinQueue.empty()) {
        //forward search
        forwardMin = forwardMinQueue.extractMin();
        forwardMin->visited = true;
        processed.push_back(forwardMin->id);

        for(auto edge : forwardMin->adj) {
            auto elem = edge.dest;
            auto weight = edge.weight;

            if(elem->visited) continue;
            processedEdges.insert(edge.getId());

            if(forwardMin->dist + weight < elem->dist ) {
                elem->dist = forwardMin->dist + weight;
                elem->path = forwardMin;
                elem->edgePath = edge;
                elem->heuristicValue = elem->dist + heuristicDistance(elem, final);
                elem->inv = false;

                // if elem is not in queue  [old dist(w) was infinite]
                if(elem->queueIndex == 0) forwardMinQueue.insert(elem);
                else forwardMinQueue.decreaseKey(elem);
            }
        }

        for(auto edge : forwardMin->invAdj) {
            auto elem = edge.origin;
            auto weight = edge.weight;

            if(elem->visited) continue;
            processedEdges.insert(edge.getId());

            if(forwardMin->dist + weight < elem->dist ) {
                elem->dist = forwardMin->dist + weight;
                elem->path = forwardMin;
                elem->edgePath = edge;
                elem->heuristicValue = elem->dist + heuristicDistance(elem, final);
                elem->inv = false;

                // if elem is not in queue  [old dist(w) was infinite]
                if(elem->queueIndex == 0) forwardMinQueue.insert(elem);
                else forwardMinQueue.decreaseKey(elem);
            }
        }

        if(find(backward_processed.begin(), backward_processed.end(), forwardMin->id) != backward_processed.end()) {
            middle_vertex = forwardMin;
            break;
        }

        //backward search
        backwardMin = backwardMinQueue.extractMin();
        backwardMin->invVisited = true;
        backward_processed.push_back(backwardMin->id);

        for(auto edge : backwardMin->invAdj) {
            auto elem = edge.origin;
            auto weight = edge.weight;

            if(elem->invVisited) continue;
            processedEdgesInv.insert(edge.getId());

            if(backwardMin->invDist  < elem->invDist) {
                elem->invDist = backwardMin->invDist + weight;
                elem->invHeuristicValue = elem->invDist + heuristicDistance(elem, start);
                elem->invPath = backwardMin;
                elem->invEdgePath = edge;
                elem->inv = true;

                // if elem is not in queue  [old dist(w) was infinite]
                if(elem->invQueueIndex == 0) backwardMinQueue.insert(elem);
                else backwardMinQueue.decreaseKey(elem);

                continue;
            }
        }

        for(auto edge : backwardMin->adj) {
            auto elem = edge.dest;
            auto weight = edge.weight;

            if(elem->invVisited) continue;
            processedEdgesInv.insert(edge.getId());

            if(backwardMin->invDist < elem->invDist) {
                elem->invDist = backwardMin->invDist + weight;
                elem->invHeuristicValue = elem->invDist + heuristicDistance(elem, start);
                elem->invPath = backwardMin;
                elem->invEdgePath = edge;
                elem->inv = true;

                // if elem is not in queue [old dist(w) was infinite]
                if(elem->invQueueIndex == 0) backwardMinQueue.insert(elem);
                else backwardMinQueue.decreaseKey(elem);

                continue;
            }
        }

        if(find(processed.begin(), processed.end(), backwardMin->id) != processed.end()){
            middle_vertex = backwardMin;
            break;
        }
        i+=2;
    }

    int min_dist = middle_vertex->heuristicValue + middle_vertex->invHeuristicValue;

    // the intersected point of the two queues may not be part of the shortest path
    while(!forwardMinQueue.empty()) {
        forwardMin = forwardMinQueue.extractMin();

        if(forwardMin->heuristicValue + forwardMin->invHeuristicValue < min_dist) {
            min_dist = forwardMin->heuristicValue + forwardMin->invHeuristicValue;
            middle_vertex = forwardMin;
        }
    }

    // the intersected point of the two queues may not be part of the shortest path
    while(!backwardMinQueue.empty()) {
        backwardMin = backwardMinQueue.extractMin();

        if(backwardMin->heuristicValue+ backwardMin->invHeuristicValue < min_dist){
            min_dist = backwardMin->heuristicValue+ backwardMin->invHeuristicValue;
            middle_vertex = backwardMin;
        }
    }

    while(middle_vertex->invPath != nullptr) {
        middle_vertex->invPath->path = middle_vertex;
        middle_vertex->invPath->edgePath = middle_vertex->invEdgePath;
        middle_vertex->invPath->dist = middle_vertex->dist + middle_vertex->invEdgePath.getWeight();
        middle_vertex = middle_vertex->invPath;
    }

    cost = middle_vertex->dist;

    cout << endl;
    cout << "A* iterations: " << i << endl;
    cout << "A* path cost: " << cost << endl << endl;

    return true;
}

/**************** All Pairs Shortest Path  ***************/

void Graph::floydWarshallShortestPath() {
    int vertSize = this->vertexSet.size();

    this->minDistance.resize(vertSize);
    for(auto &elem : this->minDistance) elem.resize(vertSize);

    this->next.resize(vertSize);
    for(auto &elem : next) elem.resize(vertSize);

    // this->minDistance -> <vector<vector<double>> that stores distance between i and j
    // this->next        -> <vector<vector<Vertex<T>*>> that stores the path between i and j
    for(int i = 0; i < vertSize; i++) {
        for(int j = 0; j < vertSize; j++) {
            this->minDistance.at(i).at(j) = infinite;
            this->next.at(i).at(j) = nullptr;
        }
    }

    for(Vertex *vertex : this->vertexSet) {
        for(Edge &edge : vertex->adj) {
            int u = -1;
            int v = -1;

            for(int k = 0; k < vertSize; k++) {
                if(this->vertexSet.at(k)->id == vertex->id) u = k;
                if(this->vertexSet.at(k)->id == edge.dest->id) v = k;
            }

            // for each edge dist[u][v] = weight(u, v)
            this->minDistance.at(u).at(v) = edge.weight;
            this->next.at(u).at(v) = edge.dest;
        }
    }

    // for each vertex dist[v][v] = 0
    for(int i = 0; i < vertSize; i++) {
        this->minDistance.at(i).at(i) = 0;
        this->next.at(i).at(i) = this->vertexSet.at(i);
    }

    for(int k = 0; k < vertSize; k++) {
        for (int i = 0; i < vertSize; i++) {
            for(int j = 0; j < vertSize; j++) {
                if(this->minDistance.at(i).at(j) > this->minDistance.at(i).at(k) + this->minDistance.at(k).at(j)) {
                    this->minDistance.at(i).at(j) = this->minDistance.at(i).at(k) + this->minDistance.at(k).at(j);
                    this->next.at(i).at(j) = this->next.at(i).at(k);
                }
            }
        }
    }
}

vector<int> Graph::getfloydWarshallPath(const int orig, const int dest) const {
    vector<int> res;
    int vertSize = this->vertexSet.size();

    Vertex *init, *final;
    init = this->findVertex(orig);
    final = this->findVertex(dest);
    if(init == nullptr || final == nullptr) return res;

    int initIndex, finalIndex;
    for(int i = 0; i < vertSize; i++)
        if(this->vertexSet.at(i)->id == init->id) initIndex = i;

    for(int i = 0; i < vertSize; i++)
        if(this->vertexSet.at(i)->id == final->id) finalIndex = i;

    if(this->next.at(initIndex).at(finalIndex) == nullptr) return res;

    res.push_back(init->id);
    while(init->id != final->id) {
        init = next.at(initIndex).at(finalIndex);
        res.push_back(init->id);
        for(int i = 0; i < vertSize; i++)
            if(this->vertexSet.at(i)->id == init->id) initIndex = i;
    }

    return res;
}

#endif