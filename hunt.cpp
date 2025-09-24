// IDENTIFIER  = 40FB54C86566B9DDEAB902CC80E8CE85C1C62AAD
#include <algorithm>
#include <deque>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <sstream>

using namespace std;

// Enums for mode and terrain
enum class Mode {
    kNone = 0,
    stack,
    queue
};

struct Options {  
    Mode captain_mode = Mode::stack;
    Mode first_mate_mode = Mode::queue;
    string hunt_order = "NESW"; // Default order
    char show_path = '\0';      // 'M' for map, 'L' for list
    bool verbose = false;
    bool stats = false;
};

struct Coords{
    int x; 
    int y; 
};

struct Point {
    ///int x = -1; 
    ///int y = -1;
    char direction = ' '; // Direction from the parent cell
    char value = '.';           // Terrain type ('.', 'o', '#', '@', '$')
};

// Function prototypes
void printHelp(const char *command);
void parseOptions(int argc, char **argv, Options &options);
void loadMap(istream& input, vector<vector<Point>> &map, int &map_size, Coords &start, Coords &treasure);
void huntTreasure(const Options &options, vector<vector<Point>> &map, Coords &start, Coords &treasure);
void first_mate_hunt(Coords &start_location, const Options &options, vector<vector<Point>> &map, 
                            Coords &treasure, int &land_investigated);

void showStats(const Options &options, const Coords &start, const Coords &treasure,
               int water_investigated, int land_investigated, int went_ashore, int path_length);

int backtraceAndShowPath(const Options &options, Coords &start, 
                         const Coords &treasure, vector<vector<Point>> &map);

bool is_valid_move(int &row, int &col, char &dir, vector<vector<Point>> &map);




bool treasure_found = false;
int water_investigated = 0;
int land_investigated = 0;
int went_ashore = 0;
//deque<pair<int, int>> captain_container;


//can delete comments
//void printMap(const vector<vector<Point>> &map, int map_size, const Coords &start, const Coords &treasure);


int main(int argc, char **argv) {
    ios_base::sync_with_stdio(false);

    Options options;
    parseOptions(argc, argv, options);

    vector<vector<Point>> map;
    int map_size = 0;


    Coords start, treasure; 
    loadMap(cin, map, map_size, start, treasure);   //need help on loadmap//took away start and treasure
    
    //TESTING
    //printMap(map, map_size, start, treasure);

    
    huntTreasure(options, map, start, treasure);


    if (treasure_found) {
         // backtraceAndShowPath(options, start, treasure, map);
        int path_length = backtraceAndShowPath(options, start, treasure, map);

        //used to be show stats
        cout << "Treasure found at " << treasure.x << "," << treasure.y
                << " with path length " << path_length << ".\n";
    }
    //CONSIDER MOVING THIS
    if(!treasure_found){
        if(options.verbose){
            cout << "Treasure hunt failed\n";
        }
        showStats(options, start, {-1, -1}, water_investigated, land_investigated, went_ashore, -1);
        backtraceAndShowPath(options, start, treasure, map);  //FOR TEST H02
        cout << "No treasure found after investigating " << water_investigated + land_investigated << " locations.\n";  //+land for H05
    }
    return 0;
}

// Prints the help message
void printHelp(const char *command) {
    cout << "Usage: " << command << " [options] < inputfile\n";
    cout << "Options:\n";
    cout << "  -h, --help              Show this help message and exit\n";
    cout << "  -c, --captain <STACK|QUEUE>  Set the Captain's routing container\n";
    cout << "  -f, --first-mate <STACK|QUEUE>  Set the First Mate's routing container\n";
    cout << "  -o, --hunt-order <ORDER>  Set the hunt order (e.g., NESW)\n";
    cout << "  -p, --show-path <M|L>   Show the path as a Map (M) or List (L)\n";
    cout << "  -v, --verbose           Enable verbose output\n";
    cout << "  -s, --stats             Display search statistics\n";
}

// Parses command-line options
void parseOptions(int argc, char **argv, Options &options) {
    int choice;
    int index = 0;
    bool print = false;
    static struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"captain", required_argument, nullptr, 'c'},
        {"first-mate", required_argument, nullptr, 'f'},
        {"hunt-order", required_argument, nullptr, 'o'},
        {"show-path", required_argument, nullptr, 'p'},
        {"verbose", no_argument, nullptr, 'v'},
        {"stats", no_argument, nullptr, 's'},
        {nullptr, 0, nullptr, '\0'}
    };

    while ((choice = getopt_long(argc, argv, "hc:f:o:p:vs", long_options, &index)) != -1) {
        switch (choice) {
        case 'h':
            printHelp(argv[0]);
            exit(0);
        case 'c':
            if (string(optarg) == "STACK") {
                options.captain_mode = Mode::stack;
            } else if (string(optarg) == "QUEUE") {
                options.captain_mode = Mode::queue;
            } else {
                cerr << "Invalid argument to --captain\n";
                exit(1);
            }
            break;
        case 'f':
            if (string(optarg) == "STACK") {
                options.first_mate_mode = Mode::stack;
            } else if (string(optarg) == "QUEUE") {
                options.first_mate_mode = Mode::queue; 
            } else {
                cerr << "Invalid argument to --first-mate\n";
                exit(1);
            }
            break;
        case 'o':
            if (optarg && string(optarg).length() == 4 &&
                string(optarg).find('N') != string::npos &&
                string(optarg).find('E') != string::npos &&
                string(optarg).find('S') != string::npos &&
                string(optarg).find('W') != string::npos) {
                options.hunt_order = optarg;
            } else {
                cerr << "Invalid argument to --hunt-order\n";
                exit(1);
            }
            break;
        case 'p':
            if ((string(optarg) == "M" || string(optarg) == "L") && !print) {
                print = true;
                options.show_path = optarg[0];

            } else {
                cerr << "Invalid argument to --show-path\n";
                exit(1);
            }
            break;
        case 'v':
            options.verbose = true;
            break;
        case 's':
            options.stats = true;
            break;
        default:
            cerr << "Unknown option\n";
            exit(1);
        }
    }
}

// Loads the map from an input file
void loadMap(istream& input, vector<vector<Point>> &map, int &map_size, Coords &start, Coords &treasure) {
    string line;
    char file_type = '\0';
    while (getline(input, line)) {
        if (line.empty() || line[0] == '#') {
            continue; // Skip comments
        }

        if (file_type == '\0') {
            file_type = line[0];
            continue;
        }

        if (map_size == 0) {
            map_size = stoi(line);
            map.resize(map_size, vector<Point>(map_size));
            //continue;
        }

        if (file_type == 'M') {
            for (int i = 0; i < map_size; ++i) {
                getline(input, line);
                for (int j = 0; j < map_size; ++j) {
                    map[i][j].value = line[j];
                    if (line[j] == '@') {start = {i, j};} 
                    else if (line[j] == '$') {treasure = {i, j};}
                }
            }
            return;
        }



        else if (file_type == 'L') {
            int row, col;   
            char terrain;
            //map.resize(map_size, vector<Point>(map_size));
            // for (int i = 0; i < map_size; ++i) {
            //     for (int j = 0; j < map_size; ++j) {
            //         map[i][j].value = '.';                  
            //     }
            // }

            while (getline(input, line)) { 
                if (line.empty()) continue;
                istringstream iss(line);
                iss >> row >> col >> terrain;
                map[row][col].value = terrain;
                if (terrain == '@') start = {row, col};
                if (terrain == '$') treasure = {row, col};
            }
            break;
        }
    }
}


void addToContainer(deque<Coords> &container, Mode mode, Coords &start){
    if (mode == Mode::stack) {
        container.push_back(start);
    } else {
        container.push_front (start);
    }
}



bool is_valid_move(int &row, int &col, char &dir, vector<vector<Point>> &map){
    
    int size = static_cast<int>(map.size());
    
    return (dir == 'N' && row > 0) || (dir == 'S' && row < size - 1) ||
       (dir == 'E' && col < size - 1) || (dir == 'W' && col > 0);


    // if (dir == 'N') 
    // return (row - 1 >= 0);

    // else if (dir == 'W') 
    // return (col - 1 >= 0);

    // else if (dir == 'S') 
    // return (row + 1 < size);

    // else if (dir == 'E') 
    // return (col + 1 < size);

    // return false;
}


void huntTreasure(const Options &options, vector<vector<Point>> &map, Coords &start, 
                                                                    Coords &treasure) {
    deque<Coords> captain_container;

    addToContainer(captain_container, options.captain_mode, start);
    //map[start.first][start.second].direction = '@';
    
    if (options.verbose) {
        cout << "Treasure hunt started at: " << start.x << "," << start.y << "\n";
    }
    
    Coords current;

    while (!captain_container.empty() && !treasure_found) {
        current = captain_container.back();
        captain_container.pop_back();
        water_investigated++;
    

        for (char dir : options.hunt_order) {
            Coords next = current;
            //int nx = current.first + (dir == 'N' ? -1 : dir == 'S' ? 1 : 0);
            //int ny = current.second + (dir == 'E' ? 1 : dir == 'W' ? -1 : 0);
            
            if(is_valid_move(next.x, next.y, dir, map)){
                //move 
                if(dir == 'N')
                next.x--;   

                else if(dir == 'S')
                next.x++;

                else if(dir == 'E')
                next.y++;

                else if(dir == 'W')
                next.y--;

                if (map[next.x][next.y].value == '.' && map[next.x][next.y].direction == ' ') {
                    map[next.x][next.y].direction = dir;                
                    addToContainer(captain_container, options.captain_mode, next);
                } 
                
                else if (map[next.x][next.y].value == 'o' && map[next.x][next.y].direction == ' ') {    //added $ to making it to first mate
                    ++went_ashore;
                    if (options.verbose) {
                        cout << "Went ashore at: " << next.x << "," << next.y << "\n";
                    }   

                    map[next.x][next.y].direction = dir;   //REVIEW THIS
                    //jump to first mate function
                    first_mate_hunt(next, options, map, treasure, land_investigated); 

                } 

                else if (map[next.x][next.y].value == '$' && map[treasure.x][treasure.y].direction == ' ' ){
                    treasure_found = true;
                    went_ashore++;
                    land_investigated++;
                    if(options.verbose) {
                        cout << "Went ashore at: " << next.x << "," << next.y << "\n"
                          << "Searching island... party found treasure at " << next.x << "," << next.y << ".\n"; 
                    }
                    map[treasure.x][treasure.y].direction = dir;
                    break;
                }
            }

           
    }
}
                                                                        }




void first_mate_hunt(Coords &start_location, const Options &options, vector<vector<Point>> &map,
                                        Coords &treasure, int &land_investigated) {        
    
    deque<Coords> first_mate_container;
    addToContainer(first_mate_container, options.first_mate_mode, start_location);
    //map[start_location.first][start_location.second].direction = ' ';

    while (!first_mate_container.empty() && treasure_found == false) {
        Coords current = first_mate_container.back();
        start_location = current;
        first_mate_container.pop_back();
        land_investigated++;

        for(char dir: options.hunt_order){

            Coords land_next = start_location;
            
            //int ln = land_current.first + (dir == 'N' ? -1 : dir == 'S' ? 1 : 0);
            //int lm = land_current.second + (dir == 'E' ? 1 : dir == 'W' ? -1 : 0);

            //char land_terrain = map[ln][lm];
            

            if(is_valid_move(land_next.x, land_next.y, dir, map)){
                if(dir == 'N')
                land_next.x--;   

                else if(dir == 'S')
                land_next.x++;

                else if(dir == 'E')
                land_next.y++;

                else if(dir == 'W')
                land_next.y--;

                
                if ((map[land_next.x][land_next.y].value == 'o' || map[land_next.x][land_next.y].value == '$') && map[land_next.x][land_next.y].direction == ' ') {   
                    
                    map[land_next.x][land_next.y].direction = dir;
                    //map[search_point.x][search_point.y].discovered = true; 
                    addToContainer(first_mate_container, options.first_mate_mode, land_next);

                    if(map[land_next.x][land_next.y].value == '$'){ 
                        //map[land_next.x][land_next.y].direction == dir;
                        land_investigated++;
                        treasure_found = true;
                        if (options.verbose) {
                            cout << "Searching island... party found treasure at " << land_next.x << "," << land_next.y << ".\n";
                        }
                        break;
                    } 
                }
            }
        }
    }
    if (!treasure_found){
        if (options.verbose && map[treasure.x][treasure.y].direction == ' ') {
        cout << "Searching island... party returned with no treasure.\n";
        } 
    }

}


void showStats(const Options &options, const Coords &start, const Coords &treasure,
               int water_investigated, int land_investigated, int went_ashore, int path_length) {
    if (!options.stats) return;

    cout << "--- STATS ---\n";
    cout << "Starting location: " << start.x << "," << start.y << "\n";
    cout << "Water locations investigated: " << water_investigated << "\n";
    cout << "Land locations investigated: " << land_investigated << "\n";
    cout << "Went ashore: " << went_ashore << "\n";

    if (path_length != -1) {
        cout << "Path length: " << path_length << "\n";
        cout << "Treasure location: " << treasure.x << "," << treasure.y << "\n";
    }
    cout << "--- STATS ---\n";
}

 

int backtraceAndShowPath(const Options &options, Coords &start, 
                         const Coords &treasure, vector<vector<Point>> &map) {
    if(!treasure_found)
    return 0;
    
    vector<Coords> path; // To store the traced path
    Coords current = treasure;

    // Backtrace the path using the `direction` field
    while (current.x != start.x || current.y != start.y) {
        path.push_back(current);
        char dir = map[current.x][current.y].direction;

        if (dir == 'N') current.x++;
        else if (dir == 'S') current.x--;
        else if (dir == 'E') current.y--;
        else if (dir == 'W') current.y++;
    }
    path.push_back(start);

    // Reverse the path to get it from start to treasure
    reverse(path.begin(), path.end());


    int pathsize = int(path.size() - 1);

    showStats(options, start, treasure, water_investigated, land_investigated, went_ashore, pathsize);


    // Display the path based on the selected mode
    if (options.show_path == 'M') {
        // vector<string> treasure_map(map.size(), string(map.size(), '.'));
        // for (size_t i = 0; i < map.size(); ++i) {
        //     for (size_t j = 0; j < map[i].size(); ++j) {
        //         treasure_map[i][j] = map[i][j].value;
        //     }
        // }
        for (size_t i = 1; i < path.size(); ++i) {
            auto [x1, y1] = path[i - 1];
            auto [x2, y2] = path[i];

            // Determine the direction of the current segment
            if (x1 == x2) {
                map[x1][y1].value = '-'; // Horizontal  //min(y1, y2)
            } else if (y1 == y2) {
                map[x2][y2].value = '|'; // Vertical min(x1, x2)
            }

            // Add corners when direction changes
            if (i > 1) {
                auto [x0, y0] = path[i - 2];
                if ((x0 != x1 && y1 != y2) || (y0 != y1 && x1 != x2)) {
                    map[x1][y1].value = '+'; // Mark as a corner
                }
            }
    }

        map[start.x][start.y].value = '@';
        map[treasure.x][treasure.y].value = 'X';

        for (const auto &row : map) {
            for (const auto &point : row) {
                cout << point.value;
            }
            cout << "\n";
        }
        
    } else if (options.show_path == 'L') {
        // Output the path as a coordinate list
        cout << "Sail:\n";
        for (size_t i = 0; i < path.size(); ++i) {
            if (map[path[i].x][path[i].y].value == 'o' || i == path.size() - 1) {
                break;
            }
            cout << path[i].x << "," << path[i].y << "\n";
        }

        cout << "Search:\n";
        for (size_t i = 0; i < path.size(); ++i) {
            if (map[path[i].x][path[i].y].value == 'o' || i == path.size() - 1) {
                for (size_t j = i; j < path.size(); ++j) {
                    cout << path[j].x << "," << path[j].y << "\n";
                }
                break;
            }
        }
    }

    return pathsize; // Path length (excluding start and treasure)
}



