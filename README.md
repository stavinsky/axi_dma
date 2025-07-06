# Zynq FIFO Data Acquisition with UIO and AXI Registers

This project implements a data acquisition system on Xilinx Zynq that combines:

✅ A FIFO in programmable logic (PL) to buffer streaming data.  
✅ A custom AXI-Lite register block (`simple_register`) to expose live FIFO occupancy and other status to software.  
✅ A UIO-based Linux driver that maps registers to userspace via `/dev/uio0` and provides interrupt notifications from your FIFO.

---

## ⚠️ Important Note on Throughput

🔔 **This design is *not* suitable for high-throughput applications!**

- UIO-based interrupt handling in Linux userspace introduces significant latency and CPU overhead.
- Frequent small interrupts (e.g., sub-1KB packets at high rates) will overwhelm the CPU and cause missed samples.
- This solution is ideal for **slow or moderate-speed data sources**, such as:
  - I2S microphones (e.g., 48 kSPS per channel).
  - Other low-bandwidth sensors where reliability is more important than maximum throughput.

✅ For high-speed applications (e.g., >1 MSPS), you should use a proper kernel driver or frameworks like DMAengine, with interrupt coalescing or polling inside the kernel.

---

## 📐 Architecture Overview

- The FIFO logic in the FPGA collects streaming samples.
- When the FIFO reaches a threshold, it generates an interrupt to signal data is ready.
- An AXI-Lite peripheral (`simple_register`) provides registers that expose live FIFO occupancy and other signals to software.
- The UIO driver maps the register block to userspace via `/dev/uio0`, and passes interrupts to userspace.
- The userspace application waits for either register occupancy or interrupts before starting DMA transfers using `dma-proxy`.

---

## 🛠️ Example Device Tree Nodes

Add these entries to your device tree, e.g., in `system-user.dtsi`:

```dts
// UIO device node for interrupts and register mapping
myfifo: fifo {
    compatible = "generic-uio";
    interrupt-parent = <&intc>;
    interrupts = <0 30 1>;                // SPI type, IRQ 30, rising-edge
    status = "okay";
    reg = <0x43c10000 0x10000>;           // maps simple_register's AXI registers
};

// AXI register peripheral node (simple_register)
simple_register_0: simple_register@43c10000 {
    clock-names = "s00_axi_aclk";
    clocks = <&clkc 15>;
    compatible = "xlnx,simple-register-1.0";
    reg = <0x43c10000 0x10000>;
    xlnx,s00-axi-addr-width = <0x4>;
    xlnx,s00-axi-data-width = <0x20>;
};
```


```sh
 . ../peta/images/linux/sdk/environment-setup-cortexa9t2hf-neon-xilinx-linux-gnueabi 

make KDIR=../peta/images/linux/sdk/sysroots/cortexa9t2hf-neon-xilinx-linux-gnueabi/lib/modules/6.6.40-xilinx-g2b7f6f70a62a/build/ dma-proxy.ko

```