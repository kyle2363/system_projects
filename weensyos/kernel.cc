#include "kernel.hh"
#include "k-apic.hh"
#include "k-vmiter.hh"
#include "obj/k-firstprocess.h"
#include <atomic>

// kernel.cc
//
//    This is the kernel.


// INITIAL PHYSICAL MEMORY LAYOUT
//
//  +-------------- Base Memory --------------+
//  v                                         v
// +-----+--------------------+----------------+--------------------+---------/
// |     | Kernel      Kernel |       :    I/O | App 1        App 1 | App 2
// |     | Code + Data  Stack |  ...  : Memory | Code + Data  Stack | Code ...
// +-----+--------------------+----------------+--------------------+---------/
// 0  0x40000              0x80000 0xA0000 0x100000             0x140000
//                                             ^
//                                             | \___ PROC_SIZE ___/
//                                      PROC_START_ADDR

#define PROC_SIZE 0x40000       // initial state only

proc ptable[PID_MAX];           // array of process descriptors
                                // Note that `ptable[0]` is never used.
proc* current;                  // pointer to currently executing proc

#define HZ 100                  // timer interrupt frequency (interrupts/sec)
static std::atomic<unsigned long> ticks; // # timer interrupts so far


// Memory state - see `kernel.hh`
physpageinfo physpages[NPAGES];


[[noreturn]] void schedule();
[[noreturn]] void run(proc* p);
void exception(regstate* regs);
uintptr_t syscall(regstate* regs);
void memshow();
void free_pagetable(x86_64_pagetable *pagetable);

int enable_tick = 1; //enable Extra credit sleep


// kernel_start(command)
//    Initialize the hardware and processes and start running. The `command`
//    string is an optional string passed from the boot loader.

static void process_setup(pid_t pid, const char* program_name);

void kernel_start(const char* command) {
    // initialize hardware
    init_hardware();
    log_printf("Starting WeensyOS\n");

    ticks = 1;
    init_timer(HZ);

    // clear screen
    console_clear();

    // (re-)initialize kernel page table
    for (uintptr_t addr = 0; addr < MEMSIZE_PHYSICAL; addr += PAGESIZE) {
        int perm = PTE_P | PTE_W | PTE_U;
        if (addr == 0) {
            // nullptr is inaccessible even to the kernel
            perm = 0;
        }
        if(addr > 0 && addr < PROC_START_ADDR){
            perm = PTE_P | PTE_W;
        }
        // install identity mapping
        int r = vmiter(kernel_pagetable, addr).try_map(addr, perm);
        assert(r == 0); // Assume mapping is successful
                        // (OK at startup, but won't always be true later!)
    }

    // set up process descriptors
    for (pid_t i = 0; i < PID_MAX; i++) {
        ptable[i].pid = i;
        ptable[i].state = P_FREE;
    }
    if (!command) {
        command = WEENSYOS_FIRST_PROCESS;
    }
    if (!program_image(command).empty()) {
        process_setup(1, command);
    } else {
        process_setup(1, "allocator");
        process_setup(2, "allocator2");
        process_setup(3, "allocator3");
        process_setup(4, "allocator4");
    }

    // switch to first process using run()
    run(&ptable[1]);
}


// kalloc(sz)
//    Kernel physical memory allocator. Allocates at least `sz` contiguous bytes
//    and returns a pointer to the allocated memory, or `nullptr` on failure.
//    The returned pointer’s address is a valid physical address, but since the
//    WeensyOS kernel uses an identity mapping for virtual memory, it is also a
//    valid virtual address that the kernel can access or modify.
//
//    The allocator selects from physical pages that can be allocated for
//    process use (i.e., not reserved pages or kernel data), and from physical
//    pages that are currently unused (`physpages[N].refcount == 0`).
//
//    On WeensyOS, `kalloc` is a page-based allocator: if `sz > PAGESIZE`
//    the allocation fails; if `sz < PAGESIZE` it allocates a whole page
//    anyway.
//
//    The returned memory is initially filled with 0xCC, which corresponds to
//    the `int3` instruction. Executing that instruction will cause a `PANIC:
//    Unhandled exception 3!` This may help you debug.
void* kalloc(size_t sz) {
    if (sz > PAGESIZE) {
        return nullptr;
    }

    int pageno = 0;
    int page_increment = 1;
    // `pageno` indicates the starting point to search for free pages.
    // In the stencil versions, `kalloc` returns the first free page.
    // Other search strategies can be faster, or may help you debug.
    // For example, this initialization returns a random free page:
    //     int pageno = rand(0, NPAGES - 1);
    // This initialization remembers the most-recently-allocated page and
    // starts the search from there:
    //     static int pageno = 0;

    for (int tries = 0; tries != NPAGES; tries++) {
        uintptr_t pa = pageno * PAGESIZE;
        if (allocatable_physical_address(pa)
            && physpages[pageno].refcount == 0) {
            physpages[pageno].refcount++;
            memset((void*) pa, 0xCC, PAGESIZE);
            return (void*) pa;
        }
        pageno = (pageno + page_increment) % NPAGES;
    }

    return nullptr;
}


// kfree(kptr)
//    Free `kptr`, which must have been previously returned by `kalloc`.
//    If `kptr == nullptr` does nothing.
void kfree(void* kptr) {
    (void) kptr; // Suppress unused variable warning

    // TODO:  Implement!
    if (kptr == nullptr) return;
    uintptr_t pa = (uintptr_t) kptr;
    assert(pa % PAGESIZE == 0);

    int pn = pa / PAGESIZE;

    assert(pn >= 0 && pn < NPAGES);

    assert(physpages[pn].refcount > 0);
    physpages[pn].refcount--;

    if (physpages[pn].refcount == 0) {
        memset(kptr, 0xCC, PAGESIZE);
    }

}

void log_program_info(const char* program_name) {
    program_image pgm(program_name);
    log_printf("program %s: entry point %p\n", program_name, pgm.entry());
    size_t n = 0;
    for (auto seg = pgm.begin(); seg != pgm.end(); ++seg, ++n) {
        log_printf("  segment %zu: addr %p, size %lu, data_size %lu, %s\n",
                   n, seg.va(), seg.size(), seg.data_size(),
                   seg.writable() ? "writable" : "not writable");
        if (seg.data_size() > 0) {
            log_printf("    first data byte: %u\n", (unsigned char) *seg.data());
        }
    }
}


// process_setup(pid, program_name)
//    Load application program `program_name` as process number `pid`.
//    This loads the application's code and data into memory, sets its
//    %rip and %rsp, gives it a stack page, and marks it as runnable.
void process_setup(pid_t pid, const char* program_name) {
    init_process(&ptable[pid], 0);

    x86_64_pagetable* pt = kalloc_pagetable();

    // obtain reference to program image
    // (The program image models the process executable.)
    program_image pgm(program_name);

    for (uintptr_t addr = 0; addr < PROC_START_ADDR; addr += PAGESIZE) {
        int perm = PTE_P | PTE_W;
        if (addr == 0) perm = 0; 
        if(addr == 0xB8000)
            perm |= PTE_U;
        int r = vmiter(pt, addr).try_map(addr, perm);
        assert(r == 0);
    }

    for (program_image_segment seg = pgm.begin(); seg != pgm.end(); seg.operator++()) {
        int perm = PTE_P | PTE_U;
        if (seg.writable()) 
            perm |= PTE_W;
        for (uintptr_t a = round_down(seg.va(), PAGESIZE); a < (seg.va() + seg.size());
            a += PAGESIZE) {

            void* pa = kalloc(PAGESIZE); 
            //assert(physpages[a / PAGESIZE].refcount == 0);
            int r = vmiter(pt, a).try_map((uintptr_t)pa, perm);
            assert(r == 0);
        }
    }

    // copy instructions and data from program image into process memory
    for (auto seg = pgm.begin(); seg != pgm.end(); seg++) {
	    void* pa = (void*)vmiter(pt, seg.va()).pa();  // Stencil:  assume PA == VA
        memset(pa, 0, seg.size());
        memcpy(pa, seg.data(), seg.data_size());
    }

    // mark entry point
    ptable[pid].regs.reg_rip = pgm.entry();

    ptable[pid].wakeuptick = 0;

    // allocate and map stack segment
    // Compute process virtual address for stack page
    uintptr_t stack_addr = round_down(MEMSIZE_VIRTUAL - PAGESIZE, PAGESIZE);

    // Stencil version:  assume physical address at `stack_addr` is free
    // and claim it by incrementing its reccount.
    void* stack_pa = kalloc(PAGESIZE);
    assert(stack_pa != nullptr);

    ptable[pid].regs.reg_rsp = stack_addr + PAGESIZE;
    int r = vmiter(pt, stack_addr).try_map(stack_pa,PTE_P | PTE_W | PTE_U);

    // mark process as runnable
    ptable[pid].state = P_RUNNABLE;
    ptable[pid].pagetable = pt;
}



// exception(regs)
//    Exception handler (for interrupts, traps, and faults).
//
//    The register values from exception time are stored in `regs`.
//    The processor responds to an exception by saving application state on
//    the kernel's stack, then jumping to kernel assembly code (in
//    k-exception.S). That code saves more registers on the kernel's stack,
//    then calls exception().
//
//    Note that hardware interrupts are disabled when the kernel is running.

void exception(regstate* regs) {
    // Copy the saved registers into the `current` process descriptor.
    current->regs = *regs;
    regs = &current->regs;
    

    // It can be useful to log events using `log_printf`.
    // Events logged this way are stored in the host's `log.txt` file.
    /* log_printf("proc %d: exception %d at rip %p\n",
                current->pid, regs->reg_intno, regs->reg_rip); */

    // Show the current cursor location and memory state
    // (unless this is a kernel fault).
    console_show_cursor(cursorpos);
    if (regs->reg_intno != INT_PF || (regs->reg_errcode & PTE_U)) {
        memshow();
    }

    // If Control-C was typed, exit the virtual machine.
    check_keyboard();


    // Actually handle the exception.
    switch (regs->reg_intno) {

    case INT_IRQ + IRQ_TIMER:
        ticks++;
        lapicstate::get().ack();
        schedule();
        break;                  /* will not be reached */

    case INT_PF: {
        // Analyze faulting address and access type.
        uintptr_t addr = rdcr2();
        const char* operation = regs->reg_errcode & PTE_W
                ? "write" : "read";
        const char* problem = regs->reg_errcode & PTE_P
                ? "protection problem" : "missing page";

        if (!(regs->reg_errcode & PTE_U)) {
            proc_panic(current, "Kernel page fault on %p (%s %s, rip=%p)!\n",
                       addr, operation, problem, regs->reg_rip);
        }
        error_printf(CPOS(24, 0), COLOR_ERROR,
                     "PAGE FAULT on %p (pid %d, %s %s, rip=%p)!\n",
                     addr, current->pid, operation, problem, regs->reg_rip);
        log_print_backtrace(current);
        current->state = P_FAULTED;
        break;
    }

    default:
        proc_panic(current, "Unhandled exception %d (rip=%p)!\n",
                   regs->reg_intno, regs->reg_rip);

    }


    // Return to the current process (or run something else).
    if (current->state == P_RUNNABLE) {
        run(current);
    } else {
        schedule();
    }
}


int syscall_page_alloc(uintptr_t addr);
pid_t syscall_fork();
void syscall_exit();
int syscall_page_free(uintptr_t addr, size_t len);
int syscall_kill(pid_t pid);
int syscall_sleep(unsigned long ticks_to_sleep);


// syscall(regs)
//    Handle a system call initiated by a `syscall` instruction.
//    The process’s register values at system call time are accessible in
//    `regs`.
//
//    If this function returns with value `V`, then the user process will
//    resume with `V` stored in `%rax` (so the system call effectively
//    returns `V`). Alternately, the kernel can exit this function by
//    calling `schedule()`, perhaps after storing the eventual system call
//    return value in `current->regs.reg_rax`.
//
//    It is only valid to return from this function if
//    `current->state == P_RUNNABLE`.
//
//    Note that hardware interrupts are disabled when the kernel is running.

uintptr_t syscall(regstate* regs) {
    // Copy the saved registers into the `current` process descriptor.
    current->regs = *regs;
    regs = &current->regs;

    // It can be useful to log events using `log_printf`.
    // Events logged this way are stored in the host's `log.txt` file.
    /* log_printf("proc %d: syscall %d at rip %p\n",
                  current->pid, regs->reg_rax, regs->reg_rip); */

    // Show the current cursor location and memory state.
    console_show_cursor(cursorpos);
    memshow();

    // If Control-C was typed, exit the virtual machine.
    check_keyboard();


    // Actually handle the exception.
    switch (regs->reg_rax) {

    case SYSCALL_PANIC:
        user_panic(current);
        break; // will not be reached

    case SYSCALL_GETPID:
        return current->pid;

    case SYSCALL_YIELD:
        current->regs.reg_rax = 0;
        schedule();             // does not return

    case SYSCALL_PAGE_ALLOC:
        return syscall_page_alloc(current->regs.reg_rdi);

    case SYSCALL_FORK:
        return syscall_fork();

    case SYSCALL_EXIT:
        syscall_exit();
        schedule(); // does not return
    
    // EC
    case SYSCALL_KILL:
        return syscall_kill(current->regs.reg_rdi);

    case SYSCALL_SLEEP:
        syscall_sleep(current->regs.reg_rdi);
        break;

    case SYSCALL_PAGE_FREE:
        return syscall_page_free(current->regs.reg_rdi, current->regs.reg_rsi);

    default:
        proc_panic(current, "Unhandled system call %ld (pid=%d, rip=%p)!\n",
                   regs->reg_rax, current->pid, regs->reg_rip);

    }

    panic("Should not get here!\n");
}


// syscall_page_alloc(addr)
//    Handles the SYSCALL_PAGE_ALLOC system call. This function
//    should implement the specification for `sys_page_alloc`
//    in `u-lib.hh` (but in the stencil version, it does not).
int syscall_page_alloc(uintptr_t addr) {

    // Stencil version: assume that physical page at `addr` is free
    // and claim it by incrementing its refcount.  `addr` is the
    // virtual address where the program wants to allocate a page
    //
    // (This assumes that the physical address at `addr` is not already
    // allocated, which won't work long term!)
    /*assert(physpages[addr / PAGESIZE].refcount == 0);
    physpages[addr / PAGESIZE].refcount++;

    memset((void*) addr, 0, PAGESIZE);*/
    if (addr < PROC_START_ADDR || addr >= MEMSIZE_VIRTUAL) return -1;
    void* pa = kalloc(PAGESIZE);
    if (!pa) return -1;

    

    memset(pa, 0, PAGESIZE);

    int r = vmiter(current->pagetable, round_down(addr,PAGESIZE))
                .try_map((uintptr_t)pa, PTE_P | PTE_W | PTE_U);
    if (r < 0) {
        kfree(pa);
        return -1;
    }
    return 0;
}

// syscall_fork()
//    Handles the SYSCALL_FORK system call. This function
//    implements the specification for `sys_fork` in `u-lib.hh`.
pid_t syscall_fork() {
    // TODO: Implement!

    int pid = 1;
    while(pid < PID_MAX && ptable[pid].state != P_FREE ){
        pid ++;
    } 
    if(pid == PID_MAX){
        return -1;
    }

    init_process(&ptable[pid], 0);
    ptable[pid].wakeuptick = 0;

    

    x86_64_pagetable* new_pt = kalloc_pagetable();

    if (!new_pt) {
        return -1;
    }

    x86_64_pagetable* old_pt = current->pagetable;

    for (vmiter it(old_pt, 0); it.va() < PROC_START_ADDR; it += PAGESIZE) {
        if (it.present()) {
            int r = vmiter(new_pt, it.va()).try_map(it.pa(), it.perm());
            if (r < 0) {
                free_pagetable(new_pt);
                return -1;
            }
        }
    }

    for (vmiter it(old_pt, PROC_START_ADDR); it.va() < MEMSIZE_VIRTUAL; it += PAGESIZE) {
        if (it.present() && it.user()){
            if(it.writable()){
                void* new_pa = kalloc(PAGESIZE); 
                if (!new_pa) {
                    free_pagetable(new_pt);
                    return -1;
                }

                memcpy(new_pa, (void*) it.pa(), PAGESIZE);

                int r = vmiter(new_pt, it.va()).try_map((uintptr_t)new_pa, it.perm());
                if (r < 0) {
                    kfree(new_pa);
                    free_pagetable(new_pt);
                    return -1;
                }
            }else{
                physpages[it.pa() / PAGESIZE].refcount++;
                int r = vmiter(new_pt, it.va()).try_map(it.pa(), it.perm());
                if (r < 0) {
                    physpages[it.pa() / PAGESIZE].refcount--;
                    free_pagetable(new_pt);
                    return -1;
                }
            }
        }
    }

    ptable[pid].regs = current->regs;
    ptable[pid].regs.reg_rax = 0;     
    current->regs.reg_rax = pid;      

    ptable[pid].state = P_RUNNABLE;
    ptable[pid].pagetable = new_pt;

    return pid;
}

// syscall_exit()
//    Handles the SYSCALL_EXIT system call. This function
//    implements the specification for `sys_exit` in `u-lib.hh`.
void syscall_exit() {
    // TODO:  Implement!
    free_pagetable(current->pagetable);
    current->state = P_FREE;
}

// free_pagetable(x86_64_pagetable *pagetable)
//    Suggested helper function: free all pages associated with a page table:
//    this includes the pages that have mappings in the page table, as well as
//    the pages used to store the page table itself.
//    (If you modify the arguments/return type, be sure to change the function
//     prototype at the top of the file as well!)
void free_pagetable(x86_64_pagetable *pagetable) {
    // TODO:  Implement!  (If you choose to use this helper)
    for (vmiter it(pagetable, 0); it.va() < MEMSIZE_VIRTUAL; it += PAGESIZE) {
        if (it.present() && it.user() && it.pa()!=0xB8000){
            kfree((void*)it.pa());
        }
    }

    for (ptiter it(pagetable); it.va() < MEMSIZE_VIRTUAL; it.next()) {
        kfree((void*)it.pa());
    }
    kfree(pagetable);
}


// schedule
//    Pick the next process to run and then run it.
//    If there are no runnable processes, spins forever.

void schedule() {
    pid_t pid = current->pid;
    for (unsigned spins = 1; true; spins++) {
        pid = (pid + 1) % PID_MAX;
        init_timer(100);
        for (int i = 0; i < PID_MAX; i++) {
            if (ptable[i].state == P_BLOCKED &&
                ticks.load() >= ptable[i].wakeuptick) {
                log_printf("make runable tick:%lu\n", ticks.load());
                ptable[i].state = P_RUNNABLE;
            }
        }
        if (ptable[pid].state == P_RUNNABLE) {
            run(&ptable[pid]);
        }

        // If Control-C was typed, exit the virtual machine.
        check_keyboard();

        // If spinning forever, show the memviewer.
        if (spins % (1 << 12) == 0) {
            memshow();
        }
        if(spins % 1000 == 0 && enable_tick)
            ticks++;
    }
}


// run(p)
//    Run process `p`. This involves setting `current = p` and calling
//    `exception_return` to restore its page table and registers.

void run(proc* p) {
    assert(p->state == P_RUNNABLE);
    current = p;

    // Check the process's current registers.
    check_process_registers(p);

    // Check the process's current pagetable.
    check_pagetable(p->pagetable);

    // This function is defined in k-exception.S. It restores the process's
    // registers then jumps back to user mode.
    exception_return(p);

    // should never get here
    while (true) {
    }
}


// memshow()
//    Draw a picture of memory (physical and virtual) on the CGA console.
//    Switches to a new process's virtual memory map every 0.25 sec.
//    Uses `console_memviewer()`, a function defined in `k-memviewer.cc`.

void memshow() {
    static unsigned last_ticks = 0;
    static int showing = 0;

    // switch to a new process every 0.25 sec
    if (last_ticks == 0 || ticks - last_ticks >= HZ / 2) {
        last_ticks = ticks;
        showing = (showing + 1) % PID_MAX;
    }

    proc* p = nullptr;
    for (int search = 0; !p && search < PID_MAX; search++) {
        if (ptable[showing].state != P_FREE
            && ptable[showing].pagetable) {
            p = &ptable[showing];
        } else {
            showing = (showing + 1) % PID_MAX;
        }
    }

    console_memviewer(p);
    if (!p) {
        console_printf(CPOS(10, 26), 0x0F00, "   VIRTUAL ADDRESS SPACE\n"
            "                          [All processes have exited]\n"
            "\n\n\n\n\n\n\n\n\n\n\n");
    }
}

// EXTRA CREDIT

// syscall_kill()
// a system call that lets one process kill another process
int syscall_kill(pid_t pid) {
    
    if (pid <= 0 || pid >= PID_MAX) {
        return -1;
    }

    if (ptable[pid].state == P_FREE) {
        return -1;
    }

    free_pagetable(ptable[pid].pagetable);
    ptable[pid].state = P_FREE;

    if (pid == current->pid) {
        schedule();  
    }
    
    return 0;
}

// syscall_sleep()
// a system call that puts the calling process to 
//sleep until a given amount of time (measured in ticks) elapses.
int syscall_sleep(unsigned long ticks_to_sleep) {
    current->wakeuptick = ticks.load() + ticks_to_sleep;
    log_printf("tick:%lu\n", ticks.load());
    log_printf("tick+s:%lu\n", ticks.load()+ ticks_to_sleep);
    current->state = P_BLOCKED;
    current->regs.reg_rax = 0;
    schedule();
    return 0;
}

// syscall_page_free(addr, len)
//    Unmaps all pages in [addr, addr+len):
//    - addr must be page-aligned and len must be > 0
//    - only unmaps pages that are present and user-accessible
//    - decrements refcount; frees physical page when refcount hits 0
int syscall_page_free(uintptr_t addr, size_t len) {
    if (len == 0) 
        return -1;

    if (addr % PAGESIZE != 0) 
        return -1;

    if (addr < PROC_START_ADDR || addr + len > MEMSIZE_VIRTUAL) return -1;

    uintptr_t end = round_up(addr + len, PAGESIZE);
    for (uintptr_t a = addr; a < end; a += PAGESIZE) {
        vmiter it(current->pagetable, a);

        if (!it.present() || !it.user()) continue;

        uintptr_t pa = it.pa();

        it.try_map((uintptr_t)0, 0);
        kfree((void*) pa);   
    }

    return 0;
}
