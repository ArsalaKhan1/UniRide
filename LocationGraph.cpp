#include "LocationGraph.h"
#include <sqlite3.h>
#include <iostream>

bool LocationGraph::loadFromDatabase(const std::string& dbPath) {
    sqlite3* db;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Cannot open database: " << dbPath << std::endl;
        return false;
    }

    const char* sql = "SELECT area1, area2, distance_km FROM edges";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement" << std::endl;
        sqlite3_close(db);
        return false;
    }

    graph.clear();
    int edgeCount = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string area1 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string area2 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        double distance = sqlite3_column_double(stmt, 2);

        // Add bidirectional edges
        graph[area1].emplace_back(area2, distance);
        graph[area2].emplace_back(area1, distance);
        edgeCount++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    initialized = true;
    std::cout << "Location graph loaded: " << edgeCount << " edges, " 
              << graph.size() << " locations" << std::endl;
    return true;
}

bool LocationGraph::areConnected(const std::string& area1, const std::string& area2) const {
    if (!initialized || area1 == area2) return true;
    
    // Special case for NED Campus - always connected to itself
    if (area1 == "NED Campus" && area2 == "NED Campus") return true;
    
    auto it = graph.find(area1);
    if (it == graph.end()) return false;
    
    for (const auto& edge : it->second) {
        if (edge.first == area2) return true;
    }
    return false;
}