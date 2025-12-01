# NorDB

NorDB is a lightweight, sector-based database designed for embedded systems. While optimized for NOR flash, it can operate on various storage backends, including RAM and file-based systems, making it versatile for testing and simulation. It provides a simple record-based storage system and is optimized for environments with limited resources.

## Features

- **Wear-Leveling:** Implements wear-leveling mechanisms to distribute writes across flash blocks, extending the life of the storage medium.
- **Flexible Backends:** Supports multiple storage layers, including NOR flash, RAM, and file-based storage, through a hardware abstraction layer (HAL).
- **Partitioning:** Provides capabilities for flash partitioning.
- **Portable:** Written in C for high portability across different embedded platforms.
- **Robust:** Respects erase-before-write constraints and manages data to prevent corruption.
- **Space Reclamation:** Can reclaim space by moving valid data out of blocks selected for erasure (garbage collection).

## How it Works

At a high level, NorDB abstracts the physical storage through a backend interface that requires functions for read, write, erase, and geometry information. It implements a logical-to-physical mapping, which ensures that data updates do not repeatedly target the same flash blocks. By distributing writes, the library reduces uneven wear on the flash memory.

When a block needs to be erased, NorDB can be configured to perform garbage collection, moving valid data to new blocks before erasing the old one, thus reclaiming space and preventing data loss.

## Project Structure

- `src`: Contains the source code for the NorDB library.
- `Demo`: Contains example applications that demonstrate how to use the NorDB library with different hardware configurations (File, RAM, NorFlash).

## Getting Started

To build the project, you'll need CMake. Follow these steps:

1.  Create a build directory:
    ```bash
    mkdir build
    ```

2.  Navigate into the build directory:
    ```bash
    cd build
    ```

3.  Run CMake to configure the project:
    ```bash
    cmake ..
    ```

4.  Compile the project:
    ```bash
    make
    ```

This will build the NorDB library and the example applications located in the `Demo` directory.

## Demos and Tests

The `Demo` directory contains several examples to showcase NorDB's functionality on different backends:

-   **`Demo/File`**: An example of using a standard file as the storage backend. This is useful for testing the database logic on a host machine.
-   **`Demo/Ram`**: Demonstrates how to run NorDB with a RAM-backed storage layer.
-   **`Demo/NorFlash`**: Provides a more complex example for a real NOR flash chip, including a driver for a CH341-based programmer.
-   **`Demo/Tests`**: Contains a suite of tests to verify the core functionality of the database.

After building the project, the compiled examples can be found in the `build/Demo` directory.

## Contributing

Contributions are welcome! If you'd like to contribute, please fork the repository and submit a pull request. For any issues or feature requests, please open an issue on the GitHub repository.

## License

This project is licensed under the MIT License. See the `LICENSE` file for more details.
