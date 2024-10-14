# Clustering Algorithms Project

This project implements clustering algorithms using the Leiden algorithm, leveraging the `igraph` and `libleidenalg` libraries. It includes all necessary dependencies as Git submodules and provides a build script to automate the compilation process.

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

- **Leiden Algorithm Implementation**: Efficient community detection using the Leiden algorithm.
- **Graph Creation**: Supports custom graph creation using `igraph`.
- **Automated Build Script**: Simplifies the build process with a single script.
- **Modular Codebase**: Easy to extend and modify for custom applications.

## Prerequisites

- **Operating System**: Linux or macOS (the build script is designed for Unix-like systems).
- **Git**: Version control system to clone the repository.
- **CMake**: Version 3.5 or higher.
- **C++ Compiler**: Supporting C++11 (e.g., GCC or Clang).
- **Shell**: Ability to run shell scripts (`bash`).

## Installation

### 1. Clone the Repository

Clone the repository along with its submodules:

```bash
"git clone --recurse-submodules https://github.com/mdindoost/Clustering_Algorithms.git"
cd Clustering_Algorithms

If you have already cloned the repository without submodules, initialize them:

git submodule update --init --recursive

### 2. Run the Build Script
Make sure the build.sh script has execute permissions:


chmod +x build.sh
Execute the build script:

./build.sh
Note: If you need to rebuild dependencies (igraph and libleidenalg), run:


./build.sh --rebuild-deps
3. Load Necessary Modules (If Applicable)
If you're on a system that uses environment modules (e.g., a computing cluster like Kruskal), you may need to load modules before building:


module load cmake
module load gcc

Usage
After building, you can run the application to perform clustering on a sample graph (e.g., Zachary's Karate Club):

Option 1: Run from the Project Root

./build/src/clustering_app

Option 2: Run from the Build Directory

cd build
./src/clustering_app
Note: If you encounter any library-related errors, ensure that the LD_LIBRARY_PATH environment variable includes the paths to the libraries:


export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/igraph/install/lib64:$(pwd)/libleidenalg/install/lib

Contributing
Contributions are welcome! If you would like to contribute to this project, please follow these steps:

Fork the Repository: Create a personal fork of the repository on GitHub.
Clone Your Fork: Clone your fork to your local machine.
Create a Feature Branch: Create a new branch for your feature or bug fix.
Commit Your Changes: Make your changes and commit them with clear messages.
Push to Your Fork: Push your changes to your fork on GitHub.
Submit a Pull Request: Open a pull request to the main repository.
Please ensure that your code adheres to the project's coding standards and includes appropriate documentation.

License
This project is licensed under the GNU General Public License v3.0.

Acknowledgments
igraph: The project uses the igraph library for graph manipulation.
libleidenalg: The Leiden algorithm implementation is provided by libleidenalg.
V.A. Traag: Author of the Leiden algorithm and libleidenalg.
Community: Thanks to everyone who has contributed to the open-source libraries used in this project.
Contact Information

For any questions or issues, please open an issue on the GitHub repository or contact me at md724@njit.edu.


