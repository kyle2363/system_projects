# WeensyOS: Virtual Memory and Processes

An educational x86-64 kernel project built on WeensyOS. The implementation work is centered in [kernel.cc](kernel.cc), including page-table handling, process memory, fork, and exit.

The supplied emulator, boot code, hardware support, and libraries are part of the teaching framework. Preserve [COPYRIGHT](COPYRIGHT) and [AUTHORS.md](AUTHORS.md); see [ABOUT.md](ABOUT.md) for upstream background and debugging instructions.

## Build and run

In the Linux course environment, from this directory:

```bash
make
make run-console
```

QEMU is needed to run the kernel. Building an image alone does not verify virtual-memory isolation or runtime behavior. See [../VALIDATION.md](../VALIDATION.md) for the scope of verification.

## Known limitations

The additional sleep, kill, and page-free system calls are experimental. The original implementation notes report timer progress problems during keyboard input and address/permission faults in kill and page-free scenarios. These extensions are retained for study and are not presented as finished features.
