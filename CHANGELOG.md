# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).

## [Unreleased]
### Added

### Changed

## Release 13
### Added
- Support for Amazon AWS F1
- Support for [BittWare XUP-P3R](https://www.bittware.com/fpga/xup-p3r) FPGA board
- Tursi pickling tool based on FuseSoC
- New, higher performance AXI4 memory controller (and AXI4 memory zeroing)
- Exponential back-off for Ariane LR/SC

### Changed
- Fixed Ariane RTC bug that showed 2x real time
- Better instructions and other readme fixes (thanks to Dave McEwan)
- Fixed parameter syntax in SPARC core (thanks to Scott Temple)
- Pitonstream can reset the FPGA from the host

## Release 12
### Added
- AXI4 memory controller option on vc707
- Multicore Verilator simulation using DPI alternative to existing PLI
- Simulation of OpenPiton+Ariane with VCS
- Simulation of OpenPiton+Ariane with Verilator
- Ethernet support on G2 for OpenPiton+Ariane

### Changed
- Remove BUFG and clock gating latches for FPGA targets
- Remove inferred latches in all dynamic_node variants, l2_pipe1_ctrl and uart_mux
- Update storage_addr_trans* to include different board configurations for Ariane
- Update Ariane version to v4.2. This includes several bugfixes and improvements.
- Update RISC-V peripherals (new PLIC, updated debug module with support for multi-hart debug)
- OS stability improvements from LR/SC invalidation fix
- Rewrite SD controller. Cross-timing-domain hazards fixed
- Update Xilinx IPs to Vivado 2016.4
- Moved to CAM structure for L2 MSHR to improve timing
- Changed generate if blocks in tile to reflect restrictions with Verilator. Functionality is unchanged.
- Rework noc-axilite-plic interface to fix issues with Ethernet IRQs (OpenPiton+Ariane)
- Removed 32 bit libraries and binaries that are no longer supported
- Moved to a 64 bit version of m4 that has mpeval support

## Release 11

### Added

- Support for Verilator simulation

For Ariane:
- Support for Pitonstream
- Support for RISC-V compliant debug
- Device tree generator
- RISC-V compliant interrupt controllers (PLIC, CLINT)
- Support for SMP Linux
- Support for Ariane builds on the Genesys2, Nexys Video and VC707 FPGA boards

### Changed

For Ariane:
- Updated to Ariane v4.1
- Bugfixes in write-through cache system of Ariane

