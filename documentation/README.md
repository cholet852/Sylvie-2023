### Introduction to the EMC2 Servo

(Deprecated. Due for an update soon)

The primary reason behind 'real' robotic home appliances not being a common household item is the lack of affordable actuators/gearboxes.

For a long time, 3d printed planetary gearboxes were considered toys. Thanks to advances in the production of 3d printed filament as well as the greater range of materials available to us, this is no longer the case.

Nylon is considered a great filament for 3d printed gears, but in my experience PETG is way better. Nylon tends to have poor layer adhesion and high warpning which makes it inaccessible to most users.
This may not be the case with Nylon 12 Carbon Fibre filaments. Nylon 12 CF has less moisture absorption, and the fact that it is filled with 15-20% carbon should in theory reduce the warping.

Nylon filament is obviously more expensive, especially Nylon 12 CF, but 3d printing gears doesn't require that much filament to begin with. You should be able to print gears for the whole robot using only one or two 1kg spools. One need not fret too much about gears melting at high speed operations so long as they are lubricated with either a lithium grease, or a silicone lubricant (recommended).

AS5600 magnetic encoders are incredibly cheap and also incredibly tiny, resulting in a light and compact form factor. The EMC2 Servo uses Nema 17 stepper motors, AS5600 encoders, ESP32 microcontrollers and TMC2130 stepper drivers in SPI mode with a motor voltage of 36v, allowing one to change driver current on the fly and significantly reduce power consumption.

Planetary gear ratios used include 66:1 as well as 53.6:1, multiplied by the bevel gear ratio if using a dual gearbox setup for bilateral mechanical stability. The 3D printed bearings are almost indestructible and make use of 4.5mm steel bb's.

The new and latest Balance Mode feature allows one to 'lock the joint in its current position and follow the direction of gravity'. Exiting balance mode keeps the joint at its last movement angle while also remembering the exact position on the rotary encoder. The rotary encoder ensures that no steps will be lost, and it will automatically move to its correct position if stalling is detected.

Our goal is to keep robotics as simple and as accessible as possible. Custom printed PCB's will soon be available this year - which will make it much easier to assemble these servos from scratch with minimal soldering.

### Current development state of EMC2 Servo

The code and hardware of the EMC2 servo are fully functional and [should] be bug-free by now. It's a little bit tricky to assemble the 3d printed parts at the moment as it's a very early prototype, though I intend to release a second updated version that will make installation, alignment and maintenance a lot easier.

The second version of the EMC2 Servo kit will be available for sale once I finish designing and ordering my custom made PCB's. All components are modular and easily sourced, meaning if you accidentally fry e.g. the ESP32 MCU while tinkering, you only need to replace the ESP32 MCU, and not the whole PCB.

If you then wish to write new and better code for the servo, that'll also be a walk in the park!

### Overpowering the I2C Protocol

I2C was originally meant to be used on pcbs for short-distance chip communication. The proliferation of smartphones gave us an oversupply of cheap sensors such as the MPU6050 accelerometer, IR Temperature sensors, IR range sensors, compass modules and more. Because the chips were already designed to use the i2c protocol, it thus proved easier just to simply attach some cables to the pins, than it was to redesign the chips from scratch.

The protocol tends to handle distances of up to 50cm without any issues. However, any length greater than that tends to increase capacitance, which can distort the signals and eventually cause I/O errors. 

Say hello to the I2C Splitter: a system consisting of two arduino nanos (soon to be replaced with a single ESP32) capable of taking in any message in i2c, and repeating it in the form of a brand new signal. Not only does this mean that we can now use lengths greater than 50cm, it also means we can re-use i2c addresses. So it doubles as an i2c multiplexer.

For extra signal protection, you should ideally use twisted pair and shielded cable. Where each i2c line (SDA, SDL) is twisted with either ground, or 3.3v/5v power. We now have a cheap and ridiculously simple way of communicating with our servos in order to control a human-sized robot.

Sylvie 2023 uses 2x i2c splitters for each leg. The Super Master arduino controls both top splitters located at the thighs (address 0x61 for right leg, 0x62 for left left). The splitters at the thigh, then control the splitters on the knees(address 0x64 for both). As well as the servos located within that scope.

### Some notes on the semiconductor shortage

Due to the recent 2020-2022 chip shortage, we are now experiencing low stock for Arduinos and Raspberry Pi's. ESP32's don't seem to be affected. In fact, it's cheaper to buy an ESP32 right now than it is to buy an Arduino Nano, even. All my arduinos will eventually be replaced with ESP32's in order to avoid the use of level shifters in future. TMC2130 v1.1's are also out of stock all around the world (the stepper drivers used for the EMC2 Servo). 

TMC2130 v1.2 and v3.0 (from BIGTREETECH) however, are still being sold. One is unfortunately only meant for FYSTEC F6 3d printer boards only, and the other is advertised as being v24 max by some sellers (they are still the same chip though, so it should work) - but not all hope is lost. In order to modify a TMC2130 v1.2 into a v.1.1, one must solder the 6 slots located at the bottom right corner of the actual chip, underneath the stepper driver pcb. I'm still not sure whether the v3.0 requires this modification, but it might pay to have a look at the CFG slots. 

- v1.2 top row (CFG5): [X][X][X], bottom row (CFG4): [X][X][X]
- v1.1 top row (CFG5): [X][X__X], bottom row (CFG4): [X__X][X]   

Don't try this with a large soldering iron. Get a 5v USB mini soldering pen and plug it into an adapter with low current (maybe 1A or 1.5A) to keep temperature low. If you already know how to do soldering work at the chip-scale, then power to you.

TMC2130s still continue to be very low on stock, and it's only a matter of time until every single version is completely sold out. The DRV8825's can still be used interchangably on the EMC2 PCB (provided you upload the code for it), you just won't be able to control the current on the fly. So it's best to save the TMC2130s for the waist and the balancing thighs, and then simply use DRV8825s for the rest of the body. Take that, supply chains!   

Apparently, there also exists a FYSETC TMC5160 QHV 60V, which claims an input voltage of up to 60v and currents of up to 4.2A, complete with SPI control. Code for arduino control of this motor driver is also available on Github. I haven't tested that one yet, but they're at least 3x as expensive as the TMC2130 on Aliexpress, unfortunately. Not that we couldn't experiment with them in future versions of the EMC2 servo in order to double our stepper motor speed and torque. Torque (current) is often more important than voltage, as we can lower our gear ratio for higher speeds, as opposed to acquiring a Li-ion battery with a voltage higher than 36v. 

Your stepper motor may, however, only have a low torque and current limit, in which case using higher voltages and higher gear ratios is the better option.

### Some notes on AS5600

AS5600 is cheap but a bit tricky to work with at first. A lot can go wrong. First, it's important to make sure that your DIR pin is pulled either HIGH (VCC) or low (GND), either by controlling it from software, or by soldering it to the ground or the 3.3v power pin, depending on whether you're working with a clockwise, or anti-clockwise setup. Leaving the DIR pin floating will cause random problems down the road.

Another problem I've frequently encountered with this magnetic encoder is magnet slipping from knocks and strong vibrations. One easy solution to this problem is to apply a drop of epoxy or superglue to the back of the stepper shaft and then place the magnet as centered as you can get it. Epoxy will probably give you more time to adjust (just don't use JB Weld for this job because not only is it overkill - it's actually magnetic!)

Last, but not least - make sure you buy the right magnets. Magnets must be diametrically magnetized. They must touch each other side-by-side like a chain as opposed to merely stacking on top of each other. When stuck side-by-side, try pushing them up and down. If they spring back into place like a guitar string, they're good magnets.

6x3mm magnets work best for 48mm and 40mm and 34mm length steppers. 6x2mm or 5x2mm work best for 22mm length pancake steppers, but it might vary from manufacturer to manufacturer. Buy a bunch of diametrically magnetized magnets before you buy AS5600 because a lot of sellers online will sell you the wrong magnets.

The software serial of the EMC2 servo will tell you if your magnet is positioned too close, too far or just in the right place. If for whatever reason the encoder is knocked out of its position during movement, the EMC2 and stepper movement will immediately freeze until it is put back in its place. You might have to tweak the AS5600 holder models a bit depending on your build. It's always best to use screws to attach them, but some steppers such as the 48mm length model may require 50mm m3 machine screws which can be a bit difficult to source.

### Basic EMC2 Servo commands (Serial)

When connected to an i2c splitter, you will need to first invoke the joint number/letter. e.g. 0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f,g, etc.

Write in the format of [number][command] e.g. 1m10,10 | 0m10,10 | am10,10

To execute commands on multiple joints at once, simply add a semicolon ; and then write the next command e.g. 0m10,10;1m20,10;2m15,15;3b;4x

Full list of available commands for use in Serial:

- b : Initiate balancing mode (if MPU6050 installed)
- x : Exit balancing mode.
- h : Initiate homing sequence until limit switch pressed.
- c : Set current (tmc2130 only) e.g. c800 c1100
- m[value],[speed] : Move to angle e.g. m10 m0 m1 m3 m-5 or m10,50 m20,15 m-15,15
- s : Set speed e.g. s50 s30 s80
- p : Set balance PID kP e.g. p6.00
- i : Set balance PID kI e.g. i5.00
- d : Set balance PID kD e.g. d0.01
- l : Set balance speed e.g. l30
- n : Set minimum balance speed e.g. n60

Full list of subcommands:

- @ : Request data from servo e.g. m@, s@, c@
