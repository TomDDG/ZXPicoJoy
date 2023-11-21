// Versions
// v0.1 initial release copied from v0.2 of ZX PicoJoy
// v0.2 added button 3 mapping
//
#define PROG_NAME   "ZX PicoJoyM"
#define VERSION_NUM "v0.2"
//
// ---------------------------------------------------------------------------
// includes
// ---------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
//
#include "zxpicojoym.pio.h"
#include "zxpicojoym_k.pio.h"
#include "zxpicojoypress.pio.h"
//
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/flash.h"
#include "hardware/i2c.h" // for OLED
//
#define SET_SPLASH 2000
#define KEY_REPEAT_LONG 500000
#define KEY_REPEAT_SHORT 200000
//
// ---------------------------------------------------------------------------
// GPIO Assignments
// ---------------------------------------------------------------------------
// 0 A8/D0
#define PIN_A8D0    0
// 1 A9/D1
// 2 A10/D2
// 3 A11/D3
// 4 A12/D4
// 5 A13/D5
// 6 A14/D6
// 7 A15/D7
//
// 8 RIGHT (ph) rldu123m
#define MAP_MBUDLR  0b1001111100000000
#define MAP_MBLR    0b1001001100000000
//
#define PIN_RIGHT   8
#define MAP_RIGHT   0b0000000100000000
// 9 LEFT (ph)
#define PIN_LEFT    9
#define MAP_LEFT    0b0000001000000000
// 10 DOWN (ph)
#define PIN_DOWN    10
#define MAP_DOWN    0b0000010000000000
// 11 UP (ph)
#define PIN_UP      11
#define MAP_UP      0b0000100000000000
// 12 BUTTON 1 (ph) also ENTER in MENU
#define PIN_B1      12
#define MAP_B1      0b0001000000000000
// 13 BUTTON 2 (ph)
#define PIN_B2      13
#define MAP_B2      0b0010000000000000
// 13 BUTTON 3 (ph)
#define PIN_B3      14
#define MAP_B3      0b0100000000000000
// 15 MENU (ph)
#define PIN_MENU    15
#define MAP_MENU    0b1000000000000000
// 16 ADDON
#define PIN_ADDON   16
// 17 DATAON
#define PIN_DATAON  17
// 18 A5RD
#define PIN_A5RD    18
// 19 KEMP
#define PIN_KEMP    19
// 20 KEYRD
#define PIN_KEYRD   20
// 21 NC
// 22 NC
// 23 NC
// 24 NC
// 25 NC
// 26 SDA (OLED)
#define PIN_SDA     26
// 27 SCL (OLED)
#define PIN_SCL     27
// 28 NC
//
// ---------------------------------------------------------------------------
// I/O Chips
// ---------------------------------------------------------------------------
// BUFFER 1 (3V3) DATA(A-B) -> DATON(P) Switches ON(0)/OFF(1)
// D0A8(P)   -> D0(E)
// D1A9(P)   -> D1(E)
// D2A10(P)   -> D2(E)
// D3A11(P)   -> D3(E)
// D4D12(P)   -> D4(E)
// D5A13(P)   -> D5(E)
// D6A14(P)   -> D6(E)
// D7A15(P)   -> D7(E)
// DATAON OE
//
// BUFFER 2 (3V3) ADD(B-A) -> ALWAY ON
// D0A8(P)  <- A8(E)
// D1A9(P)  <- A9(E)
// D2A10(P) <- A10(E)
// D3A11(P) <- A11(E)
// D4A12(P) <- A12(E)
// D5A13(P) <- A13(E)
// D6A14(P) <- A14(E)
// D7A15(P) <- A15(E)
// ADDON OE
//
// LOGIC OR (3V3)
// IORQ(E)+RD(E)+A0(E)   -> KEYRD(P)
// A5(E)+IORQ(E)+RD(E)   -> A5RD(O)
// A6(E)+A7(E)+A5RD(O)   -> KEMP(P)
//
// ---------------------------------------------------------------------------
// keyRow which address to alter
// keyCol which bit to alter
// ---------------------------------------------------------------------------
    const uint8_t keyRow[]= { 3,3,3,3,3, // 1,2,3,4,5
                              4,4,4,4,4, // 6,7,8,9,0
                              2,2,2,2,2, // Q,W,E,R,T
                              5,5,5,5,5, // Y,U,I,O,P
                              1,1,1,1,1, // A,S,D,F,G
                              6,6,6,6,6, // H,J,K,L,ENTER
                              0,0,0,0,0, // SHIFT,Z,X,C,V
                              7,7,7,7,7, // B,N,M,SS,SPACE
                              0 }; // kempston
    //  
    const uint8_t keyCol[]= {   0b10111110, // 1
                                0b10111101, // 2
                                0b10111011, // 3 
                                0b10110111, // 4
                                0b10101111, // 5
                                //
                                0b10101111, // 6
                                0b10110111, // 7
                                0b10111011, // 8
                                0b10111101, // 9
                                0b10111110, // 0
                                //
                                0b10111110, // Q
                                0b10111101, // W
                                0b10111011, // E
                                0b10110111, // R
                                0b10101111, // T
                                //
                                0b10101111, // Y
                                0b10110111, // U
                                0b10111011, // I
                                0b10111101, // O
                                0b10111110, // P
                                //
                                0b10111110, // A
                                0b10111101, // S
                                0b10111011, // D
                                0b10110111, // F
                                0b10101111, // G
                                //
                                0b10101111, // H
                                0b10110111, // J
                                0b10111011, // K
                                0b10111101, // L
                                0b10111110, // ENTER
                                //
                                0b10111110, // SHIFT
                                0b10111101, // Z
                                0b10111011, // X
                                0b10110111, // C
                                0b10101111, // V
                                //
                                0b10101111, // B
                                0b10110111, // N
                                0b10111011, // M
                                0b10111101, // SS
                                0b10111110, // SPACE
                                //
                                0b11111111 // kempston
                                };
//
// ---------------------------------------------------------------------------
// OLED structure
// ---------------------------------------------------------------------------
typedef struct {
    uint8_t w;      // width 128
    uint8_t h;      // height 64
    uint8_t p;      // pages 8
    uint8_t a;      // i2c address of display
    i2c_inst_t *i;  // i2c connection instance
    bool e;         // whether display uses external vcc
    uint8_t *c;	    // display buffer with command
    uint8_t *b;	    // display buffer 
    size_t s;		// total display buffer size
} oled_t;
#define MEMSIZE 1025 // 128*64=8192bits -> 1024bytes + 1 byte
#define MEMS_OLED 0 // start of main buffer used for OLED buffer
uint8_t mem_buffer[MEMSIZE];
//
// ---------------------------------------------------------------------------
// OLED command constants
// ---------------------------------------------------------------------------
#define SET_CONTRAST        0x81
#define SET_ENTIRE_ON       0xA4
#define SET_NORM_INV        0xA6
#define SET_DISP            0xAE
#define SET_MEM_ADDR        0x20
#define SET_COL_ADDR        0x21
#define SET_PAGE_ADDR       0x22
#define SET_DISP_START_LINE 0x40
#define SET_SEG_REMAP       0xA0
#define SET_MUX_RATIO       0xA8
#define SET_COM_OUT_DIR     0xC0
#define SET_DISP_OFFSET     0xD3
#define SET_COM_PIN_CFG     0xDA
#define SET_DISP_CLK_DIV    0xD5
#define SET_PRECHARGE       0xD9
#define SET_VCOM_DESEL      0xDB
#define SET_CHARGE_PUMP     0x8D
//
// ---------------------------------------------------------------------------
// functions
// ---------------------------------------------------------------------------
void runOLEDcmd(oled_t *pOLED,uint8_t v);
void dtoBuffer(uint8_t *to,const uint8_t *from);
void showOLED(oled_t *pOLED,uint8_t col,uint8_t page,size_t size);
void oledInit(oled_t *pOLED,uint8_t w,uint8_t h,uint8_t a,i2c_inst_t *i,bool e);
void pltIcons(uint8_t row,uint8_t col,const uint8_t *icon,uint8_t *to,bool invert);
void pltAllIcons(uint8_t *to,uint8_t iconStart,uint8_t iconPos);
void pltIcon(const uint8_t *icon,uint8_t *to);
uint8_t getKey(oled_t *pOLED, uint8_t *to,uint8_t iconShift);
void menu(oled_t *pOLED, uint8_t *to);
void writeConfig();
void clrFlash(oled_t *pOLED, uint8_t *to);
void resetAll();
//
// ---------------------------------------------------------------------------
// menu layout
// ---------------------------------------------------------------------------
#include "scr_img.h"
const uint8_t *scrimg[] = { scr_00, // 0
                            scr_01, // 1
                            scr_0101,scr_010001a,scr_010001b,scr_010002,scr_010003, //  2, 3, 4, 5, 6
                            scr_0102,scr_010001a,scr_010001b,scr_010002,scr_010003, //  7, 8, 9,10,11
                            scr_0103,scr_010001a,scr_010001b,scr_010002,scr_010003, // 12,13,14,15,16
                            scr_0104,scr_010001a,scr_010001b,scr_010002,scr_010003, // 17,18,19,20,21
                            scr_0105,scr_010001a,scr_010001b,scr_010501,scr_010502,scr_010503,scr_010504,scr_010505,scr_010002,scr_010003, // 22,23,24,25,26,27,28,29,30,31
                            scr_02, // 32
                            scr_0201,scr_0202, // 33,34
                            scr_010001c, // 35
                            scr_reset // 36
                             }; 
//
// ---------------------------------------------------------------------------
// action on enter press
// ---------------------------------------------------------------------------
// =0 do nothing
// <128 move to screen x
// =128 key selector 1
// =129 key selector 2 (includes additiohnal buttons)
// =130 activate (reset 3,7,11,15,19)
// =131 reset flash
//                           ss  jy  kp  a1  a2  b2  b3  s1  a1  a2  b2  b3  s2  a1  a2  b2  b3  cr  a1  a2  b2  b3  ky  a1  a2  up  dn  lf  rg  b1  b2  b3  bt  ps  xb  sw  rs
//                            0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36
const uint8_t scrEnter[] = {    0,  
                                2,  
                                3,130,  0,129,129,
                                8,130,  0,129,129,
                               13,130,  0,129,129,
                               18,130,  0,129,129,
                               23,130,  0,128,128,128,128,128,129,129,
                               33,
                                0,  0,
                                0,
                              131};
//
// ---------------------------------------------------------------------------
// icon swap, what to swap to. also used to determine which joystick selected
// ---------------------------------------------------------------------------
//                   ss jy kp a1 a2 b2 b3 s1 a1 a2 b2 b3 s2 a1 a2 b2 b3 cr a1 a2 b2 b3 ky a1 a2 up dn lf rg b1 b2 b3 bt ps xb sw rs
//                    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36
uint8_t scrSwap[] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//
#define swapto 35
//
#define swp_kp 3 // kempston
#define swp_s1 8 // sinclair 1
#define swp_s2 13 // sinclair 2
#define swp_cr 18 // cursor
#define swp_ky 23 // keyboard
//
#define b2_kp 5
#define b3_kp 6
#define b2_s1 10
#define b3_s1 11
#define b2_s2 15
#define b3_s2 16
#define b2_cr 20
#define b3_cr 21
//
#define up_ky 25
#define dn_ky 26
#define lf_ky 27
#define rg_ky 28
#define b1_ky 29
#define b2_ky 30
#define b3_ky 31
//
// ---------------------------------------------------------------------------
// next, previous what screen to move to (0 means can't move)
// ---------------------------------------------------------------------------
//                         ss jy kp a1 a2 b2 b3 s1 a1 a2 b2 b3 s2 a1 a2 b2 b3 cr a1 a2 b2 b3 ky a1 a2 up dn lf rg b1 b2 b3 bt ps xb sw rs
//                          0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36
const uint8_t scrNext[] = { 0,32, 7, 5, 5, 6, 0,12,10,10,11, 0,17,15,15,16, 0,22,20,20,21, 0, 0,25,25,26,27,28,29,30,31, 0, 0,34, 0, 0, 1};
//                         ss jy kp a1 a2 b2 b3 s1 a1 a2 b2 b3 s2 a1 a2 b2 b3 cr a1 a2 b2 b3 ky a1 a2 up dn lf rg b1 b2 b3 bt ps xb sw rs
//                          0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36
const uint8_t scrPrev[] = { 0,36, 0, 0, 0, 3, 5, 2, 0, 0, 8,10, 7, 0, 0,13,15,12, 0, 0,18,20,17, 0, 0,23,25,26,27,28,29,30, 1, 0,33, 0, 0};
//
// ---------------------------------------------------------------------------
// config
// 0-9 header (ZXPicoJoyM)
// 10 current joystick selected (using scrSwap)
// 11 kempston b2
// 12 sinclair1 b2
// 13 sinclair2 b2
// 14 cursor b2
// 15-21 keys (inc. b3)
// 22 kempston b3
// 23 sinclair1 b3
// 24 sinclair2 b3
// 25 cusor b3
// ---------------------------------------------------------------------------
//                         h   h   h   h   h   h   h   h   h   h  wj b2 b2 b2 b2 ky ky ky ky ky ky ky b3 b3 b3 b3
//                         0   1   2   3   4   5   6   7   8   9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
const uint8_t config_d[]={'Z','X','P','i','c','o','J','o','y','M', 8, 1, 2, 2, 2,14,24,22,23,41,29,28, 0, 2, 2, 2};
#define cfg_cjy 10
#define cfg_kp2 11
#define cfg_s12 12
#define cfg_s22 13
#define cfg_cr2 14
#define cfg_key 15
#define cfg_ky2 20
#define cfg_ky3 21
#define cfg_kp3 22
#define cfg_s13 23
#define cfg_s23 24
#define cfg_cr3 25
#define cfg_end 26
uint8_t config[256];
//
// ---------------------------------------------------------------------------
// flash config allocated 256bytes
// if not present use default and create, otherwise read in
// ---------------------------------------------------------------------------
#define FLASH_TARGET_OFFSET (256 * 1024)
const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
//
// ---------------------------------------------------------------------------
// default keys and mappings
// ---------------------------------------------------------------------------
uint8_t keySel[7]; 
// maps to a key, 44 (-4=40) is do nothing, 0=b3, 1=b2, 2=b1, 3=up
// the keys in selector rows 
uint8_t kmpSel[7] = {44,44,44,44,44, 1, 0}; 
uint8_t s1Sel[7]  = {12,11, 9,10,13, 2, 2};
uint8_t s2Sel[7]  = { 7, 6, 4, 5, 8, 2, 2};
uint8_t curSel[7] = {10, 9, 8,11,13, 2, 2};
//
// ---------------------------------------------------------------------------
// key icons
//   0-b3  1-b2  2-b1  3-up
//   4-1   5-2   6-3   7-4
//   8-5   9-6  10-7  11-8  
//  12-9  13-0  14-q  15-w
//  16-e  17-r  18-t  19-y
//  20-u  21-i  22-o  23-p
//  24-a  25-s  26-d  27-f
//  28-g  29-h  30-j  31-k
//  32-l  33-en 34-cs 35-z
//  36-x  37-c  38-v  39-b
//  40-n  41-m  42-ss 43-sp
// ---------------------------------------------------------------------------
#include "icons.h"
const uint8_t *icons[]={ icon_b3,icon_b2,icon_b1,icon_up,
                         icon_1,icon_2,icon_3,icon_4,
                         icon_5,icon_6,icon_7,icon_8,
                         icon_9,icon_0,icon_q,icon_w,
                         icon_e,icon_r,icon_t,icon_y,
                         icon_u,icon_i,icon_o,icon_p,
                         icon_a,icon_s,icon_d,icon_f,
                         icon_g,icon_h,icon_j,icon_k,
                         icon_l,icon_en,icon_cs,icon_z,
                         icon_x,icon_c,icon_v,icon_b,
                         icon_n,icon_m,icon_ss,icon_sp                     
                         };
//
// ---------------------------------------------------------------------------
// main program
// ---------------------------------------------------------------------------
void main() {   
    //
    // ---------------------------------------------------------------------------------------
    // On PICO boot all gpio pins are set to input, 4ma drive strength, pull down and schmitt 
    // trigger enabled. On Spectrum boot M1 (read instruction) goes low after ~100ms so need 
    // stablility before this turning off both buffers first takes 19.6ms which is quick 
    // enough to ensure no interference with either address or data lines. Another option 
    // would be to control the RESET line using the PICO but not enough GPIO pins
    // ---------------------------------------------------------------------------------------
    //
    gpio_init(PIN_ADDON); // address on(0)/off(1)
    gpio_set_dir(PIN_ADDON,true); // set to out
    gpio_put(PIN_ADDON,1); // off till needed    
    gpio_init(PIN_DATAON); // data on(0)/off(1)
    gpio_set_dir(PIN_DATAON,true); //  set to out
    gpio_put(PIN_DATAON,1); // off till needed
    //
    gpio_init(PIN_KEYRD); // keyread
    gpio_set_dir(PIN_KEYRD,true); //  set to out
    gpio_put(PIN_KEYRD,1); // off till needed
    gpio_init(PIN_KEMP); // kempston
    gpio_set_dir(PIN_KEMP,true); //  set to out
    gpio_put(PIN_KEMP,1); // off till needed
    gpio_init(PIN_A5RD); // A5 (mouse) 
    gpio_set_dir(PIN_A5RD,true); //  set to out
    gpio_put(PIN_A5RD,1); // off till needed
    //
    // ---------------------------------------------------------------------------------------
    // for debugging
    // stdio_init_all();
    // while(!stdio_usb_connected());    
    // ---------------------------------------------------------------------------------------
    //
    // ---------------------------------------------------------------------------------------
    // button GPIOs & pull-ups, all are pull up so active wen connected to ground. 
    // Same as the hardware joystick
    // ---------------------------------------------------------------------------------------
    gpio_init(PIN_LEFT); 
    gpio_set_dir(PIN_LEFT,false); // input   
    gpio_pull_up(PIN_LEFT); // pull high so pressing goes low   
    gpio_init(PIN_RIGHT); 
    gpio_set_dir(PIN_RIGHT,false); // input        
    gpio_pull_up(PIN_RIGHT); 
    gpio_init(PIN_UP); 
    gpio_set_dir(PIN_UP,false); // input  
    gpio_pull_up(PIN_UP);
    gpio_init(PIN_DOWN); 
    gpio_set_dir(PIN_DOWN,false); // input  
    gpio_pull_up(PIN_DOWN);
    gpio_init(PIN_B1); 
    gpio_set_dir(PIN_B1,false); // input  
    gpio_pull_up(PIN_B1);
    gpio_init(PIN_B2); 
    gpio_set_dir(PIN_B2,false); // input       
    gpio_pull_up(PIN_B2); 
    gpio_init(PIN_B3); 
    gpio_set_dir(PIN_B3,false); // input       
    gpio_pull_up(PIN_B3); 
    gpio_init(PIN_MENU); 
    gpio_set_dir(PIN_MENU,false); // input    
    gpio_pull_up(PIN_MENU); 
    //
    // master dataset used to pass data from DMA to PIO, algined to 256 boundary so can use absolute address i.e. address pins = memory address
    volatile uint8_t dataout[256] __attribute__((aligned (256))); // for all address combinations (8bits)  
    volatile uint8_t joypress=0b11111111;    
    volatile uint8_t kemppress=0b00000000;
    //
    // ---------------------------------------------------------------------------------------
    // Set-up PIO, 4 state machines (SM) per PIO, 2 PIOs each with 4 SMs -> 1 PIO used & 3 SMs
    // PIO 0
    //   SM1 13 instructiuons: read address, write data, uses DMA
    //   SM2 2 instructions: read joystick, write to single byte, uses DMA
    //   SM3 7 instructions: write data, uses DMA
    // 22 instructions, max 32
    //
    // Also set-up associatedDMAs, 12 channels in total, 6 used
    // for SM2 & SM3 the DMA channels are identical and used to call each other
    // ---------------------------------------------------------------------------------------
    PIO pio=pio0; // use pio 0
    //      
    // ---------------------------------------------------------------------------------------
    // keyboard (PIO 0, SM 1)
    // ---------------------------------------------------------------------------------------
    uint joy_data_sm=pio_claim_unused_sm(pio,true); // grab an free state machine from PIO 0
    uint joy_data_offset=pio_add_program(pio,&zxpicojoym_program); // get instruction memory offset for the loaded program
    pio_sm_config joy_data_config=zxpicojoym_program_get_default_config(joy_data_offset); // get the default state machine config    
    sm_config_set_in_pins(&joy_data_config,PIN_A8D0);
    sm_config_set_in_shift(&joy_data_config,false,true,8); // shift left input pins into ISR, autopush
    // set-up OUT data pins
    for(uint i=PIN_A8D0;i<PIN_A8D0+8;i++) pio_gpio_init(pio,i); // initialise all 8 output pins, this is needed for ouput (not for input)
    sm_config_set_out_pins(&joy_data_config,PIN_A8D0,8); // set OUT pin base & number (bits)
    sm_config_set_out_shift(&joy_data_config,true,false,8); // right shift 8 bits of OSR to pins (D0-D7) with autopull off
    pio_sm_set_consecutive_pindirs(pio,joy_data_sm,PIN_A8D0,8,true); // set all output pins to output
    // set-up SET pins
    pio_gpio_init(pio,PIN_ADDON); // 
    pio_gpio_init(pio,PIN_DATAON);      
    sm_config_set_set_pins(&joy_data_config,PIN_ADDON,2);
    pio_sm_set_consecutive_pindirs(pio,joy_data_sm,PIN_ADDON,2,true);
    // initialise state machine but do not start yet
    pio_sm_init(pio,joy_data_sm,joy_data_offset,&joy_data_config); // reset state machine and configure it
    // 
    // ---------------------------------------------------------------------------------------
    // 2 DMAs for keyboard, 1 reads address from PIO -> 1 writes data to PIO
    // ---------------------------------------------------------------------------------------
    int addr_chan=dma_claim_unused_channel(true);
    int data_chan=dma_claim_unused_channel(true);
    // DMA move the requested memory data to PIO for output *DATA*
    dma_channel_config data_dma=dma_channel_get_default_config(data_chan); // default configuration structure
    channel_config_set_high_priority(&data_dma,true); // all high priority channels are considered first
    // channel_config_set_dreq - Select a transfer request signal in a channel configuration object
    // pio_get_dreq - Return the DREQ to use for pacing transfers to/from a particular state machine FIFO
    channel_config_set_dreq(&data_dma,pio_get_dreq(pio,joy_data_sm,true)); // transfer data to PIO SM
    channel_config_set_transfer_data_size(&data_dma,DMA_SIZE_8); // transfer 8bits at a time    
    // channel_config_set_chain_to - Set DMA channel chain_to channel in a channel configuration object
    channel_config_set_chain_to(&data_dma,addr_chan); // trigger addr_chan when complete
    // Configure all DMA parameters
    dma_channel_configure(data_chan,                    // dma channel
                            &data_dma,                  // pointer to DMA config structure
                            &pio->txf[joy_data_sm],     // write to -> PIO FIFO
                            dataout,                    // read from dataout array (absolute memory address)
                            1,                          // 1 transfer
                            false);                     // do not start transfer, triggered by addr_dma
    // DMA move address from PIO into the data DMA config *ADDRESS*
    dma_channel_config addr_dma=dma_channel_get_default_config(addr_chan); // default configuration structure
    channel_config_set_high_priority(&addr_dma,true); // all high priority channels are considered first
    channel_config_set_dreq(&addr_dma,pio_get_dreq(pio,joy_data_sm,false)); // receive data from PIO
    channel_config_set_read_increment(&addr_dma,false); // read from same address
    channel_config_set_chain_to(&addr_dma,data_chan); // trigger data channel when complete
    dma_channel_configure(addr_chan,                                    // dma channel
                            &addr_dma,                                  // pointer to DMA config structure
                            &dma_channel_hw_addr(data_chan)->read_addr, // write to -> Data DMA read address
                            &pio->rxf[joy_data_sm],                     // read from <- PIO FIFO
                            1,                                          // 1 transfer
                            true);                                      // start transfer immediately
    //
    pio_sm_put(pio,joy_data_sm,((dma_channel_hw_addr(data_chan)->read_addr)>>8)); // put start memory address into PIO shifted 8bits so when added to the 8 gpios it gives the correct memory location
    pio_sm_set_enabled(pio,joy_data_sm,true); // enable state machine 1       
    //
    // ---------------------------------------------------------------------------------------
    // joypress (PIO 0, SM 2)
    // ---------------------------------------------------------------------------------------
    uint joypress_data_sm=pio_claim_unused_sm(pio,true); // grab an free state machine from PIO 0
    uint joypress_data_offset=pio_add_program(pio,&zxpicojoypress_program); // get instruction memory offset for the loaded program
    pio_sm_config joypress_data_config=zxpicojoypress_program_get_default_config(joypress_data_offset); // get the default state machine config    
    sm_config_set_in_pins(&joypress_data_config,PIN_RIGHT);
    sm_config_set_in_shift(&joypress_data_config,false,true,7); // shift left input pins into ISR, autopush
    // initialise state machine but do not start yet
    pio_sm_init(pio,joypress_data_sm,joypress_data_offset,&joypress_data_config); // reset state machine and configure it   
    //
    // ---------------------------------------------------------------------------------------
    // 2 DMAs for joypress, both read joypress from PIO, 2 needed to make continuous
    // ---------------------------------------------------------------------------------------
    int joypress_chan=dma_claim_unused_channel(true);
    int joypress2_chan=dma_claim_unused_channel(true);
    // 2 identical DMAs to move hardware joystick press from PIO to joypress
    dma_channel_config joypress_dma=dma_channel_get_default_config(joypress_chan); // default configuration structure
    channel_config_set_high_priority(&joypress_dma,true); // all high priority channels are considered first
    channel_config_set_dreq(&joypress_dma,pio_get_dreq(pio,joypress_data_sm,false)); // receive data from PIO
    channel_config_set_transfer_data_size(&joypress_dma, DMA_SIZE_8);
    channel_config_set_read_increment(&joypress_dma,false); // read from same address
    channel_config_set_chain_to(&joypress_dma,joypress2_chan); // trigger joypress2_chan channel when complete    
    dma_channel_configure(joypress_chan,                    // dma channel
                            &joypress_dma,                  // pointer to DMA config structure
                            &joypress,                      // write to 
                            &pio->rxf[joypress_data_sm],    // read from <- PIO FIFO
                            8,                              // 1 transfer
                            true);                          // start transfer immediately 
    // need two identical DMAs to ensure they run infinitely
    dma_channel_config joypress2_dma=dma_channel_get_default_config(joypress2_chan); // default configuration structure
    channel_config_set_high_priority(&joypress2_dma,true); // all high priority channels are considered first
    channel_config_set_dreq(&joypress2_dma,pio_get_dreq(pio,joypress_data_sm,false)); // receive data from PIO
    channel_config_set_transfer_data_size(&joypress2_dma, DMA_SIZE_8);
    channel_config_set_read_increment(&joypress2_dma,false); // read from same address
    channel_config_set_chain_to(&joypress2_dma,joypress_chan); // trigger joypress_chan channel when complete
    dma_channel_configure(joypress2_chan,                   // dma channel
                            &joypress2_dma,                 // pointer to DMA config structure
                            &joypress,                      // write to 
                            &pio->rxf[joypress_data_sm],    // read from <- PIO FIFO
                            8,                              // 1 transfer
                            false);                         // do not start transfer, triggered by joypress_dma  
    //                           
    pio_sm_set_enabled(pio,joypress_data_sm,true); // enable state machine 2   
    //
    // ---------------------------------------------------------------------------------------
    // kempston (PIO 0, SM 3)       
    // ---------------------------------------------------------------------------------------
    uint joy_k_data_sm=pio_claim_unused_sm(pio,true); // grab an free state machine from PIO 1
    uint joy_k_data_offset=pio_add_program(pio,&zxpicojoym_k_program); // get instruction memory offset for the loaded program
    pio_sm_config joy_k_data_config=zxpicojoym_k_program_get_default_config(joy_k_data_offset); // get the default state machine config    
    // sm_config_set_in_pins(&joy_k_data_config,PIN_KEMP);
    // sm_config_set_in_shift(&joy_k_data_config,false,true,1); // shift left input pin into ISR, autopush
    // set-up OUT data pins
    sm_config_set_out_pins(&joy_k_data_config,PIN_A8D0,8); // set OUT pin base & number (bits)
    sm_config_set_out_shift(&joy_k_data_config,true,false,8); // right shift 8 bits of OSR to pins (D0-D7) with autopull off
    //pio_sm_set_consecutive_pindirs(pio,joy_k_data_sm,PIN_A8D0,8,true); // set all output pins to output
    // set-up SET pins 
    sm_config_set_set_pins(&joy_k_data_config,PIN_ADDON,2);
    //pio_sm_set_consecutive_pindirs(pio,joy_k_data_sm,PIN_ADDON,2,true);
    // initialise state machine but do not start yet
    pio_sm_init(pio,joy_k_data_sm,joy_k_data_offset,&joy_k_data_config); // reset state machine and configure it
    // 
    // ---------------------------------------------------------------------------------------
    // 2 DMAs for kempston both write kemppress to PIO, 2 needed to make continuous
    // ---------------------------------------------------------------------------------------
    int data_k_chan=dma_claim_unused_channel(true);
    int data_k2_chan=dma_claim_unused_channel(true);
    // DMA move the requested memory data to PIO for output *DATA*
    dma_channel_config data_k_dma=dma_channel_get_default_config(data_k_chan); // default configuration structure
    channel_config_set_high_priority(&data_k_dma,true); // all high priority channels are considered first
    channel_config_set_dreq(&data_k_dma,pio_get_dreq(pio,joy_k_data_sm,true)); // transfer data to PIO SM
    channel_config_set_read_increment(&data_k_dma,false); // read from same address    
    channel_config_set_transfer_data_size(&data_k_dma,DMA_SIZE_8); // transfer 8bits at a time    
    channel_config_set_chain_to(&data_k_dma,data_k2_chan); // trigger data_k2_chan when complete
    dma_channel_configure(data_k_chan,                    // dma channel
                            &data_k_dma,                  // pointer to DMA config structure
                            &pio->txf[joy_k_data_sm],     // write to -> PIO FIFO
                            &kemppress,                    // read from kemppress
                            8,                          // 1 transfer ** set to 8 as some weird bug in DMA that won't chain
                            true);                     // start
    //
    dma_channel_config data_k2_dma=dma_channel_get_default_config(data_k2_chan); // default configuration structure
    channel_config_set_high_priority(&data_k2_dma,true); // all high priority channels are considered first
    channel_config_set_dreq(&data_k2_dma,pio_get_dreq(pio,joy_k_data_sm,true)); // transfer data to PIO SM
    channel_config_set_transfer_data_size(&data_k2_dma,DMA_SIZE_8); // transfer 8bits at a time 
    channel_config_set_read_increment(&data_k2_dma,false); // read from same address     
    channel_config_set_chain_to(&data_k2_dma,data_k_chan); // trigger data_k_chan when complete
    dma_channel_configure(data_k2_chan,                    // dma channel
                            &data_k2_dma,                  // pointer to DMA config structure
                            &pio->txf[joy_k_data_sm],     // write to -> PIO FIFO
                            &kemppress,                    //  read from kemppress
                            8,                          // 1 transfer
                            false);                     // do not start transfer, triggered by data_k_dma
    //
    pio_sm_set_enabled(pio,joy_k_data_sm,true); // enable state machine 3
    //
    // ---------------------------------------------------------------------------------------
    // initialise OLED
    // ---------------------------------------------------------------------------------------
    i2c_init(i2c1,400000); // 400KHz on i2c1
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C); // sda i2c1
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C); // scl i2c1
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
    oled_t oledDisp;
    oledInit(&oledDisp,128,64,0x3C,i2c1,false);
    //
    // ---------------------------------------------------------------------------------------
    // decompress splashscreen & show for SET_SPLASH secs
    // ---------------------------------------------------------------------------------------
    dtoBuffer(oledDisp.b,scrimg[0]);  
    showOLED(&oledDisp,0,0,0);      
    sleep_ms(SET_SPLASH);      
    //clrFlash(&oledDisp,oledDisp.b); // used if need to reset flash on 1st boot
    //
    // --------------------------------------------------------------------------------------- 
    // check flash & set-up keys etc...
    // ---------------------------------------------------------------------------------------
    uint8_t crc=0;
    for(uint i=0;i<10;i++) if(config_d[i]!=flash_target_contents[i]) crc++;
    //
    // if header doesn't match copy default config into flash
    if(crc>0) {  
        clrFlash(&oledDisp,oledDisp.b);
    } 
    //
    // if config is already in flash copy into config
    else {
        for(uint i=0;i<256;i++) config[i]=flash_target_contents[i];
    }
    //
    // reset all keys etc... using flash config
    resetAll();
    //
    // ---------------------------------------------------------------------------------------
    // main variables
    // ---------------------------------------------------------------------------------------
    uint8_t keyUsed[7]; // max 7 joystick buttons -> Right, Left, Down, Up, B1, B2 & B3 (may expand this)
    uint8_t dataset[128][8]; // lookup array used to set dataout based on joypress (7 joystick positions -> 7bits or 128bytes, 8 address locations)
    gpio_set_dir(PIN_A5RD,false); // switch A5RD back to input              
    //
    // ---------------------------------------------------------------------------------------
    // main loop     
    // ---------------------------------------------------------------------------------------
    while(true) {
        memset(oledDisp.b,0x00,oledDisp.s); // clear screen buffer
        showOLED(&oledDisp,0,0,0);     
        runOLEDcmd(&oledDisp,SET_DISP|0x00); // off   
        //
        // ---------------------------------------------------------------------------------------
        // build lookup tables, do this each loop as menu may change keys
        //
        // keyboard: IN 0xfe Reads keys (bit 0 to bit 4 inclusive)
        // monitor A0 to determine if 0xfe accessed
        //   0xfefe  SHIFT, Z, X, C, V            0xeffe  0, 9, 8, 7, 6
        //   0xfdfe  A, S, D, F, G                0xdffe  P, O, I, U, Y
        //   0xfbfe  Q, W, E, R, T                0xbffe  ENTER, L, K, J, H
        //   0xf7fe  1, 2, 3, 4, 5                0x7ffe  SPACE, SYM SHFT, M, N, B
        // A8-A15 set to:
        //   254 (11111110) - SHIFT, Z, X, C, V
        //   253 (11111101) - A, S, D, F, G
        //   251 (11111011) - Q, W, E, R, T
        //   247 (11110111) - 1, 2, 3, 4, 5
        //   239 (11101111) - 0, 9, 8, 7, 6
        //   223 (11011111) - P, O, I, U, Y
        //   191 (10111111) - ENTER, L, K, J, H
        //   127 (01111111) - SPACE, SYM SHFT, M, N, B        
        // ---------------------------------------------------------------------------------------
        uint8_t *joySel;
        if(scrSwap[swp_ky]==1) joySel=keySel; // keyboard
        else if(scrSwap[swp_cr]==1) joySel=curSel; // cursor
        else if(scrSwap[swp_s2]==1) joySel=s2Sel; // sinclair 2
        else if(scrSwap[swp_s1]==1) joySel=s1Sel; // sinclair 1
        else joySel=kmpSel;
        //
        keyUsed[0]=joySel[3]-4; // right
        keyUsed[1]=joySel[2]-4; // left
        keyUsed[2]=joySel[1]-4; // down
        keyUsed[3]=joySel[0]-4; // up
        keyUsed[4]=joySel[4]-4; // b1
        //
        // b2 (0-b3,1-b2,2-b1,3-up)
        if(joySel[5]==0) { // if b2 mapped to b3 then...
            if(joySel[6]==0) keyUsed[5]==40; // if b3 mapped to b3 then map to nothing
            else if(joySel[6]==1) keyUsed[5]=40; // if b3 mapped to b2 then map to nothing
            else if(joySel[6]==2) keyUsed[5]=keyUsed[4]; // map to b1
            else if(joySel[6]==3) keyUsed[5]=keyUsed[3]; // map to up
            else keyUsed[5]=joySel[6]-4; // b3 key
        }
        else if(joySel[5]==1) keyUsed[5]=40; // if b2 mapped to b2 then map to nothing
        else if(joySel[5]==2) keyUsed[5]=keyUsed[4]; // map to b1
        else if(joySel[5]==3) keyUsed[5]=keyUsed[3]; // map to up
        else keyUsed[5]=joySel[5]-4;
        //
        // b3 (0-b3,1-b2,2-b1,3-up)
        if(joySel[6]==0) keyUsed[6]==40; // if b3 mapped to b3 then map to nothing
        else if(joySel[6]==1) { // if b3 mapped to b2 then...
            if(joySel[5]==0) keyUsed[6]==40; // if b2 mapped to b3 then map to nothing
            else if(joySel[5]==1) keyUsed[6]=40; // if b2 mapped to b2 then map to nothing
            else if(joySel[5]==2) keyUsed[6]=keyUsed[4]; // map to b1
            else if(joySel[5]==3) keyUsed[6]=keyUsed[3]; // map to up
            else keyUsed[6]=joySel[5]-4; // b2 key
        }
        else if(joySel[6]==2) keyUsed[6]=keyUsed[4]; // map to b1
        else if(joySel[6]==3) keyUsed[6]=keyUsed[3]; // map to up
        else keyUsed[6]=joySel[6]-4;
        //
        // key lookup
        for(uint i=0;i<256;i++) dataout[i]=0b11111111; // reset all of the dataout array
        kemppress=0b00000000; // kempston is invert of keyboard, not really required as overwritten in loop
        //
        for(uint i=0;i<128;i++) {
            for(uint j=0;j<8;j++) dataset[i][j]=0b11111111; // reset dataset for given i, then below overides specific bits
            if((i&0b0000001)==0) dataset[i][keyRow[keyUsed[0]]]&=keyCol[keyUsed[0]]; // right pressed
            if((i&0b0000010)==0) dataset[i][keyRow[keyUsed[1]]]&=keyCol[keyUsed[1]]; // left pressed
            if((i&0b0000100)==0) dataset[i][keyRow[keyUsed[2]]]&=keyCol[keyUsed[2]]; // down pressed
            if((i&0b0001000)==0) dataset[i][keyRow[keyUsed[3]]]&=keyCol[keyUsed[3]]; // up pressed
            if((i&0b0010000)==0) dataset[i][keyRow[keyUsed[4]]]&=keyCol[keyUsed[4]]; // b1 pressed
            if((i&0b0100000)==0) dataset[i][keyRow[keyUsed[5]]]&=keyCol[keyUsed[5]]; // b2 pressed
            if((i&0b1000000)==0) dataset[i][keyRow[keyUsed[6]]]&=keyCol[keyUsed[6]]; // b3 pressed
        }
        //
        // ---------------------------------------------------------------------------------------
        // check for kempston
        // ---------------------------------------------------------------------------------------
        uint8_t kempMask=0b01111111;
        if(config[cfg_cjy]==swp_kp) {
            gpio_set_dir(PIN_KEMP,false); // switch kempston back to input
            gpio_set_dir(PIN_A5RD,false); // switch A5RD back to input            
            if(config[cfg_kp2]>=4) {
                gpio_set_dir(PIN_KEYRD,false); // if b2 for kempston is a key then turn on key read
                kempMask&=0b01011111;
            } else {
                
            }
            if(config[cfg_kp3]>=4) {
                gpio_set_dir(PIN_KEYRD,false); // if b2 for kempston is a key then turn on key read
                kempMask&=0b00111111;
            }            
        }
        else {
            gpio_set_dir(PIN_KEYRD,false); // switch key read back to input
        }
        //
        // ---------------------------------------------------------------------------------------
        // key read loop, populate dataout as keys are pressed rather than just read on keyrd
        // this routine can be replaced by the bluetooth joystick read, or run both
        // ---------------------------------------------------------------------------------------       
        do {
            dataout[254]=dataset[joypress][0];
            dataout[253]=dataset[joypress][1];
            dataout[251]=dataset[joypress][2];
            dataout[247]=dataset[joypress][3];
            dataout[239]=dataset[joypress][4];
            dataout[223]=dataset[joypress][5];
            dataout[191]=dataset[joypress][6];
            dataout[127]=dataset[joypress][7];
            kemppress=(joypress^0b1111111)&kempMask; // kempston just an invert of the joystick
        } while(gpio_get(PIN_MENU)==1); // stop on menu button
        //
        // ---------------------------------------------------------------------------------------
        // this basically turns off the PIO
        // ---------------------------------------------------------------------------------------
        gpio_set_dir(PIN_KEYRD,true); // switch key read to out
        gpio_put(PIN_KEYRD,1); // force off
        gpio_set_dir(PIN_KEMP,true); // switch kempston to out
        gpio_put(PIN_KEMP,1); // force off
        gpio_set_dir(PIN_A5RD,true); // switch A5RD to out
        gpio_put(PIN_KEMP,1); // force off        
        //
        // ---------------------------------------------------------------------------------------
        // launch menu
        // ---------------------------------------------------------------------------------------
        menu(&oledDisp,oledDisp.b);   
    } // loop back to top
}
//
// ---------------------------------------------------------------------------
// clrFlash - option to reset the config and write to flash clearing any
//            stored settings - reset to default
// input:
//   pOLED - OLED structure
//   to - OLED screen buffer
// ---------------------------------------------------------------------------
void clrFlash(oled_t *pOLED, uint8_t *to) {
    dtoBuffer(to,scr_setup);  
    showOLED(pOLED,0,0,0);    
    for(uint i=0;i<256;i++) {
        if(i<cfg_end) config[i]=config_d[i];
        else config[i]=0x00;
    }
    writeConfig();
    for(uint i=0;i<10;i++) {
        dtoBuffer(to,scr_setup90);  
        showOLED(pOLED,0,0,0);             
        sleep_ms(100); 
        dtoBuffer(to,scr_setup);  
        showOLED(pOLED,0,0,0);             
        sleep_ms(100); 
    }
}
//
// ---------------------------------------------------------------------------
// resetAll - set-up the key mappings & button 2/3 mappings from config
// ---------------------------------------------------------------------------
void resetAll() {
    // restore key mappings
    for(uint i=0;i<7;i++) keySel[i]=config[i+15];
    // restore b2 & b3 mappings
    kmpSel[5]=config[cfg_kp2];
    kmpSel[6]=config[cfg_kp3];
    s1Sel[5]=config[cfg_s12];    
    s1Sel[6]=config[cfg_s13];
    s2Sel[5]=config[cfg_s22];       
    s2Sel[6]=config[cfg_s23];       
    curSel[5]=config[cfg_cr2];    
    curSel[6]=config[cfg_cr3]; 
    // reset which joystick selected       
    scrSwap[swp_kp]=scrSwap[swp_s1]=scrSwap[swp_s2]=scrSwap[swp_cr]=scrSwap[swp_ky]=0;
    scrSwap[config[cfg_cjy]]=1;
}
//
// ---------------------------------------------------------------------------
// menu - menu to change key mappings, activate joystick etc...
// input:
//   pOLED - OLED structure
//   to - OLED screen buffer
// ---------------------------------------------------------------------------
void menu(oled_t *pOLED, uint8_t *to) {
    //
    // wait for button release
    while((gpio_get_all()&MAP_MBLR)!=MAP_MBLR) {
        tight_loop_contents();
    }       
    runOLEDcmd(pOLED,SET_DISP|0x01); // turn OLED on
    uint8_t currentScr=1;
    uint8_t prevScr=0;
    uint8_t scrBack[5];
    uint8_t level=0;
    uint8_t swap;
    for(uint i=0;i<5;i++) scrBack[i]=0; // allow up to 5 backs (5 levels down)
    uint32_t c,d;
    uint32_t keyRepeat=KEY_REPEAT_LONG;
    uint64_t lastPing;
    uint8_t sel;
    do {
        if(currentScr!=prevScr) { // only update if screen changes
            dtoBuffer(to,scrimg[currentScr+scrSwap[currentScr]]); 
            //
            // ---------------------------------------------------------------------------
            // check if need to add icon
            // ---------------------------------------------------------------------------
            //
            // kempston b2/3
            if(currentScr==b2_kp) {
                pltIcon(icons[kmpSel[5]],to);
            } 
            else if(currentScr==b3_kp) {
                pltIcon(icons[kmpSel[6]],to);
            }
            //
            // sinclair1 b2/3
            else if(currentScr==b2_s1) {
                pltIcon(icons[s1Sel[5]],to);
            }
            else if(currentScr==b3_s1) {
                pltIcon(icons[s1Sel[6]],to);
            }  
            //          
            // sinclair2 b2/3
            else if(currentScr==b2_s2) {
                pltIcon(icons[s2Sel[5]],to);
            }   
            else if(currentScr==b3_s2) {
                pltIcon(icons[s2Sel[6]],to);
            }  
            //            
            // cursor b2/3
            else if(currentScr==b2_cr) {
                pltIcon(icons[curSel[5]],to);
            }
            else if(currentScr==b3_cr) {
                pltIcon(icons[curSel[6]],to);
            }  
            //          
            // all keys
            else if(currentScr>=up_ky&&currentScr<=b3_ky) {
                pltIcon(icons[keySel[currentScr-up_ky]],to);
            }
            showOLED(pOLED,0,0,0); // show screen 
            prevScr=currentScr; // update previous screen
        }
        //
        // ---------------------------------------------------------------------------
        // check for button press
        // ---------------------------------------------------------------------------
        c=gpio_get_all()&MAP_MBLR;
        if(c==d) { // key repeat?
            lastPing=time_us_64();
            do {
                busy_wait_ms(100);                
                c=gpio_get_all()&MAP_MBLR;    
            } while(c==d&&((time_us_64()-lastPing)<keyRepeat));
            if(c==d) keyRepeat=KEY_REPEAT_SHORT;
            else {
                keyRepeat=KEY_REPEAT_LONG;
                d=c;
            }
        } 
        else {
            keyRepeat=KEY_REPEAT_LONG;
            d=c;
        }
        if((c&MAP_RIGHT)==0) { // move to next screen if allowed
            if(scrNext[currentScr]>0) {
                currentScr=scrNext[currentScr];
            } 
        }
        else if((c&MAP_LEFT)==0) { // move to previous screen if allowed
            if(scrPrev[currentScr]>0) {
                currentScr=scrPrev[currentScr];
            } 
        }
        //
        // ---------------------------------------------------------------------------
        // enter pressed, multiple options based on screen
        // 131 - reset flash config
        // 130 - activate
        // 129 - show key selector with b3,b2,b1 & up additional options
        // 128 - show key selector without additional
        // ---------------------------------------------------------------------------        
        else if((c&MAP_B1)==0) { 
            if(scrEnter[currentScr]>0) {
                if(scrEnter[currentScr]>=128) {
                    if(scrEnter[currentScr]==130) { // activate
                        scrSwap[currentScr]=1; // reset activate
                        dtoBuffer(to,scrimg[swapto]); 
                        showOLED(pOLED,0,0,0);
                        config[cfg_cjy]=currentScr; // then set current
                        writeConfig();
                        busy_wait_ms(1000);
                        currentScr=0;
                    }
                    else if(scrEnter[currentScr]==128) { // key selector without additional
                        sel=getKey(pOLED,to,4);   
                        if(sel!=99) { 
                            config[cfg_key+(currentScr-up_ky)]=keySel[currentScr-up_ky]=sel;
                            writeConfig();
                        }
                    }
                    else if(scrEnter[currentScr]==129) { // key selector with b3,b2,b1 & up additional options
                        sel=getKey(pOLED,to,0);     
                        if(sel!=99) { 
                            if(currentScr==b2_s1) { // sinclair1
                                config[cfg_s12]=s1Sel[5]=sel;
                            }
                            else if(currentScr==b3_s1) { // sinclair1
                                config[cfg_s13]=s1Sel[6]=sel;
                            }
                            else if(currentScr==b2_s2) { // sinclair2
                                config[cfg_s22]=s2Sel[5]=sel;
                            }
                            else if(currentScr==b3_s2) { // sinclair2
                                config[cfg_s23]=s2Sel[6]=sel;
                            }                            
                            else if(currentScr==b2_cr) { // cursor
                                config[cfg_cr2]=curSel[5]=sel;
                            }  
                            else if(currentScr==b3_cr) { // cursor
                                config[cfg_cr3]=curSel[6]=sel;
                            }                                                   
                            else if(currentScr==b2_ky) { // keyboard
                                config[cfg_ky2]=keySel[5]=sel;
                            }
                            else if(currentScr==b3_ky) { // keyboard
                                config[cfg_ky3]=keySel[6]=sel;
                            }                            
                            else if(currentScr==b2_kp) { // kempston                
                                config[cfg_kp2]=kmpSel[5]=sel;
                            }
                            else if(currentScr==b3_kp) { // kempston                          
                                config[cfg_kp3]=kmpSel[6]=sel;
                            }                            
                            writeConfig();
                        }
                    }
                    else if(scrEnter[currentScr]==131) {
                        clrFlash(pOLED,to);
                        resetAll(); 
                    }
                    prevScr=0; // force screen re-draw
                } 
                else { // back button
                    level++;
                    scrBack[level]=currentScr;
                    currentScr=scrEnter[currentScr]; 
                }
            } 
        }
        else if((c&MAP_MENU)==0) {
            if(level==0) currentScr=0;
            else currentScr=scrBack[level--];
        }
    } while(currentScr>0); // if=0 then exit menu
    while((gpio_get_all()&MAP_MBLR)!=MAP_MBLR) { // wait for keyup
        tight_loop_contents(); 
    }       
}
//
// ---------------------------------------------------------------------------
// writeConfig - erase flash area and write the config
// ---------------------------------------------------------------------------
void writeConfig() {
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, config, FLASH_PAGE_SIZE);
}
//
// ---------------------------------------------------------------------------
// getKey - key selector from icons, uses U D L R & B1 to select which key
// input:
//   pOLED - OLED structure
//   to - OLED screen buffer
//   iconShift - used to show or not show additional icons
// return:
//   x - which icon/key selected
//   99 - exit/back
// ---------------------------------------------------------------------------
uint8_t getKey(oled_t *pOLED, uint8_t *to,uint8_t iconShift) {
    uint32_t c,d;
    uint32_t keyRepeat=KEY_REPEAT_LONG;
    uint64_t lastPing;    
    uint8_t iconPos=0;
    uint8_t iconStart=iconShift;
    pltAllIcons(to,iconStart,iconPos); // show icon selector
    showOLED(pOLED,0,0,0);
    while((gpio_get_all()&MAP_MBUDLR)!=MAP_MBUDLR) { // wait for keyup
        tight_loop_contents();
    }
    while(true) { // loop forever
        c=gpio_get_all()&MAP_MBUDLR;
        if(c==d) { // key repeat?
            lastPing=time_us_64();
            do {
                busy_wait_ms(100);                
                c=gpio_get_all()&MAP_MBUDLR;    
            } while(c==d&&((time_us_64()-lastPing)<keyRepeat));
            if(c==d) keyRepeat=KEY_REPEAT_SHORT;
            else {
                keyRepeat=KEY_REPEAT_LONG;
                d=c;
            }
        } 
        else {
            keyRepeat=KEY_REPEAT_LONG;
            d=c;
        }
        if((c&MAP_RIGHT)==0) { // next icon inc. wrap
            iconPos++;
            if(iconStart<32) {
                if(iconPos==12) {
                    iconPos-=4;
                    iconStart+=4;
                }
            } else if(iconPos==12) {
                iconPos=0;
                iconStart=iconShift;
            }         
        }
        else if((c&MAP_LEFT)==0) { // previous icon inc. wrap
            if(iconPos==0&iconStart==iconShift) {
                iconPos=11;
                iconStart=32;
            } else if(iconPos==0) {
                iconPos+=3;
                iconStart-=4;
            } else {
                iconPos--;
            }   
        }
        else if((c&MAP_UP)==0) { // up 1 icon row inc. wrap
            if(iconPos>3) {
                iconPos-=4;
            } else if(iconStart>(3+iconShift)) {
                iconStart-=4;
            } else {
                iconStart=32;
                iconPos+=8;
            }
        }
        else if((c&MAP_DOWN)==0) { // down 1 icon row inc. wrap
            if(iconPos<8) {
                iconPos+=4;
            } else if(iconStart<32) {
                iconStart+=4;
            } else {
                iconStart=iconShift;
                iconPos-=8;
            }
        }
        else if((c&MAP_B1)==0) { // enter pressed return which icon (key) selected
            while((gpio_get_all()&MAP_MBUDLR)!=MAP_MBUDLR) {
                tight_loop_contents();
            }            
            return iconPos+iconStart;            
        }
        else if((c&MAP_MENU)==0) { // back pressed
            while((gpio_get_all()&MAP_MBUDLR)!=MAP_MBUDLR) {
                tight_loop_contents();
            }            
            return 99;
        }        
        if(c!=MAP_MBUDLR) { // if something pressed update screen
            pltAllIcons(to,iconStart,iconPos);
            showOLED(pOLED,0,0,0); 
        }
    }
}
//
// ---------------------------------------------------------------------------
// pltAllIcons - plot 4x3 icons to screen, uses pltIcons
// input:
//   to - OLED screen buffer
//   iconStart - which icon to start at
//   iconPos - which icon should be inverted (selected)
// ---------------------------------------------------------------------------
void pltAllIcons(uint8_t *to,uint8_t iconStart,uint8_t iconPos) {
    uint8_t row; // 0-2
    uint8_t col; // 0-3
    for(row=0;row<3;row++)
        for(col=0;col<4;col++)
            if(((row*4)+col)==iconPos) pltIcons(row,col,icons[(row*4)+col+iconStart],to,true);
            else pltIcons(row,col,icons[(row*4)+col+iconStart],to,false);
}
//
// ---------------------------------------------------------------------------
// pltIcon - plot single icon and middle bottom of screen, used to show key
//           mapping for joystick direction
// input:
//   icon - which icon to plot
//   to - OLED screen buffer
// ---------------------------------------------------------------------------    
void pltIcon(const uint8_t *icon,uint8_t *to) {
    uint8_t buffer[96];
    dtoBuffer(buffer,icon);
    for(uint i=0;i<32;i++) {
            to[i+640+48]=buffer[i];
            to[i+768+48]=buffer[i+32];
            to[i+896+48]=buffer[i+64];
    }
}
//
// ---------------------------------------------------------------------------
// pltIcons - decompress icon and put into screen at specifc row & column
//            used for key selecor & allows icon to be inverted (selected)
// input:
//   row - which row
//   col - which column
//   icon - which icon to plot
//   to - OLED screen buffer
//   invert - invert or not
// ---------------------------------------------------------------------------
void pltIcons(uint8_t row,uint8_t col,const uint8_t *icon,uint8_t *to,bool invert) {
    uint8_t buffer[96];
    dtoBuffer(buffer,icon);
    //
    // ---------------------------------------------------------------------------
    // need to plot the icon differently dependent on the row as not exact fit
    // ---------------------------------------------------------------------------    
    if(row==0) {
        for(uint i=0;i<32;i++) {
            to[i+(col*32)]=buffer[i];
            to[i+128+(col*32)]=buffer[i+32];
            to[i+256+(col*32)]=buffer[i+64]&0b00111111;
        }
        if(invert) { // show inverted icon, mask bits
            for(uint j=2;j<30;j++) {
                to[j+(col*32)]^=0b11111100;
                to[j+128+(col*32)]^=0b11111111;
                to[j+256+(col*32)]^=0b00001111;
            }
        }
    }
    else if(row==1) {
        for(uint i=0;i<32;i++) {
            to[i+256+(col*32)]|=buffer[i]<<5; // 2bytes from top
            to[i+384+(col*32)]=buffer[i]>>3|buffer[i+32]<<5; // bottom 6bytes
            to[i+512+(col*32)]=buffer[i+32]>>3|buffer[i+64]<<5;
            to[i+640+(col*32)]=(buffer[i+64]>>3)&0b00000111;
        }
        if(invert) {
            for(uint j=2;j<30;j++) {
                to[j+256+(col*32)]^=0b10000000;
                to[j+384+(col*32)]^=0b11111111;
                to[j+512+(col*32)]^=0b11111111;
                to[j+640+(col*32)]^=0b00000001;        
            }
        }        
    }
    else {
        for(uint i=0;i<32;i++) {
            to[i+640+(col*32)]|=buffer[i]<<2; // 2bytes from top
            to[i+768+(col*32)]=buffer[i]>>6|buffer[i+32]<<2; // bottom 6bytes
            to[i+896+(col*32)]=buffer[i+32]>>6|buffer[i+64]<<2;
        }
        if(invert) {
            for(uint j=2;j<30;j++) {
                to[j+640+(col*32)]^=0b11110000;
                to[j+768+(col*32)]^=0b11111111;
                to[j+896+(col*32)]^=0b00111111;
            }
        }
    }
}
//
// ---------------------------------------------------------------------------
// oledInit - initialise OLED screen & set-up structure 
//            only works with 128x32 screens
// input:
//   pOLED - pointer to the OLED structure
//   w - screen width (128)
//   h - screen height (64)
//   a - screen address
//   i - i2c instance
//   e - external power true/false (false)
// *based on Python version https://github.com/makerportal/rpi-pico-ssd1306
// ---------------------------------------------------------------------------
void oledInit(oled_t *pOLED,uint8_t w,uint8_t h,uint8_t a,i2c_inst_t *i,bool e) {
    //
    // ---------------------------------------------------------------------------
    // copy inputs to the structure
    // ---------------------------------------------------------------------------
    pOLED->w=w;
    pOLED->h=h;
    pOLED->p=h/8;
    pOLED->a=a;
    pOLED->i=i;
    pOLED->e=e;
    pOLED->s=(pOLED->p)*(pOLED->w); // buffer size p*w (1024bytes)
    pOLED->c=&mem_buffer[MEMS_OLED];
    memset(pOLED->c,0x00,pOLED->s+1);
    pOLED->b=pOLED->c;
    *pOLED->b++=0x40; // Co=0, D/C#=1
    runOLEDcmd(pOLED,SET_DISP|0x00); // turn display off 
    runOLEDcmd(pOLED,SET_MEM_ADDR); // address setting
    runOLEDcmd(pOLED,0x00); // horizontal
    runOLEDcmd(pOLED,SET_DISP_START_LINE|0x00); // resolution and layout
    runOLEDcmd(pOLED,SET_SEG_REMAP|0x01); // column addr 127 mapped to SEG0
    runOLEDcmd(pOLED,SET_MUX_RATIO);
    runOLEDcmd(pOLED,h-1);
    runOLEDcmd(pOLED,SET_COM_OUT_DIR|0x08); // scan from COM[N] to COM0
    runOLEDcmd(pOLED,SET_DISP_OFFSET);
    runOLEDcmd(pOLED,0x00);
    runOLEDcmd(pOLED,SET_COM_PIN_CFG);
    runOLEDcmd(pOLED,w>2*h?0x02:0x12);
    runOLEDcmd(pOLED,SET_DISP_CLK_DIV); // timing and driving scheme
    runOLEDcmd(pOLED,0x80);
    runOLEDcmd(pOLED,SET_PRECHARGE);
    runOLEDcmd(pOLED,pOLED->e?0x22:0xF1);
    runOLEDcmd(pOLED,SET_VCOM_DESEL);
    runOLEDcmd(pOLED,0x30); // 0.83*Vcc
    runOLEDcmd(pOLED,SET_CONTRAST); // display
    runOLEDcmd(pOLED,0x7F); // half
    runOLEDcmd(pOLED,SET_ENTIRE_ON); // output follows RAM contents
    runOLEDcmd(pOLED,SET_NORM_INV); // not inverted
    runOLEDcmd(pOLED,SET_CHARGE_PUMP); // charge pump
    runOLEDcmd(pOLED,pOLED->e?0x10:0x14);
    runOLEDcmd(pOLED,SET_DISP|0x01); // turn display on
    showOLED(pOLED,0,0,0); // clear screen
}
//
// ---------------------------------------------------------------------------
// showOLED - send commands to display the buffer on the OLED screen
//            column 0-128 + page 0,1,2,3,4,5,6,7 
//            (4ms for one page, 26ms for all)
// input:
//   pOLED - pointer to the OLED structure
//   c - column offset (0-128)
//   p - page offset (0-7)
//   s - size to show in bytes (0=all of the buffer) 
// ---------------------------------------------------------------------------
void showOLED(oled_t *pOLED,uint8_t c,uint8_t p,size_t s) {
    if(s==0) s=pOLED->s+1;
    runOLEDcmd(pOLED,SET_COL_ADDR);
    runOLEDcmd(pOLED,c);
    runOLEDcmd(pOLED,pOLED->w-1-c);
    runOLEDcmd(pOLED,SET_PAGE_ADDR);
    runOLEDcmd(pOLED,p);
    runOLEDcmd(pOLED,pOLED->p-1-p);
    i2c_write_blocking(pOLED->i,pOLED->a,pOLED->c,s,false);
}
//
// ---------------------------------------------------------------------------
// runOLEDcmd - send command to OLED (based on Python version)
// input:
//   pOLED - pointer to OLED structure
//   v - the command
// ---------------------------------------------------------------------------
void runOLEDcmd(oled_t *pOLED,uint8_t v) {
    uint8_t s[2]={0x80,v}; // Co=1, D/C#=0
    i2c_write_blocking(pOLED->i,pOLED->a,s,2,false);
}
//
// ---------------------------------------------------------------------------
// dtoBuffer - decompress compressed screen directly into buffer (simple LZ)
// input:
//   to - the buffer
//   from - the compressed storage
// *simple LZ has a simple 256 backwards window and greedy parser but is very
// fast
// ---------------------------------------------------------------------------
void dtoBuffer(uint8_t *to,const uint8_t *from) { 
    uint i=0,j=0,k;
    uint8_t c,o;
    do {
        c=from[j++];
        if(c==128) return;
        else if(c<128) {
            for(k=0;k<c+1;k++) to[i++]=from[j++];
        }
        else {
            o=from[j++]; // offset
            for(k=0;k<(c-126);k++) {
                to[i]=to[i-(o+1)];
                i++;
            }
        }
    } while(true);
}
