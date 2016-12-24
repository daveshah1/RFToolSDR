# RFTool Rev 1.0 Hardware Errata and Notes
 - The Schottky diode for the +3.3V buck converter is missing. Connect a
Schottky such as the MBR0520 between ground and the SW pin of U4.
 - The connector part in the BOM used for P5 (JTAG) does not have enough clearance
to fit the connector on the JTAG ribbon cable, at least for the cable included with my Waveshare
dongle. Ideally another connector should be used.
 - Trace widths will need to be adjusted to meet impedance requirements depending on your stackup.
 `rftool_impedances.png` shows impedance control requirements. Green is 50Ω single ended; blue is 50Ω differential
 and yellow is 75Ω differential. The USB 3 pairs were not included in this diagram but should be 100Ω differential.
 - It is not possible to use both receive channels simultaneously as channel 1 uses ports 1A and 1B; whereas 2 uses port 2C
 and the AD9361 does not support using different port letters at the same time. Ideally the design should be changed
 so the second LNA connects to 2B; or the first LNA connects to 1C.
 - The ENABLE pin of the Skyworks LNAs (pin 6 of U2 and U21) is active low but was mistakenly connected to +3V rather than
 ground. Cutting the trace going to this pin enables the LNAs to work properly.
