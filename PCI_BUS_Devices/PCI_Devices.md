# QNX PCI Device Management

This document provides an overview of how to interact with PCI/PCIe devices using the modern QNX `libpci` API. The `pci-server` manages PCI resources, and applications link against `libpci` (`-lpci`) to discover, configure, and map device resources.

## Core PCI APIs

All applications communicating with the PCI server must include `<pci/pci.h>`. Many of these functions require I/O privileges, which a thread can request by calling `ThreadCtl(_NTO_TCTL_IO, 0)`.

### 1. `pci_device_find`
Locates a specific PCI device on the bus based on search criteria like Vendor ID (VID), Device ID (DID), and Class Code.

```c
pci_bdf_t pci_device_find(
    const uint_t idx,
    const pci_vid_t vid,
    const pci_did_t did,
    const pci_ccode_t classcode
);
```
*   **`idx`**: The 0-based index of the matching device (used to iterate over multiple instances of the same device).
*   **`vid`, `did`, `classcode`**: Search parameters. Use `PCI_VID_ANY`, `PCI_DID_ANY`, or `PCI_CCODE_ANY` for wildcards.
*   **Returns**: A Bus/Device/Function (`pci_bdf_t`) identifier, or `PCI_BDF_NONE` if not found.

### 2. `pci_device_attach`
Establishes a connection to the device and reserves it for configuration or mapping. Most other functions require the handle returned by this function.

```c
pci_devhdl_t pci_device_attach(
    pci_bdf_t bdf,
    pci_attachFlags_t flags,
    pci_err_t * err
);
```
*   **`bdf`**: The identifier returned by `pci_device_find`.
*   **`flags`**: Access level, such as `pci_attachFlags_e_EXCLUSIVE` or `pci_attachFlags_e_SHARED`.
*   **Returns**: A valid `pci_devhdl_t` handle, or `NULL` on failure.

### 3. `pci_device_read_ba`
Retrieves the Base Address Registers (BARs) for the device. BARs describe the memory and I/O spaces required by the hardware.

```c
pci_err_t pci_device_read_ba(
    pci_devhdl_t hdl, 
    int_t * nba, 
    pci_ba_t * ba, 
    pci_reqType_t reqType 
);
```
*   **`nba`**: A pointer to an integer specifying the number of elements in the `ba` array. The function updates it with the actual number of BARs found.
*   **`ba`**: An array of `pci_ba_t` structures to hold the BAR information.

### 4. `pci_device_map_as`
Maps a device's physical memory space (described by a BAR) into the CPU's addressable memory, typically for DMA or direct memory-mapped I/O (MMIO) access.

```c
pci_err_t pci_device_map_as(
    pci_devhdl_t hdl, 
    const pci_ba_t * const as, 
    pci_ba_t * as_xlate 
);
```
*   **`as`**: The source base address descriptor (from `pci_device_read_ba`).
*   **`as_xlate`**: The resulting translated descriptor representing the CPU mapping.

### 5. `pci_device_read` & `pci_device_write`
Instead of monolithic read/write functions, `libpci` provides specific typed functions for standard configuration space registers (e.g., Command, Status, VID, DID). Device-specific extended registers can be accessed using `pci_device_cfg_rd*` and `pci_device_cfg_wr*` (e.g., 8, 16, or 32-bit access).

```c
// Example: Reading the status register
pci_err_t pci_device_read_status(pci_bdf_t bdf, pci_stat_t *status);

// Example: Writing to the command register
pci_err_t pci_device_write_cmd(pci_devhdl_t hdl, pci_cmd_t cmd, pci_cmd_t *cmd_p);

// Example: Writing to an extended 32-bit register at a specific offset
pci_err_t pci_device_cfg_wr32(pci_devhdl_t hdl, uint_t offset, uint32_t val, uint32_t *val_p);
```

### 6. `pci_device_reset`
Triggers a Function Level Reset (FLR) or secondary bus reset for the device, restoring it to an uninitialized state.

```c
pci_err_t pci_device_reset(
    pci_devhdl_t hdl,
    pci_resetType_t resetType,
    pci_reqType_t reqType
);
```

### 7. `pci_device_detach`
Releases the device handle and disconnects from the PCI server, freeing associated resources.

```c
pci_err_t pci_device_detach(pci_devhdl_t hdl);
```
