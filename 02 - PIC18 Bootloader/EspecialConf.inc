; PIC18F27J13 Configuration Bit Settings

; ASM source line config statements

;#include "p18F27J13.inc"

; CONFIG1L
  CONFIG  WDTEN = OFF           ; Watchdog Timer (Disabled - Controlled by SWDTEN bit)
  CONFIG  PLLDIV = 5            ; 96MHz PLL Prescaler Selection (PLLSEL=0) (Divide by 5 (20 MHz oscillator input))
  CONFIG  CFGPLLEN = ON         ; PLL Enable Configuration Bit (PLL Enabled)
  CONFIG  STVREN = OFF          ; Stack Overflow/Underflow Reset (Disabled)
  CONFIG  XINST = OFF           ; Extended Instruction Set (Disabled)

; CONFIG1H
  CONFIG  CP0 = OFF             ; Code Protect (Program memory is not code-protected)

; CONFIG2L
  CONFIG  OSC = HSPLL           ; Oscillator (HS+PLL)
  CONFIG  SOSCSEL = DIG         ; T1OSC/SOSC Power Selection Bits (Digital (SCLKI) mode selected)
  CONFIG  CLKOEC = OFF          ; EC Clock Out Enable Bit  (CLKO output disabled on the RA6 pin)
  CONFIG  FCMEN = OFF           ; Fail-Safe Clock Monitor (Disabled)
  CONFIG  IESO = ON             ; Internal External Oscillator Switch Over Mode (Enabled)

; CONFIG2H
  CONFIG  WDTPS = 4             ; Watchdog Postscaler (1:4)

; CONFIG3L
  CONFIG  DSWDTOSC = INTOSCREF  ; DSWDT Clock Select (DSWDT uses INTRC)
  CONFIG  RTCOSC = INTOSCREF    ; RTCC Clock Select (RTCC uses INTRC)
  CONFIG  DSBOREN = OFF         ; Deep Sleep BOR (Disabled)
  CONFIG  DSWDTEN = OFF         ; Deep Sleep Watchdog Timer (Disabled)
  CONFIG  DSWDTPS = 2           ; Deep Sleep Watchdog Postscaler (1:2 (2.1 ms))

; CONFIG3H
  CONFIG  IOL1WAY = ON          ; IOLOCK One-Way Set Enable bit (The IOLOCK bit (PPSCON<0>) can be set once)
  CONFIG  ADCSEL = BIT12        ; ADC 10 or 12 Bit Select (12 - Bit ADC Enabled)
  CONFIG  PLLSEL = PLL96        ; PLL Selection Bit (Selects 96MHz PLL)
  CONFIG  MSSP7B_EN = MSK5      ; MSSP address masking (5 Bit address masking mode)

; CONFIG4L
  CONFIG  WPFP = PAGE_127       ; Write/Erase Protect Page Start/End Location (Write Protect Program Flash Page 127)
  CONFIG  WPCFG = OFF           ; Write/Erase Protect Configuration Region  (Configuration Words page not erase/write-protected)

; CONFIG4H
  CONFIG  WPDIS = OFF           ; Write Protect Disable bit (WPFP<6:0>/WPEND region ignored)
  CONFIG  WPEND = PAGE_WPFP     ; Write/Erase Protect Region Select bit (valid when WPDIS = 0) (Pages WPFP<6:0> through Configuration Words erase/write protected)


