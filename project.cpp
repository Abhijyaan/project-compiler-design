#include <iostream>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>

using namespace std;

// Wait-For Graph (Adjacency List)
unordered_map<int, unordered_set<int>> waitForGraph;
mutex graphMutex;

// Resources (Mutexes)
vector<mutex> resources(6);  // Simulate 5 resources

// Register edge in Wait-For Graph
void addEdge(int fromThread, int toThread) {
    lock_guard<mutex> lg(graphMutex);
    waitForGraph[fromThread].insert(toThread);
}

// Remove edge from graph
void removeEdge(int fromThread, int toThread) {
    lock_guard<mutex> lg(graphMutex);
    waitForGraph[fromThread].erase(toThread);
    if (waitForGraph[fromThread].empty()) {
        waitForGraph.erase(fromThread);
    }
}

// Task simulation by threads (work in progress)
void threadTask(int threadId, int firstRes, int secondRes) {
    resources[firstRes].lock();
    addEdge(threadId, secondRes);

    this_thread::sleep_for(chrono::seconds(1));  // Simulate waiting

    // TODO: Add deadlock detection check here
    // if (detectCycle(...)) handleDeadlock();

    resources[secondRes].lock();
    cout << "Thread " << threadId << " acquired both resources.\n";

    removeEdge(threadId, secondRes);
    resources[secondRes].unlock();
    resources[firstRes].unlock();
}

// TODO: Implement cycle detection using DFS or Tarjan's algorithm
bool detectCycle(int startNode) {
    // Placeholder for cycle detection logic
    return false;
}

int main() {
    vector<thread> threads;

    // Simulating 3 threads with potential deadlock
    threads.emplace_back(threadTask, 1, 1, 2);
    threads.emplace_back(threadTask, 2, 2, 3);
    threads.emplace_back(threadTask, 3, 3, 1);

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    cout << "Simulation complete. Detection module under development.\n";
    return 0;
}
