# ZXPicoJoy
Programmable Joystick Interface for the ZX Spectrum using a Raspberry Pico.

Currently Supports:
- Kempston (on Port 0x1F)
- Sinclair 1 & 2
- Cursor
- Any Keyboard Key Combination
- Mixture of Kempston & Key Combinations

Uses an OLED and buttons to program and does not interfere with the Spectrum unless it is supposed to.

### Versions
v1.0 - First release out of beta. Lots of improvements and bug fixes since v0.2 initial release. Big thanks to @mbwum73 for testing and suggestions.
  - New menu layout making it easier to navigate and select presets
  - Added 8 configs which allows the user to create their own mappings. Configs are saved to flash so retained even after poweroff.
  - Added separate preset menu which can be accessed by holding the menu button down. This allows quick select of the mosts common joystick types.
  - Removed bluetooth screens till read, will split the build
  - Add info screen to show the current mappings.
  - Fixed kempston PIO code so it now doesn't double jump in some games
  - Fixed keyboard PIO so it now handles multiple key row reads at once (JSW & Manic Miner)

## UI Menu Layout: 9-Pin Wired Joystick
Upon entering the menu, you will see the 9-pin Wired Joystick image. 
- Press Enter (via the interface or the joystick) to enter the configuration sub-menus.
- Press Left (interface or joystick) to view the current button mappings and identify the active configuration/joystick type.
- If you hold the menu button instead of single press you will enter the preset menu which allows you to quickly select a common joystick types, including Kempston, Sinclair 1 & 2, Protek/Cursor, and Keyboard QAOPM.

### Sub-Menu Options
1. Config Selection - Access up to 8 independent configurations to store different mapping profiles (by default, the system starts on Config 1 with the Sinclair 1 (67890) profile). Other options are:
  - Save: Save the current setup to the active configuration number.
  - Save To: Save the current setup to a specific configuration number.
  - Delete/Reset Config: Revert the current configuration to its default (Sinclair 1).
  - Select a Preset: Quickly apply standard profiles (Kempston, Sinclair, Protek/Cursor, or QAOPM).
2. Redefine Button Mappings - Define the mapping for each button and direction. You can map inputs to either Key or Kempston commands, allowing you to mix and match types for each individual button.

### Joystick Set-up
The 9-pin wired joystick port supports up to 3 buttons and 4 directions (based on the Kempston extended interface & Amiga standard). 1 button joysticks are most common for the era and most games only work with a single fire button. 3 button joysticks are rare, I don't even have one to test, but 2 button ones are more available and the more modern gamepads are usually 2 button. This is useful as you can set these up to have button 1 as per the original and then button 2 can be any other key.

Default set-up is for 3 button Kempston (Amiga) with the option to flip button 2 to pin 7 enabling the use of 2 button Amstrad joysticks using the jumpers on the PCB. There is also an option to supply power to pin 7 for autofire circuits. It is wired to 3V3 instead of 5V so may not work on all joysticks, however, all the ones I have tried work fine as they are simple transistor circuits.

**Note you cannot use Megadrive/Genesis Pads with this interface as the wiring is different (see table below)**

| Pin | Sinclair IF2 | Kempston (Extended) | Atari/C64 | Amstrad CPC | Amiga | Atar ST | Megadrive | 
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Up | Up | Up | Up | Up | Up | Up |
| 2 | Down | Down | Down | Down | Down | Down |
| 3 | Left | Left | Left | Left | Left | Left | Left |
| 4 | Right | Right | Right | Right | Right | Right | Right |
| 5 | NC | Button 3 | Analogue 1 | Button 3| NC | Paddle B | 5V |
| 6 | Button 1 | Button 1 | Button 1 | Button 2 | Button 1 | Trigger | Button C |
| 7 | NC | 5V | 5V | Button 1 | 5V | 5V | GND |
| 8 | GND | GND | GND | COM | GND | GND | Select |
| 9 | NC | Button 2 | Analogue 2 | COM2 | Button 2 | Paddle A | Button B |

### The Code

The code uses the PIO fed by DMAs to convert joystick button press into the correct databus output for either Kempston or Keyboard. The hardware design allows the combination of the top 8bits of the address bus and the 7 joystick bits to be used as a lookup of an array (for Kempston it just uses the 7 joystick bits and ignores the address bus).

Example PIO code for keyboard
````
.program zxpicojoym
.side_set 2 opt // side set is used to switch on the buffer chips (one for data, one for address).
    pull block // get address header, top 17bits (add to 15bits from IN to make it a 32bits address location in PICO memory)
    mov x osr // put osr into x
.wrap_target
    mov isr x side 0b10 // get top 17bits from x -> when combined forms a 32bit address in Pico memory
    wait 0 gpio 20  // wait for KEYRD to go low
    in pins 15 // shift in 15 bits from i/o pins to ISR, auto push enabled ISR to RX FIFO -> DMA
    pull block // bring in data from DMA      
    mov y osr // copy OSR to y
    jmp !y _nodata // if y=0 then skip data output (this stops it interfering with the normal keyboard)
    out pins 8 side 0b01 // shift 8 bits from OSR to i/o pins
_nodata:
    wait 1 gpio 20 // wait for KEYRD to go high
.wrap // loop
````

#### Jet Set WIlly & Manic Miner

The original code was set-up to mirror the ROM keyread scanning routine, so for each half row scan address (5 keys in each half row) it presents the correct byte to the data bus. This worked fine apart from Jet Set Willy & Manic Miner. Reviewing the Jet Set Willy disassembly it turns out the code actually scans multiple half rows at once, basically as they key mappings are the same across multiple rows (key 5 goes left, so does key 6 which is the same byte but different half rows). Simply adapting the output so it works with combined half row addresses fixed this issue.

| Adress | Keys |
| --- | --- |
| A8 Low | CAPS SHIFT, Z, X, C, V |
| A9 Low | A, S, D, F, G |
| A10 Low | Q, W, E, R, T |
| A11 Low | 1, 2, 3, 4, 5 |
| A12 Low | 0, 9, 8, 7, 6 |
| A13 Low | P, O, I, U, Y |
| A14 Low | ENTER, L, K, J, H |
| A15 Low | SPACE, SYMBOL SHIFT, M, N, B |

Normal operation only one address bit goes low for each key read. For Jet Set Willy A11 & A12 are both low on the same read so 1=0 2=9 3=8 4=7 & 5=6.

### Full Menu Layout
![image](.//images/screenlayout.png "Menu Layout")

### Config Menu Layout
![image](.//images/configmenu.png "Config Menu")

## The Circuit
![image](.//images/schematic.png "Schematic")

## The PCB
PCB designed to have all the components on the side away from the Spectrum to give more room. The edge connector then sits on the back side as well as the OLED so this is visible to the user. This is also why the buttons are reversed. 
### PCB v1.6c Front
![image](.//images/zxpiocojoy_v1.6c.png "PCB Front")
### PCB v1.6c Back
![image](.//images/zxpiocojoy_v1.6c_back.png "PCB Back")

### Bill of Materials (BoM)
- 1x [Raspberry Pico](https://shop.pimoroni.com/products/raspberry-pi-pico?variant=40059364311123). Recommend H variant as header pre-soldered.
- 2x [SN74LVC245AN Octal Bus Transceiver With 3-State Outputs](https://www.mouser.co.uk/ProductDetail/595-SN74LVC245AN). Note these need to be the LVC variant to work with the 3v3 of the Pico.
  - Optional 2x10pin sockets
- 1x [CDx4HC4075 Triple 3-Input OR Gates](https://www.mouser.co.uk/ProductDetail/595-CD74HC4075EE4). Note these need to be the HC variant in order to work with 3V3 supply voltage.
  - Optional 2x7pin socket
- 3x [100nF/0.1uF Capacitor](https://www.mouser.co.uk/ProductDetail/Vishay-BC-Components/K104K15X7RF5TH5?qs=CuWZN%2F5Vbiofhf%252BuZNGw%2Fg%3D%3D) 
- 1x [1N4001 Diode](https://www.mouser.co.uk/ProductDetail/Rectron/1N4001-B?qs=%252BtLcN0raKGUACwkD5chVvg%3D%3D) or [1N5158 Diode](https://www.mouser.co.uk/ProductDetail/STMicroelectronics/1N5817?qs=JV7lzlMm3yKNnxZdh%252BSMnw%3D%3D)
- 1x [56pin PCB Edge Connector](https://www.ebay.co.uk/sch/i.html?_from=R40&_trksid=p2047675.m570.l1313&_nkw=zx+spectrum+edge+connector&_sacat=0). Needs both ends cutting off, the pins removing top & bottom at position 5 and a piece of plastic or metal inserting for the slot (see [ZX Spectrum Edge Connector](https://i0.wp.com/projectspeccy.com/documents/ZXSpectrum_Edge_Connector_Diagram_48K.jpg)). This ensures everything aligns when connecting.
- 2x [20pin female header sockets](https://www.ebay.co.uk/sch/i.html?_from=R40&_trksid=p2380057.m570.l1313&_nkw=20pin+female+header+socket&_sacat=0)
- 5x [6x6x9mm Right Angled PCB Momentary Push Button](https://www.ebay.co.uk/sch/i.html?_from=R40&_trksid=p2380057.m570.l1313&_nkw=6x6x9mm+Right+Angled+PCB+Momentary+Push+Button&_sacat=0)
- 1x [6x6x9mm PCB Momentary Push Button](https://www.ebay.co.uk/sch/i.html?_from=R40&_trksid=p2334524.m570.l1313&_nkw=6x6x9mm+PCB+Momentary+Push+Button&_sacat=0&LH_TitleDesc=0&_osacat=0&_odkw=6+x+6+x9mm+right+angled+pcb+momentary+push+button)
- 1x [DE9 9pin Male PCB Connector](https://www.ebay.co.uk/sch/i.html?_from=R40&_trksid=p2334524.m570.l1313&_nkw=db9+9pin+pcb+male+connector&_sacat=0&LH_TitleDesc=0&_osacat=0&_odkw=db9+9pin+pcb+male+connector)
- 1x 4pin PCB header socket 2.54mm Pitch
- 1x SSD1306 OLED 0.96" (you can get larger ones just make sure they are SSD1306). Be very careful of the GND & VCC placement as they are sometimes reversed.
- 1x 2x2pin PCB pin header + jumpers (default set-up is Pin 7 to 3V3 & Pin 9 to Button 2)

### Photos
![image](.//images/IMG_4917.JPEG "Unit Front")
![image](.//images/IMG_4918.JPEG "Unit Back")
![image](.//images/IMG_4920.JPEG "Plugged In")
![image](.//images/IMG_4922.JPEG "Option 1")
![image](.//images/IMG_4923.JPEG "Option 3")
![image](.//images/IMG_4927.JPEG "Option 4")
![image](.//images/IMG_4928.JPEG "Option 5")
![image](.//images/IMG_4929.JPEG "Option 7")
