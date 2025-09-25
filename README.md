# BFS--DFS Route Demo

This project is a **pathfinding visualization tool** that demonstrates
the use of **Breadth-First Search (BFS)** and **Depth-First Search
(DFS)** algorithms on a grid-based map.\
It supports configurable search modes, traversal orders, and multiple
output formats to help analyze search behavior.

------------------------------------------------------------------------

## Features

-   **Two Search Modes**
    -   **Stack-based (DFS)**
    -   **Queue-based (BFS)**
-   **Configurable Hunt Order**
    -   Default: `NESW` (North, East, South, West)
    -   Can be changed via command-line options
-   **Multiple Output Options**
    -   **Map output (`M`)** -- shows the path overlay on the grid
    -   **List output (`L`)** -- prints the list of visited cells
    -   Debugging outputs (`.lst.txt`, `.map.txt`) to trace internal
        states
-   **Statistics Mode**
    -   Tracks nodes expanded, search depth, and solution length
-   **Map File Support**
    -   Input maps in **PPM image** or text-based format
    -   Includes several sample maps and expected solution files

------------------------------------------------------------------------

## Repository Structure

    bfs-dfs-route-demo-main/
    ├── hunt.cpp                # Main program source
    ├── Makefile                # Build instructions
    ├── appA.*                  # Sample maps, outputs, and solutions
    ├── spec.*                  # Specification maps and solutions
    ├── *.sol.txt               # Precomputed solution files
    ├── *.lst.txt               # Debug listing outputs
    ├── *.map.txt               # Pathfinding map outputs
    └── test-*.txt              # Test cases

------------------------------------------------------------------------

## Building

Compile the project using the provided `Makefile`:

``` bash
make
```

This generates the `hunt_debug` (debug build) and possibly other
executables depending on the Makefile rules.

------------------------------------------------------------------------

## Usage

Run the program with a map file:

``` bash
./hunt_debug [options] < mapfile.txt
```

### Options

-   `-c` : Captain's search mode (`stack` for DFS, `queue` for BFS)
-   `-f` : First mate's search mode (stack/queue)
-   `-o` : Hunt order (default `NESW`)
-   `-p` : Show path (`M` for map, `L` for list)
-   `-s` : Show statistics
-   `-v` : Verbose output
-   `-h` : Help message

### Example

``` bash
./hunt_debug -c stack -f queue -o NESW -p M < spec.map.txt
```

This runs DFS as captain, BFS as first mate, exploring directions in
NESW order, and prints the path overlay on the map.

------------------------------------------------------------------------

## Input Format

The input map uses characters to represent terrain:

-   `.` -- Empty space
-   `o` -- Obstacle
-   `#` -- Wall
-   `@` -- Start point
-   `$` -- Goal point

------------------------------------------------------------------------

## Example Run

Input (`spec.map.txt`):

    ########
    #@....$#
    #..##..#
    ########

Command:

``` bash
./hunt_debug -c queue -p M < spec.map.txt
```

Output (`spec.map.out`):

    ########
    #@****$#
    #..##..#
    ########

------------------------------------------------------------------------

## Testing

The repository contains: - **Solution files** (`.sol.txt`) for
verification\
- **Precomputed outputs** (`.out`, `.lst.txt`, `.map.txt`) to compare
behavior

Run the executable with the included test maps to confirm correctness.
