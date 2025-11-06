#ifndef LOCATIONGRAPH_H
#define LOCATIONGRAPH_H

#include <unordered_map>
#include <vector>
#include <string>

class LocationGraph {
private:
    std::unordered_map<std::string, std::vector<std::pair<std::string, double>>> graph;
    bool initialized = false;

public:
    bool loadFromDatabase(const std::string& dbPath = "areas.db");
    bool areConnected(const std::string& area1, const std::string& area2) const;
    bool isInitialized() const { return initialized; }
};

#endif // LOCATIONGRAPH_H