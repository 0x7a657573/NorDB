# NorDB

## Project Overview

NorDB is a lightweight, sector-based FIFO (First-In-First-Out) queue database specifically designed for NOR flash memory in embedded systems. It solves the problem of reliably storing sequential records (logs, sensor data, events) on flash storage while respecting NOR flash constraints and distributing wear across sectors.

### Core Problem Solved

NOR flash requires erasing before writing (erase sets all bits to 1, writes can only flip 1→0), and has limited erase cycles (~100K-1M per block). NorDB provides a circular queue abstraction that:
*   Manages the erase-before-write cycle automatically.
*   Distributes writes across all available sectors to maximize flash lifespan (wear-leveling).
*   Provides a simple `AddRecord`/`ReadRecord` interface, hiding the complexities of flash management.
*   Ensures data integrity with CRC checks and power-loss recovery.

### Real-World Use Cases

*   Sensor data logging (temperature, accelerometer readings)
*   Event buffers (system events, error logs)
*   Message queues in embedded systems
*   Persistent circular buffers for telemetry
*   Any scenario requiring durable FIFO storage with limited flash wear


## Key Features

*   **Wear-Leveling:** NorDB implements a wear-leveling mechanism to distribute writes across the flash blocks, which helps to extend the life of the storage medium.
*   **Multi-Backend Support:** NorDB supports multiple storage backends, including NOR flash, RAM, and file-based systems. This allows for flexible testing and simulation, as well as easy adaptation to different hardware platforms.
*   **Flash Partitioning:** NorDB provides the ability to partition the flash memory, allowing for multiple databases to coexist on the same chip.
*   **Optimized for Flash:** NorDB is optimized for use with NOR flash memory, taking into account the specific characteristics of this type of memory, such as the need for erase-before-write and the limited number of erase cycles.
*   **Thread-Safe:** NorDB is designed to be thread-safe, using a semaphore to protect critical sections of the code.

## Concepts/Terminology

*   **Block/Page:** In the context of NorDB, a block is a contiguous region of memory that can be erased as a single unit. A page is a smaller unit of memory that can be written to.
*   **Erase-before-write:** NOR flash memory has the characteristic that it can only be written to after it has been erased. This means that before writing to a block, it must first be erased, which sets all the bits in the block to 1.
*   **Metadata:** NorDB stores metadata in the header of each sector, which includes a magic number, version, sync counter, and record size. This metadata is used to validate the integrity of the data and to synchronize the database state.


## Architecture and Module Organization

NorDB has a layered architecture that separates the core database logic from the underlying storage medium.

```
+---------------------+
|    Application      |
+---------------------+
          |
          v
+---------------------+
|    Public API       |
|    (NorDB.h)        |
+---------------------+
          |
          v
+---------------------+
|    Core Logic       |
|    (NorDB.c)        |
+---------------------+
          |
          v
+---------------------+
|         HAL         |
|   (NorDB_HWLayer)   |
+---------------------+
          / | \
         /  |  \
        /   |   \
       /    |    \
      v     v     v
+--------+-------+--------+
| Flash  |  RAM  | File   |
| Backend|       | Backend| 
+--------+-------+--------+
```


*   **Public API:** The public API provides a set of functions for interacting with the database, such as `NorDB_AddRecord`, `NorDB_ReadRecord`, and `NorDB_Clear`.
*   **Core Logic:** The core logic implements the main functionality of the database, such as record management, wear-leveling, and data synchronization.
*   **Hardware Abstraction Layer (HAL):** The HAL provides a generic interface to the underlying storage medium. It consists of a set of function pointers for performing low-level operations, such as `SectorErace`, `WriteBuffer`, and `ReadBuffer`.
*   **Storage Backends:** NorDB supports multiple storage backends, each of which implements the HAL interface for a specific type of storage medium (e.g., NOR flash, RAM, file).


## Getting Started

### Build Instructions

The project uses CMake as its build system. To build the project, you will need to have CMake and a C compiler installed.

1.  Create a build directory: `mkdir build`
2.  Navigate to the build directory: `cd build`
3.  Run CMake to configure the project: `cmake ..`
4.  Build the project: `make`

This will build the `NorDB` static library and the demo applications.

### Dependencies

The project has the following dependencies:

*   **pthreads:** The `pthreads` library is required for thread safety.
*   **libusb-1.0:** The `libusb-1.0` library is required for the `NorFlash_ex` demo application, which uses a CH341 USB to SPI adapter to communicate with the NOR flash chip.

### Minimal Working Example

Here is a minimal working example of how to use NorDB with the file backend:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NorDB.h"
#include "ll/File_ll.h"

typedef struct  __attribute__((__packed__))
{
    uint8_t  dummy[11];
    uint32_t checksum;
} dummy_t;

int main()
{
    // Initialize the file backend
    NorDB_HWLayer *file_hw = Filell_Init("Nor.db", 4096, 4);
    if (!file_hw) {
        printf("Error initializing file backend\n");
        return 1;
    }

    // Initialize the database
    NorDB_t *db = NorDB(file_hw, sizeof(dummy_t));
    if (!db) {
        printf("Error initializing database\n");
        return 1;
    }

    // Add a record
    dummy_t record;
    record.checksum = 0;
    for (int i = 0; i < 11; i++) {
        record.dummy[i] = i;
        record.checksum += record.dummy[i];
    }
    if (NorDB_AddRecord(db, &record) == 0) {
        printf("Error adding record\n");
        return 1;
    }

    // Read a record
    dummy_t read_record;
    if (NorDB_ReadRecord(db, &read_record) == 0) {
        printf("Error reading record\n");
        return 1;
    }

    // Verify the record
    uint32_t sum = 0;
    for (int i = 0; i < 11; i++) {
        sum += read_record.dummy[i];
    }
    if (sum != read_record.checksum) {
        printf("Record checksum error\n");
        return 1;
    }

    printf("Success!\n");

    return 0;
}
```

## Public API Reference

The public API is defined in `src/include/NorDB.h`.

*   **`NorDB_t *NorDB(NorDB_HWLayer *hw, uint16_t RecordSize)`:** Initializes a new database instance.
    *   `hw`: A pointer to the hardware abstraction layer.
    *   `RecordSize`: The size of a single record.
    *   Returns a pointer to the new database instance, or `NULL` on error.
*   **`bool NorDB_Clear(NorDB_t *db)`:** Erases the entire database.
    *   `db`: A pointer to the database instance.
    *   Returns `true` on success, or `false` on error.
*   **`uint32_t NorDB_AddRecord(NorDB_t *db, void *RecoedData)`:** Adds a new record to the database.
    *   `db`: A pointer to the database instance.
    *   `RecoedData`: A pointer to the record data.
    *   Returns the physical address of the new record, or `0` on error.
*   **`uint32_t NorDB_ReadRecord(NorDB_t *db, void *RecoedData)`:** Reads a record from the database.
    *   `db`: A pointer to the database instance.
    *   `RecoedData`: A pointer to a buffer to store the record data.
    *   Returns the physical address of the record, or `0` on error.
*   **`uint32_t NorDB_Get_TotalUnreadRecord(NorDB_t *db)`:** Gets the number of unread records in the database.
    *   `db`: A pointer to the database instance.
    *   Returns the number of unread records.
*   **`uint32_t NorDB_Get_FreeRecord(NorDB_t *db)`:** Gets the number of free records in the database.
    *   `db`: A pointer to the database instance.
    *   Returns the number of free records.

##  Backends

### Flash Backend

The flash backend is implemented in `src/ll/SerialNorFlash_ll.c`. It is designed to work with Winbond NOR flash chips and uses an SPI interface to communicate with the chip.

The backend automatically detects the flash chip by reading its JEDEC ID. It supports a range of Winbond chips, from W25Q10 to W25Q512.

The flash backend assumes the following flash geometry:

*   **Page Size:** 256 bytes
*   **Sector Size:** 4096 bytes (4 KB)
*   **Block Size:** 65536 bytes (64 KB)

To use the flash backend, you need to provide an SPI implementation by creating a `SpiBus_t` struct and setting the `SPI_WriteRead` function pointer.

### RAM Backend

The RAM backend is implemented in `src/ll/Ram_ll.c`. It uses a buffer in RAM as the storage medium. This backend is useful for testing and simulation, as it is fast and does not require any special hardware.

To use the RAM backend, you need to call the `Ramll_Init` function, specifying the sector size and total number of sectors.

### File Backend

The file backend is implemented in `src/ll/File_ll.c`. It uses a file on the host machine as the storage medium. This backend is also useful for testing and simulation, as it allows you to inspect the contents of the database using a hex editor.

To use the file backend, you need to call the `Filell_Init` function, specifying the path to the file, the sector size, and the total number of sectors.

### Implementing a Custom Backend

To implement a custom backend, you need to create a `NorDB_HWLayer` struct and set the function pointers for the following functions:

*   **`SectorErace`:** Erases a sector.
*   **`WriteBuffer`:** Writes a buffer of data to the storage medium.
*   **`ReadBuffer`:** Reads a buffer of data from the storage medium.
*   **`DriverCheck`:** Checks if the driver is initialized and ready.
*   **`DriverName`:** Returns the name of the driver.

Here is a skeleton for a custom backend:

```c
#include "NorDB.h"

void MyBackend_SectorErace(void *Param, uint32_t address) {
    // Erase a sector on your custom hardware
}

void MyBackend_WriteBuffer(void *Param, uint32_t address, uint8_t *data, uint16_t len) {
    // Write data to your custom hardware
}

void MyBackend_ReadBuffer(void *Param, uint32_t address, uint8_t *data, uint16_t len) {
    // Read data from your custom hardware
}

uint8_t MyBackend_DriverCheck(void *Param) {
    // Check if your custom hardware is ready
    return 1;
}

const char *MyBackend_DriverName(void *Param) {
    return "MyCustomBackend";
}

NorDB_HWLayer *MyBackend_Init() {
    NorDB_HWLayer *hw = (NorDB_HWLayer *)malloc(sizeof(NorDB_HWLayer));
    if (!hw) {
        return NULL;
    }

    // Initialize your custom hardware
    // ...

    hw->SectorSize = 4096;
    hw->SectorNumber = 4;
    hw->Param = NULL; // Or a pointer to your custom hardware context
    hw->SectorErace = MyBackend_SectorErace;
    hw->WriteBuffer = MyBackend_WriteBuffer;
    hw->ReadBuffer = MyBackend_ReadBuffer;
    hw->DriverCheck = MyBackend_DriverCheck;
    hw->DriverName = MyBackend_DriverName;

    return hw;
}
```


## Wear-leveling internals

### Record State Machine

The state of each record is tracked by a 2-bit status field in the sector header. The state transitions are designed to be NOR flash-friendly, meaning they only involve flipping bits from 1 to 0.

*   `nordb_FreeMark` (11): The record slot is erased and available for writing.
*   `nordb_UnReadMark` (10): A record has been written to this slot but has not yet been read.
*   `nordb_ReadMark` (00): The record has been read and is now considered invalid (garbage).

**State Transitions:**

1.  **Write (AddRecord):** `Free` → `Unread`
    *   The `nordb_UnReadMark` is written to the status field.
    *   C Code:
        ```c
        data &= ~(nordb_FreeMark << bit_shift);   // Clear: 11 → 00
        data |= nordb_UnReadMark << bit_shift;    // Set to 10
        ```
    *   **Transition:** `11` → `10` (one bit flips from 1 to 0, valid NOR flash write).

2.  **Read (ReadRecord):** `Unread` → `Read`
    *   The `nordb_ReadMark` is written to the status field.
    *   C Code:
        ```c
        data &= ~(nordb_FreeMark << bit_shift);   // Clear: 11 → 00
        data |= nordb_ReadMark << bit_shift;      // Set to 00
        ```
    *   **Transition:** `10` → `00` (one bit flips from 1 to 0, valid NOR flash write).

---

### Wear Leveling Strategy

#### Passive Circular Wear Distribution

**Mechanism (NOT traditional wear-leveling):**

NorDB does NOT track per-block erase counts or perform active block remapping. Instead, it achieves wear distribution through:

1.  **Sequential Sector Usage:**
    *   Writes fill sectors in circular order: 0 → 1 → 2 → ... → N → 0
    *   Reads consume sectors in the same order
    *   When a sector is fully consumed (all records are marked as `Read`), it is erased on the next write pass.

2.  **`SyncCounter` as Ordering Mechanism:**
    *   A global counter that increments on every erase operation.
    *   Each sector stores the `SyncCounter` value at the time it was last erased.
    *   The database identifies the next sector to write to by finding the one with the highest `SyncCounter` (the most recently used one with free space). It identifies the next sector to read from by finding the one with the lowest `SyncCounter` (the oldest one with unread data).
    *   This simple round-robin mechanism ensures that, over time, all sectors receive an approximately equal number of erase cycles.

**Example with 3 sectors:**

```
Time  | Action         | Sector 0      | Sector 1      | Sector 2
------|----------------|---------------|---------------|---------------
T0    | Init           | Sync=1 (Free) | Sync=- (Free) | Sync=- (Free)
T1    | Write fills 0  | Sync=1 (Full) | Sync=- (Free) | Sync=- (Free)
T2    | Read drains 0  | All read      | Sync=- (Free) | Sync=- (Free)
T3    | Write to 1     | All read      | Sync=2 (Full) | Sync=- (Free)
T4    | Write to 2     | All read      | Sync=2 (Full) | Sync=3 (Full)
T5    | Write wraps    | ERASE, Sync=4 | Sync=2 (Full) | Sync=3 (Full)
T6    | Continue       | Writing...    | Sync=2 (Full) | Sync=3 (Full)
...   | After N cycles | EraseCount≈N/3| EraseCount≈N/3| EraseCount≈N/3
```

#### Comparison to Traditional Wear-Leveling

| Traditional FTL                        | NorDB                               |
| -------------------------------------- | ----------------------------------- |
| Per-block erase counters               | Single global `SyncCounter`         |
| Active block selection (choose least-worn) | Passive circular writes           |
| Logical-to-physical mapping table      | Direct addressing with circular queue |
| Periodic wear balancing                | Automatic through sequential access |
| Suitable for random updates            | Optimized for append-only (FIFO)    |

**Trade-offs:**

*   **Pros:** Simple, no mapping overhead, predictable behavior, perfect for FIFO workloads.
*   **Cons:** Not suitable for random-access databases, entire sectors must be consumed before reuse.

## Data Integrity and Error Handling

### CRC8 Protection (`NorDB_crc8x_fast()` in `NorDB.c`)

**Implementation:**
- CRC-8 with polynomial 0x07 (table-driven)
- Applied to every record (user data only, not including CRC byte itself)
- Stored as last byte of each record

**Write Path:**
1. Calculate CRC8 of user data
2. Append CRC byte to record
3. Write to flash
4. Read back and verify (in `NorDB_AddRecord`)
5. If mismatch, mark record as invalid (`nordb_ReadMark`)

**Read Path:**
1. Read record including CRC byte
2. Recalculate CRC of data portion
3. Compare with stored CRC (in `NorDB_ReadRecord`)
4. Return failure if mismatch

**Protection Scope:**
- Detects bit flips in flash
- Detects write verification failures
- Does NOT protect sector headers

### Return Value Convention

**Success:** Non-zero (absolute flash address of record)
**Failure:** `0`

Functions returning `uint32_t` (record addresses):
- `NorDB_AddRecord()`: 0 = queue full or write failed
- `NorDB_ReadRecord()`: 0 = queue empty or CRC failed

Functions returning `bool`:
- `NorDB_Init_Sector()`: false = invalid sector number
- `NorDB_Clear()`: always returns true
- `NorDB_HwCheck()`: true = operational

**Error Recovery:**
- Write failure: Record marked invalid, space wasted until sector erase
- Read CRC failure: Record skipped, data lost
- No automatic retry mechanism found in code


## Concurrency and Thread Safety

### Locking Strategy (`sem_ll.c/h`)

**Semaphore Wrapper:**
- Uses POSIX `sem_t` (lines in `sem_ll.h`)
- All public API calls acquire/release lock
- Protects: sector state, counters, read/write pointers

**Protected Regions:**

1. **`NorDB_AddRecord()`** (lines 259, 305):
   - Entire write transaction is atomic
   - Prevents concurrent writes corrupting metadata

2. **`NorDB_ReadRecord()`** (lines 318, 344):
   - Entire read transaction is atomic
   - Prevents concurrent reads advancing pointer incorrectly

3. **`NorDB_SyncData()`** (lines 157, 209):
   - Sector scan is atomic
   - Prevents inconsistent state during recovery

4. **`NorDB_Clear()`** (lines 281, 289):
   - Mass erase is atomic

**Thread Safety Guarantees:**
- Multiple threads can safely call `NorDB_AddRecord()` and `NorDB_ReadRecord()`
- Semaphore ensures serialization
- No fine-grained locking (coarse lock per database instance)

**Limitations:**
- Single lock for entire database (not per-sector locking)
- Write-heavy workloads may contend
- No read-write lock optimization



## Storage Layout and Format

### Sector Physical Layout
```
Byte Offset | Content
------------|----------------------------------------------------------
0x0000      | NorDB_Header_t (10 bytes base)
0x0002      |   uint16_t Magic = 0x0766
0x0004      |   uint16_t Vertion = 0x0002
0x0006      |   uint32_t SyncCounter
0x000A      |   uint16_t RecordSize
0x000C      | Records[] bitmap (2 bits per record, variable length)
            |   Bitmap size = (Record_NumberInSector / 4) + 1 bytes
            |   Each byte stores 4 record states (2 bits each)
Header_Size | Record 0 (Record_Size bytes)
            |   User data (Record_Size - 1 bytes)
            |   CRC8 (1 byte)
            | Record 1 (Record_Size bytes)
            |   ...
            | Record N-1 (Record_Size bytes)
SectorSize  | [End of sector]
```
## Data layout/on-storage format

The on-storage format of the data is as follows:

*   **Sector Header:** Each sector begins with a header that contains the following information:
    *   `Magic`: A 16-bit magic number (`0x7A65`).
    *   `Vertion`: A 16-bit version number.
    *   `SyncCounter`: A 32-bit sync counter.
    *   `RecordSize`: The size of a single record.
    *   `Records`: A bitmap that tracks the status of each record in the sector.
*   **Records:** The records are stored after the sector header. Each record is padded with a 1-byte CRC-8 checksum.

The library uses a CRC-8 checksum to ensure data integrity. The checksum is calculated using a lookup table (`crc8x_table`) and is appended to each record before it is written to the storage medium.

The endianness of the data is not specified in the repository.

## Testing

The `Demo` directory contains several demo applications that can be used to test the library.

*   **`File_ex`:** Tests the file backend.
*   **`Ram_ex`:** Tests the RAM backend.
*   **`NorFlash_ex`:** Tests the NOR flash backend.

To run the tests, you will need to build the demo applications as described in the "Getting Started" section.

The tests cover the following functionality:

*   **`FullFill_Test`:** Fills the entire database with records and then reads them back to verify their integrity.
*   **`RoundRobin_Test`:** Tests the round-robin behavior of the database.
*   **`OverWrite_Test`:** Tests the overwrite protection of the database.
*   **`WriteRead_Time_Test`:** Measures the time it takes to write and read a specified number of records.
*   **`DeleteDB_Test`:** Tests the ability to delete and recreate the database.
*   **`Clear_Test`:** Tests the `NorDB_Clear` function.

## Performance notes + tuning knobs

The performance of the library depends on the underlying storage backend. The RAM backend is the fastest, followed by the file backend, and then the NOR flash backend.

The main tuning knobs are the sector size and the total number of sectors. The optimal values for these parameters depend on the specific application and the characteristics of the storage medium.


## License + contributing

### License

This project is licensed under the MIT License. See the `LICENSE` file for more details.

### Contributing

Contributions are welcome! If you would like to contribute to the project, please fork the repository and submit a pull request.
