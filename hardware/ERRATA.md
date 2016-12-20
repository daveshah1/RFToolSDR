# RFTool Rev 1.0 Hardware Errata and Notes
 - The Schottky diode for the +3.3V buck converter is missing. Connect a
Schottky such as the MBR0520 between ground and the SW pin of U4.
 - The connector part in the BOM used for P5 (JTAG) does not have enough clearance
to fit the connector on the JTAG ribbon cable, at least for the cable included with my Waveshare
dongle. Ideally another connector should be used.
 - Trace widths will need to be adjusted to meet impedance requirements depending on your stackup.
 `rftool_impedances.png` shows impedance control requirements. Green is 50Ω single ended; blue is 50Ω differential
 and yellow is 75Ω differential. The USB 3 pairs were not included in this diagram but should be 100Ω differential.
 
