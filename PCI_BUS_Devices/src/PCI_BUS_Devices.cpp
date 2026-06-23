#include <iostream>
#include <cstdlib>
extern "C" {
#include <pci/pci.h>
}
#include <sys/neutrino.h>

using namespace std;

int main() {
    cout << "--- QNX PCI Device Example ---" << endl;

    // In QNX, interfacing with hardware usually requires I/O privileges
    if (ThreadCtl(_NTO_TCTL_IO, 0) == -1) {
        cerr << "Error: Requires I/O privileges. Run as root." << endl;
        return EXIT_FAILURE;
    }

    // 1. Find a PCI device. 
    // We'll use PCI_VID_ANY and PCI_DID_ANY to just grab the first device on the bus (idx = 0) for demonstration.
    pci_bdf_t bdf = pci_device_find(0, PCI_VID_ANY, PCI_DID_ANY, PCI_CCODE_ANY);
    
    if (bdf == PCI_BDF_NONE) {
        cerr << "Error: No PCI devices found!" << endl;
        return EXIT_FAILURE;
    }
    
    cout << "Found a PCI device with BDF: " << hex << bdf << dec << endl;

    // Read the Vendor ID directly using the BDF
    pci_vid_t vid;
    pci_err_t err = pci_device_read_vid(bdf, &vid);
    if (err == PCI_ERR_OK) {
        cout << "Device Vendor ID: 0x" << hex << vid << dec << endl;
    }

    // 2. Attach to the device to gain access to resources and BARs
    pci_devhdl_t hdl = pci_device_attach(bdf, pci_attachFlags_e_SHARED, &err);
    if (hdl == NULL) {
        cerr << "Error: Failed to attach to PCI device. Error code: " << err << endl;
        return EXIT_FAILURE;
    }
    cout << "Successfully attached to PCI device." << endl;

    // 3. Read Base Address Registers (BARs)
    int_t num_bars = 6; // PCI standard devices can have up to 6 BARs
    pci_ba_t bars[6];
    
    err = pci_device_read_ba(hdl, &num_bars, bars, pci_reqType_e_MANDATORY);
    if (err == PCI_ERR_OK && num_bars > 0) {
        cout << "Found " << num_bars << " Base Address Register(s)." << endl;
        
        // 4. Map the first available BAR address space for demonstration
        pci_ba_t mapped_ba;
        err = pci_device_map_as(hdl, &bars[0], &mapped_ba);
        if (err == PCI_ERR_OK) {
            cout << "Successfully mapped BAR 0 into CPU space." << endl;
            // ... Here you would typically read/write to the mapped memory ...
            // e.g., pci_device_cfg_wr32(hdl, offset, val, &val_p)
        } else {
            cerr << "Warning: Could not map BAR 0. Error: " << err << endl;
        }
    } else {
        cout << "No BARs found or error reading BARs." << endl;
    }

    // 5. Reset the device (Optional: handle with care in a real system)
    // cout << "Resetting the PCI device..." << endl;
    // pci_device_reset(hdl, pci_resetType_e_FLR, pci_reqType_e_MANDATORY);

    // 6. Detach from the device
    err = pci_device_detach(hdl);
    if (err == PCI_ERR_OK) {
        cout << "Successfully detached from the PCI device." << endl;
    } else {
        cerr << "Error detaching from device." << endl;
    }

    return EXIT_SUCCESS;
}
