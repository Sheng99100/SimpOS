// 物理和内核地址空间布局
// 物理地址直接映射到相同的内核虚拟地址
// 内核虚拟地址空间大于可用的物理 RAM 地址，多出的虚拟地址空间重复映射到内核栈、trapframe，uservec 的物理地址

// qemu 约定的地址布局
// RAM 的物理地址从 0x8000000 开始. 其余预留给 MMIO
// 
// 0x00001000 -- 启动执行的固件 boot ROM ；由 qemu 提供
// 0x02000000 -- CLINT
// 0x0C000000 -- PLIC
// 0x10000000 -- uart0 
// 0x10001000 -- virtio disk
// 0x80000000 -- boot ROM 会跳转到 RAM 的起始地址
//             -让 kernel 加载到这里

// mmio 的 uart 寄存器组地址从这里开始
#define UART0 0x10000000L
#define UART0_IRQ 10 // UART 中断号

// 虚拟磁盘接口地址
#define VIRTIO0 0x10001000
#define VIRTIO0_IRQ 1 // 虚拟磁盘中断号

// qemu 约定的 risc-v PLIC（platform-level interrupt controller）起始地址
// PLIC 控制器接收中断，区分优先级、分发给 CPU
#define PLIC 0x0c000000L
#define PLIC_PRIORITY (PLIC + 0x0)
#define PLIC_PENDING (PLIC + 0x1000)
#define PLIC_SENABLE(hart) (PLIC + 0x2080 + (hart)*0x100)
#define PLIC_SPRIORITY(hart) (PLIC + 0x201000 + (hart)*0x2000)
#define PLIC_SCLAIM(hart) (PLIC + 0x201004 + (hart)*0x2000)

// 内核加载到的起始物理地址
#define KERNBASE 0x80000000L
// 可用的最高 RAM 物理地址
#define PHYSTOP (KERNBASE + 128*1024*1024)


// 内核模式的虚拟地址空间布局
// 用户模式 trap 时跳转到的处理程序地址
// 内核空间和用户空间都要映射到这个地址. 以适配 CPU 在切换为内核页表后，继续用连续的地址读指令
#define USERVEC (MAXVA - PGSIZE)
// TRAPFRAME也在内核空间和用户空间映射到相同的地址，紧跟在 uservec 下方
#define TRAPFRAME (USERVEC - PGSIZE)

// 内核栈的虚拟地址，约定栈大小等于页大小 4KB
// 每个内核栈下方空一个没有映射的页，利用 page fault 检测栈溢出
#define KSTACK(p) (USERVEC - ((p)+1)* 2*PGSIZE)

// 用户地址空间布局.
// 地址从 0 开始:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
//   ...
//   TRAPFRAME
//   USERVEC