# ZXPicoJoy
Programmable Joystick Interface for the ZX Spectrum using a Raspberry Pico.

Currently Supports:
- Kempston (on Port 0x1F)
- Sinclair 1 & 2
- Cursor
- Any Keyboard Key Combination
- Mixture of Kempston & Key Combinations

Uses an OLED and buttons to program and does not interfere with the Spectrum unless it is supposed to.

## UI Menu Layout

Upon entering the menu you are shown the 9 pin Wired Joystick Image. Pressing enter (either on the interface or fire on the joystick) moves the menu to the set-up sub-menus for this joystick type.
- Pressing Left (either on the interface or with the joystick) shows the Information Screen showing the current button mappings and which config/joystick type is active.
- Pressing Right goes the the Bluetooth controller menu which has not been implemented yet so doesn't do anything
  - Pressing Right again goes to the Bluetooth Connection Menu which again has not been implemented yet so doesn't do anything

The sub-menus are as follows:
- Activate Joystick Type (this is always on for the 9pin wired Joystick as Bluetooth not implemented yet)
- Config Selection - There are 8 independent configurations so you can store the different mappings once you've set them up. To start with config 1 is selected and all configs are defaulted to Sinclair 1 (67890).
  - In the config menu there is an option to save the current set-up in the currently active config number. This is important as changing teh layout doesn't automatically save to flash. Save is only needed if you want to retain the set-up after a power down.
  - Other two options are Save To, which allows you to save the current set-up to a new Config number & Delete/Reset Config which jsut resets the config to default (Sinclair 1). Both these write to flash automatically.
- Redefine Button Mappings - This sub-menu allows you to select what each button (and direct) do. This can be either a key or kemston and each button can be mapped to anything. This means you can have a combination of keys and kempston if you wish.
- Preset (Kempston, Sinclair 1 & 2, Protek/Cursor) - This simply maps all the buttons and directions to the standard layout so for Sinclair 1 this will be 6=Left, 7=Right, 8=Down, 9=Up and Button 1=0. For Kempston this is a one to one mapping.

For the 9pin wired joystick you can map up to 3 buttons and 4 directions. 

![image](.//images/screenlayoutxcf.png "Menu Layout")

## The Circuit
![image](.//images/schematic.png "Schematic")

## The PCB
PCB designed to have all the components on the side away from the Spectrum to give more room. The edge connector then sits on the back side as well as the OLED so this is visible to the user. This is also why the buttons are reversed. 
### PCB v1.6c Front
![image](.//images/zxpiocojoy_v1.6c.png "PCB Front")
### PCB v1.6c Back
![image](.//images/zxpiocojoy_v1.6c_back.png "PCB Back")

### Joystick Pin-Out

The joystick is wired for 3 button Kempston (Amiga) with the option to flip button 2 to pin 7 enabling the use of 2 button Amstrad joysticks. There is also an option to supply power to pin 7 for autofire circuits. It is wired to 3V3 instead of 5V so may not work on all joysticks, however, all the ones I have tried work fine as they are simple transistor circuits.

- Pin 1: Up
- Pin 2: Down
- Pin 3: Left
- Pin 4: Right
- Pin 5: Button 3
- Pin 6: Button 1
- Pin 7: Button 2 for Amstrad compatibility, not connected or 3V3
- Pin 8: GND
- Pin 9: Button 2 or not connected

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
- 1x 2x2pin PCB pin header + jumpers

### Photos
![image](.//images/IMG_4917.JPEG "Unit Front")
![image](.//images/IMG_4918.JPEG "Unit Back")
![image](.//images/IMG_4920.JPEG "Plugged In")
![image](.//images/IMG_4922.JPEG "Option 1")
![image](.//images/IMG_4923.JPEG "Option 3")
![image](.//images/IMG_4927.JPEG "Option 4")
![image](.//images/IMG_4928.JPEG "Option 5")
![image](.//images/IMG_4929.JPEG "Option 7")
