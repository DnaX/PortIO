PortIO
======

PortIO is a simple program that allows you to drive hardware connected to the PC parallel port for educational purposes. The graphical interface is divided into four parts with different functions. Unfortunately GUI and most of the code it's writte in Italian language.

I'm sorry in advance for the ugly coding style, it's an old high school project.

RAW output
----------
The first section concerns the direct sending of data to be presented on the parallel port data bus, useful in the case of manual connection of a DAC. Here it is possible to set the logical value (0 or 1) all 8 bits of the data register or set the decimal representation of the output (in practice from 0 to 255).

Function generation
-------------------
The second section concerns a function generator with a resolution of 8 bits on the data bus. The available waveforms are: triangular, sinusoidal and square. The frequency is adjustable via a slider and depending on the generated function can reach 1kHz. This section is also particularly suitable for driving an ADC.

ADC Acquisition
---------------
The third section is instead dedicated to the acquisition of 4 analog voltages by an ADC such as the ADC0808. The program is developed around this particular ADC despite the fact that it has 8 multiplexed analog inputs while the program is developed to manage only 4 of these inputs. In the ADC section, in addition to displaying the decimal representation read from the data bur from the parallel port, it also performs a conversion to show the correct value in volts for a linear scale from 0V to 5V.

LCD Output
----------
A secondary function is to be able to drive a display with HD44780 chip or compatible. Once the display is initialized, it is possible to write arbitrary text that will be immediately displayed. For how to make the connections to the correct pins of the parallel port, I refer to the lcd4linux guide (aka winamp connection).

Windows version
---------------
The Windows version, tested with Windows 2000/XP, uses a custom made device driver called "ISAKerPlug" to access the parallel port (missing). The control GUI was developed in C++ with Borland C++Builder IDE.

![win_screen1](screenshots/portio_win1.png?raw=true)
![win_screen2](screenshots/portio_win2.png?raw=true)

Linux version
-------------
The Linux version was developed in C with GTK+ 2 toolkit and compile with gcc and Makefile. 

The Linux version does not have the LCD driver feature but adds an oscilloscope-like graph on CH1 with timebase limited to the speed of the ADC. The graph uses the uber-graph code by Christian Hergert.

You will probably need to use root permissions to run the program to have direct IO access.

![lin_screen1](screenshots/portio_lin1.png?raw=true)
![lin_screen2](screenshots/portio_lin2.png?raw=true)

Hardware
--------
The switch between the ADC and DAC mode it's done using the STROBE signal in the parallel port. The diagram of the controlled circuit with the relative wiring of the parallel port is available here.

![schema](Schema-ADC-DAC.PNG?raw=true)
