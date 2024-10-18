# Clustering Algorithms Project

This project implements clustering algorithms using the Leiden algorithm, leveraging the `igraph` and `libleidenalg` libraries. It includes all necessary dependencies as Git submodules and provides a build script to automate the compilation process. The main goal is making connection between this project and Arachne.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgments](#acknowledgments)

## Introduction

The Clustering Algorithms Project provides a framework for performing community detection on graphs using the Leiden algorithm. It is suitable for researchers and developers working on network analysis, graph theory, and data clustering.

## Features

- **Leiden Algorithm Implementation**: Efficient community detection using the `Leiden algorithm 0.11.1`.
- **Graph Creation**: Supports custom graph creation using `igraph 0.10.13`.
- **Automated Build Script**: Simplifies the build process with a single script.
- **Modular Codebase**: Easy to extend and modify for custom applications.

## Prerequisites

- **Operating System**: Linux or macOS (the build script is designed for Unix-like systems).
- **Git**: Version control system to clone the repository.
- **CMake**: Version 3.5 or higher.
- **C++ Compiler**: Supporting C++17 (e.g., GCC or Clang).
- **Shell**: Ability to run shell scripts (`bash`).

## Installation

### 1. Clone the Repository

Clone the repository along with its submodules:

```bash
git clone --recurse-submodules https://github.com/mdindoost/Clustering_Algorithms.git
cd Clustering_Algorithms
```
If you have already cloned the repository without submodules, initialize them:
```bash
git submodule update --init --recursive
```

### 2. Run the Build Script
Make sure the build.sh script has execute permissions:

```bash

chmod +x build.sh
```

Execute the build script:
```bash

./build.sh
```

**Note:** If you need to rebuild dependencies (igraph and libleidenalg), run:

```bash

./build.sh --rebuild-deps
```
### 3. Load Necessary Modules (If Applicable)
If you're on a system that uses environment modules (e.g., a computing cluster like Kruskal), you may need to load modules before building:

```bash

module load cmake
module load gcc
```
## Usage
After building, you can run the application to perform clustering on a sample graph (e.g., Zachary's Karate Club):

Explanation and Instructions
##1. Configuration Options
The program supports the following configuration options:

**Resolution Parameter** (```--resolution``` or ```-r```): Controls the granularity of the communities. Higher values lead to smaller communities.

  **Partition Type** (``` --partition``` or ``` -p```): Selects the type of partition to use. Options are:

- CPM (default)
- RBConfiguration
- Modularity

**Random Seed** (```--seed``` or ```-s```): Sets the seed for random number generation to ensure reproducibility.

**Iterations** (```--iterations``` or ```-i```): Specifies the number of iterations for the Leiden algorithm. Use -1 to run until convergence (default).

##2. Command-Line Arguments
The program accepts command-line arguments to set the configuration options. For example:

```bash

./build/src/clustering_app --resolution 0.5 --partition Modularity --seed 123 --iterations 10
```
Or using the shorthand options:

```bash

./build/src/clustering_app -r 0.5 -p Modularity -s 123 -i 10
```
**Note:** If you encounter any library-related errors, ensure that the LD_LIBRARY_PATH environment variable includes the paths to the libraries:

```bash

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/igraph/install/lib64:$(pwd)/libleidenalg/install/lib64
```

## Contributing
Contributions are welcome! If you would like to contribute to this project, please follow these steps:

Fork the Repository: Create a personal fork of the repository on GitHub.
Clone Your Fork: Clone your fork to your local machine.
Create a Feature Branch: Create a new branch for your feature or bug fix.
Commit Your Changes: Make your changes and commit them with clear messages.
Push to Your Fork: Push your changes to your fork on GitHub.
Submit a Pull Request: Open a pull request to the main repository.
Please ensure that your code adheres to the project's coding standards and includes appropriate documentation.

### License
This project is licensed under the GNU General Public License v3.0.

### Acknowledgments
**igraph:** The project uses the igraph library for graph manipulation.
**libleidenalg:** The Leiden algorithm implementation is provided by libleidenalg.
V.A. Traag: Author of the Leiden algorithm and libleidenalg.
Community: Thanks to everyone who has contributed to the open-source libraries used in this project.
Contact Information

For any questions or issues, please open an issue on the GitHub repository or contact me at md724@njit.edu.


