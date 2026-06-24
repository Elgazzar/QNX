# QNX Boot Architecture & OS Images

When a QNX target board powers on, it needs an operating system to load into memory. In QNX, this operating system is bundled into a single, cohesive file known as the **OS Image** (or IFS - Image File System).

---

## 1. What is an OS Image?
At its core, a QNX OS Image is a single file that contains the entire "root" file system (the IFS) that the kernel needs to mount in RAM immediately upon boot. 

Specifically, a completely bootable OS Image **must** contain the following elements:
1.  **Bootstrap Loader:** The specialized code at the beginning of the image that handles payload decompression and memory setup.
2.  **Startup Program (`startup-boardname`):** A board-specific hardware initialization binary tailored to your exact SoC (e.g., `startup-imx8` or `startup-x86_64`).
3.  **The Operating System (`procnto` / `procnto-smp-instr`):** The core QNX Microkernel and Process Manager binary.
4.  **The Boot Script:** A text-based sequence of commands that tells `procnto` what drivers and programs to launch after initialization.
5.  **Executables and Shared Libraries:** The essential drivers, user applications, and standard C libraries (`libc.so`) required to run the commands in the boot script.

---

## 2. What is the Bootstrap?
The **Bootstrap** is a tiny, highly specialized piece of code located at the very beginning of the OS Image. 
When the bootloader (like U-Boot or UEFI) loads the OS Image into RAM and jumps to it, the Bootstrap code executes first. 

Its job is to:
*   Perform initial hardware setup (configuring the CPU, turning on the MMU).
*   Setup the initial page tables for the microkernel.
*   Decompress the rest of the image (if it is compressed).
*   Transfer control to the QNX Microkernel (`procnto`) so the OS can officially start scheduling threads.

---

## 3. The `mkifs` Build Utility
You do not build an OS Image by zipping files together manually. Instead, you use a host-side utility called **`mkifs`** (Make Image File System). 

`mkifs` requires a **Build File** (`.build`). This text file acts as a blueprint that specifies:
*   **Files & Commands:** Exactly which QNX binaries, user executables, and data files will be included in the image.
*   **Loading Options:** Specific configuration options for how individual files and executables should be loaded and mapped into memory.
*   **Image Options:** Global parameters detailing exactly how the final `.ifs` image itself should be created (e.g., compression settings, alignment, base address).
*   **The Boot Script:** For bootable images, it contains the critical boot script that tells the Process Manager (`procnto`) exactly what drivers, resource managers, and user applications to launch sequentially immediately after OS initialization.

### The `mkifs` Workflow Diagram

```mermaid
flowchart TD
    subgraph Inputs ["Inputs (Host Development Machine)"]
        A[Build File\n(.build)] -->|Blueprint| E
        B[QNX System Executables\ne.g., procnto, drivers] -->|Binaries| E
        C[User Executables\ne.g., my_app] -->|Binaries| E
        D[Optional Data Files\ne.g., config.txt] -->|Data| E
    end

    E{mkifs Utility}

    subgraph Output ["Output"]
        F[QNX OS Image File\n(.ifs)]
    end

    subgraph Deployment ["Deployment (Target Hardware)"]
        G[Target Board RAM / Flash]
    end

    E -->|Parses build file & packs files| F
    F -->|Flashed/Transferred via TFTP| G
```

### The Flow Explained:
1.  **Inputs:** You provide `mkifs` with your custom `.build` file. The build file references paths to various QNX executables, your custom user executables, and any necessary configuration data files.
2.  **Processing:** `mkifs` reads the blueprint, gathers all the specified files from your host machine, attaches the correct bootstrap code for your target architecture (ARM, x86, etc.), and packs them together.
3.  **Output:** It generates a single binary file: the **OS Image** (typically ending in `.ifs`).
4.  **Deployment:** You take this OS Image and deploy it to your target hardware (e.g., burning it to flash memory, or loading it over the network via TFTP using U-Boot).

### Example: A Simple Build File (`hello.build`)

Here is an example of what a `.build` file actually looks like. It defines the bootstrap, the boot script, and explicitly lists the binaries to pack into the image:

```text
# This is "hello.build"

[virtual=x86_64,multiboot] .bootstrap = {
    startup-x86
    PATH=/proc/boot procnto-smp-instr
}

[+script] .script = {
    procmgr_symlink /proc/boot/ldqnx-64.so.2 /usr/lib/ldqnx-64.so.2
    devc-ser8250 -e -b115200 &
    reopen /dev/ser1
    hello
}

# The explicit list of files/libraries to pack into the OS Image:
libc.so
libgcc_s.so.1
ldqnx-64.so.2
libsecpol.so
devc-ser8250
hello
```

To compile this blueprint into a bootable OS image, you would run the following command on your host machine:

```bash
mkifs hello.build hello.ifs
```

### Where Does `mkifs` Find These Files?
When you list a file like `libc.so` or `devc-ser8250` in the build file, you usually don't provide a full absolute path on your host machine. So how does `mkifs` know where to pull them from?

`mkifs` uses standard search paths defined by environment variables on your host machine. Primarily, it looks in:
*   `${QNX_TARGET}/${PROCESSOR}/bin` (for system executables)
*   `${QNX_TARGET}/${PROCESSOR}/lib` (for shared libraries)
*   `${QNX_TARGET}/${PROCESSOR}/sbin`
*   *(And other architecture-specific directories under the `$QNX_TARGET` base path)*

If `mkifs` cannot find a custom binary (like your `hello` app) in these default locations, you must either specify the explicit host path in the build file, or use the `[search=...]` attribute. 

When you apply the `search` attribute to a **particular file**, it temporarily prepends that directory to the search path just for that specific file!
*   *Example:* `[search=/home/user/workspace/my_project/bin] my_custom_app`
*   *Explanation:* `mkifs` will first look in `/home/user/workspace/my_project/bin` to find `my_custom_app` before falling back to the standard `$QNX_TARGET` directories.

### Build File Attributes
In the example above, you might have noticed instructions enclosed in square brackets `[]`. These are called **Attributes**. They act as modifiers for the files or blocks that follow them.

There are two primary types of attributes in a `.build` file:

1.  **Boolean Attributes (On/Off):**
    These attributes are toggled using a `+` (to enable) or `-` (to disable) prefix.
    *   *Example:* `[+script] .script = { ... }` 
    *   *Explanation:* The `+script` attribute tells `mkifs` to treat the following block as the boot script. Another common example is `[+compress] libc.so`, which tells `mkifs` to compress that specific library.

2.  **Value Attributes:**
    These attributes take a specific setting or parameter, assigned using an `=` sign. Multiple value attributes can be chained together with commas.
    *   *Example:* `[virtual=x86_64,multiboot] .bootstrap = { ... }`
    *   *Explanation:* This tells `mkifs` that the virtual address space layout is for an `x86_64` architecture, and it should include the `multiboot` header required by bootloaders like GRUB. Other common examples include setting ownership like `[uid=0 gid=0] my_app`.

3.  **Inline Command Attributes (Inside Scripts):**
    When the `[+script]` attribute is used, the enclosed block is parsed as a sequence of commands for `procnto` to execute at boot. You can place attributes *directly before* these commands to modify how that specific command runs!
    *   *Example:* `[pri=27f] esh`
    *   *Explanation:* This tells the OS to launch the Embedded Shell (`esh`) with a very specific, high priority (`27`) and scheduling policy (`f` for FIFO scheduling).

### Common Boot Script Patterns
If you look inside the `[+script] .script = { ... }` block, you will frequently see these two patterns:

1.  **The `reopen` Command:**
    *   *Example:* `reopen /dev/con1`
    *   *Explanation:* This is a vital built-in script command. It tells the Process Manager to immediately redirect the standard file descriptors (stdin `0`, stdout `1`, and stderr `2`) of the boot script to the specified device (in this case, the console device `/dev/con1`). Any processes launched after this command will automatically inherit these redirected file descriptors.

2.  **Complex Inline Launches:**
    *   *Example:* `[+session pri=10r] PATH=/proc/boot esh &`
    *   *Explanation:* This packs several instructions into a single line:
        *   `+session`: A boolean attribute telling `procnto` to make this process the leader of a new POSIX session.
        *   `pri=10r`: A value attribute setting the priority to `10` using Round-Robin (`r`) scheduling.
        *   `PATH=/proc/boot`: Temporarily sets the `PATH` environment variable specifically for this command.
        *   `esh`: The actual executable being launched (the Embedded Shell).
        *   `&`: Just like in standard Linux/Unix, this tells the boot script to launch the process in the background and immediately proceed to the next line in the script, preventing the boot sequence from blocking.
