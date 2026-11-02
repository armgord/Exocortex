# 8-Channel EEG Brain–Computer Interface — Acquisition Hardware

Design of an **8-channel EEG acquisition system** for a brain–computer interface, explored in two hardware iterations:

- **M1, discrete analog front end:** each channel is built from discrete stages (protection, instrumentation amplifier, filters, gain), digitized by a 12-bit ADC and read by an external STM32.
- **M2, integrated front end:** a single **TI ADS1299** 24-bit biopotential ADC with an on-board STM32 and IMU, powered over USB.

Schematics and PCBs are designed in **KiCad**.

> 🚧 **Status: work in progress.** Both schematics are complete. The M1 PCB layout is being routed, M2 is being prepared for fabrication, and the firmware is at an early polling stage. See [Roadmap](#roadmap).

> 📄 **Design report:** [`docs/BCI_Design_Report.pdf`](docs/BCI_Design_Report.pdf)

---

## System Overview

```mermaid
flowchart LR
    E[8 electrodes<br/>+ REF + BIAS] --> P[ESD / TVS<br/>protection]
    P --> RF[EMI LCL<br/>RF filters]

    subgraph M1 [M1 — discrete, per channel]
        IA[AD8422 in-amp<br/>G ≈ 400] --> HP[HPF 0.3 Hz<br/>2nd-order Butterworth]
        HP --> LP[LPF 35 Hz<br/>Butterworth]
        LP --> N[60 Hz notch<br/>≥ 30 dB]
        N --> G[Non-inverting gain<br/>total ≈ 50,000]
        G --> ADC1[ADS7828<br/>12-bit, 8-ch MUX]
        ADC1 -- I²C --> MCU1[STM32 Nucleo-64<br/>external]
    end

    subgraph M2 [M2 — integrated]
        ADS[ADS1299<br/>24-bit ΔΣ, 8-ch] -- SPI --> MCU2[STM32F303<br/>Cortex-M4]
        IMU[Gyroscope] --> MCU2
    end

    RF --> IA
    RF --> ADS
```

| | **M1 — Discrete AFE** | **M2 — ADS1299** |
|---|---|---|
| Channels | 8 | 8 |
| Conditioning | Discrete in-amp, filters and gain per channel | Integrated PGA in ADS1299 |
| ADC | ADS7828, 12-bit, I²C | ADS1299, 24-bit ΔΣ, simultaneous sampling |
| MCU | External STM32 Nucleo-64 | On-board STM32F303CCT (Cortex-M4) |
| Common-mode control | Discrete Right-Leg Drive | ADS1299 driven bias (BIASOUT) |
| Power | ±9 V batteries | USB |
| Extras | — | Gyroscope for head-motion data |

---

## M1 — Discrete Analog Front End

### Signal chain (per channel)

| Stage | Implementation | Purpose |
|---|---|---|
| **ESD protection** | 3× TI TPD4E1B06 4-ch bidirectional TVS arrays | Protect the subject and circuit on all 8 channels, REF and BIAS |
| **RF filtering** | EMI LCL filter on every input line | Reject MHz–GHz interference from broadcast and wireless sources |
| **Instrumentation amp** | ADI AD8422, R<sub>G</sub> = 50 Ω → G ≈ 400 | Low-noise differential front-end gain with high CMRR |
| **High-pass filter** | 2nd-order Butterworth, f<sub>c</sub> = 0.3 Hz | Remove DC offset and slow drift artifacts |
| **Low-pass filter** | Butterworth, f<sub>c</sub> = 35 Hz | Remove EMG and high-frequency noise |
| **Notch filter** | 60 Hz, ≥ 30 dB attenuation | Suppress mains hum |
| **Final gain** | Non-inverting op-amp stage | Total simulated gain ≈ **50,000** to fit µV-level EEG into the ADC range |

### Right-Leg Drive

The common-mode voltage of each channel is buffered, summed and inverted (LM324), then driven back to the subject through the bias electrode. This actively cancels common-mode interference and raises the effective CMRR beyond what passive grounding achieves.

<p align="center">
  <img src="images/m1_rld_bias.png" width="420" alt="Right-leg drive / bias circuit">
</p>

### Power and digital

- **LP2985-3.3** LDO from the 9 V supply powers the ADC.
- **REF3330** precision 3.3 V reference sets the ADC's full-scale range.
- **ADS7828** 12-bit, 8-channel ADC with internal MUX and sample-and-hold, connected over I²C.
- The STM32 sits **off-board on purpose**, keeping digital switching noise away from µV-level analog signals.

<p align="center">
  <img src="images/m1_pcb_layout.png" width="720" alt="M1 PCB layout in KiCad (in progress)">
  <br><sub>M1 PCB in KiCad: SMD parts placed, routing in progress</sub>
</p>

---

## M2 — Integrated Front End (ADS1299)

M2 replaces the M1 per-channel analog chain with the **ADS1299**, the low-noise 24-bit simultaneous-sampling ΔΣ ADC also used in OpenBCI's Cyton board.

- **Referential montage:** every channel is measured against a common reference electrode using the ADS1299's internal routing, which cuts electrode cabling.
- **Driven bias:** the internal summing amplifier averages the common-mode signal from the selected electrodes, and the inverted signal is driven to the subject through **BIASOUT** via a protection resistor. BIASIN is the feedback point, not a measurement input.
- **On-board digital:** STM32F303CCT Cortex-M4, USB power, and a gyroscope for head-motion tracking.

<p align="center">
  <img src="images/m2_schematic.png" width="760" alt="M2 schematic: ADS1299, STM32F303, IMU, USB and power">
</p>

---

## Firmware (M1)

[`firmware/m1_stm32_ads7828/`](firmware/m1_stm32_ads7828) has an STM32 HAL driver that reads all 8 channels sequentially from the ADS7828. For each channel it sends a single-ended command byte with the external reference selected, reads two bytes, and rebuilds the 12-bit sample.

This is an early polling version. M2 firmware has not been started.

---

## Repository Layout

```
hardware/
  m1_discrete_afe/      KiCad project: schematic + PCB (in progress)
  m2_ads1299/           KiCad project: schematic + PCB
firmware/
  m1_stm32_ads7828/     STM32 HAL driver for the ADS7828
docs/
  BCI_Design_Report.pdf Design report
images/                 Figures used in this README
```

---

## Roadmap

- [x] M1 schematic: protection, AFE, filters, RLD, power, ADC
- [x] M2 schematic: ADS1299, STM32F303, IMU, USB power
- [x] ADS7828 I²C read driver
- [ ] Finish M1 PCB routing with mixed-signal practice: split analog/digital grounds, clean return paths, isolate the AFE from switching regulators
- [ ] Fabricate and bring up M2
- [ ] Move firmware from polling to interrupt/DMA-driven acquisition so no samples are lost
- [ ] Stream data to a PC for real-time visualization
- [ ] Mount in an OpenBCI Ultracortex headset

---

## References

- Texas Instruments, [ADS1299 datasheet](https://www.ti.com/lit/ds/symlink/ads1299.pdf)
- Texas Instruments, [SBAA188: Improving Common-Mode Rejection Using the Right-Leg Drive Amplifier](http://www.ti.com/lit/an/sbaa188/sbaa188.pdf)
- TI E2E forum, [ADS1299: testing bias / right-leg drive](https://e2e.ti.com/support/data-converters-group/data-converters/f/data-converters-forum/696153/ads1299-ads1299-testing-bias-right-leg-drive)
- PolyCortex, [AcquisitionEEG](https://github.com/PolyCortex/AcquisitionEEG)
- [Sparky: testing and building](https://buildandcrash.blogspot.com/2013/05/sparky-testing-and-building-no-crashing.html)

---

**Author:** Armando Gordillo Ramírez
