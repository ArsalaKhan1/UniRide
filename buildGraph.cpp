#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <sqlite3.h>
#include <sstream>

struct Location {
    std::string name;
    double lat;
    double lon;
};

// Haversine distance (in km)
double haversine(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371.0; // Earth radius in km
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
               sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}

// Create table if not exists
void createTable(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS edges ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "area1 TEXT NOT NULL, "
        "area2 TEXT NOT NULL, "
        "distance_km REAL NOT NULL"
        ");";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Error creating table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "✅ Table 'edges' created/verified successfully." << std::endl;
    }
}

int main() {
    // STEP 1: Read CSV
    std::ifstream file("locations.csv");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open locations.csv" << std::endl;
        return 1;
    }

    std::vector<Location> locations;
    std::string line;
    
    // Skip header line
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string name, latStr, lonStr;
        
        // Parse CSV: name,lat,lon
        if (std::getline(ss, name, ',') && 
            std::getline(ss, latStr, ',') && 
            std::getline(ss, lonStr)) {
            
            try {
                double lat = std::stod(latStr);
                double lon = std::stod(lonStr);
                locations.push_back({name, lat, lon});
            } catch (const std::exception& e) {
                std::cerr << "Error parsing line: " << line << std::endl;
            }
        }
    }

    std::cout << "Loaded " << locations.size() << " locations." << std::endl;

    // STEP 2: Open SQLite DB
    sqlite3* db;
    if (sqlite3_open("areas.db", &db) != SQLITE_OK) {
        std::cerr << "Cannot open database." << std::endl;
        return 1;
    }

    createTable(db);

    // STEP 3: Compute and insert distances
    const double MAX_DIST = 4.0; // km threshold - STRICT for nearby areas only
    int count = 0;

    for (size_t i = 0; i < locations.size(); ++i) {
        for (size_t j = i + 1; j < locations.size(); ++j) {
            double dist = haversine(
                locations[i].lat, locations[i].lon,
                locations[j].lat, locations[j].lon
            );

            if (dist <= MAX_DIST) {
                std::string sql = "INSERT INTO edges (area1, area2, distance_km) VALUES ('" +
                                  locations[i].name + "', '" +
                                  locations[j].name + "', " +
                                  std::to_string(dist) + ");";

                if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK) {
                    count++;
                    std::cout << "Added edge: " << locations[i].name << " - " << locations[j].name 
                              << " (" << dist << " km)" << std::endl;
                } else {
                    std::cerr << "Failed to insert edge: "
                              << locations[i].name << " - " << locations[j].name << std::endl;
                }
            }
        }
    }

    sqlite3_close(db);
    std::cout << "✅ Graph built successfully. " << count
              << " nearby edges stored in areas.db" << std::endl;

    return 0;
}