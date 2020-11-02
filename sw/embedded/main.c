//---------------------------------------------------------------------------------------------
// INCLUDES
//

#include <SI_EFM8UB1_Register_Enums.h>                  // SFR declarations
#include "InitDevice.h"
#include "efm8_usb.h"


//---------------------------------------------------------------------------------------------
// DEFINES
//

#define SYSCLK       48000000      // SYSCLK frequency in Hz

SI_SBIT(LED,       SFR_P0, 3);
SI_SBIT(CS2,       SFR_P0, 7);
SI_SBIT(CS1,       SFR_P1, 0);
SI_SBIT(SDA,       SFR_P1, 1);
SI_SBIT(SCL,       SFR_P1, 2);


//---------------------------------------------------------------------------------------------
// TYPEDEFS
//

typedef uint16_t GAMMA_TABLE[256];


//---------------------------------------------------------------------------------------------
// PROTOTYPES
//

void Timer2_Init (int counts);
void SMB_Write(void);
void SMB_Read(void);
void DacWrite (uint8_t sel, uint8_t *dat);

//void SetPWM (uint8_t address, uint8_t channel, uint16_t v);
//void SetPWMInv (uint8_t address, uint8_t channel, uint16_t v);
//void SetPWMRaw (uint8_t address, uint8_t channel, uint16_t on, uint16_t off);
//void PCA9685Init (uint8_t address);


//---------------------------------------------------------------------------------------------
// GLOBALS
//

// in=[0:255]; round((in/255).^2.8*4095)
SI_SEGMENT_VARIABLE(led_gamma_12b_2p8, const GAMMA_TABLE, SI_SEG_CODE) = {
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     1,     1,     1,     1,     1,
       2,     2,     2,     3,     3,     4,     4,     5,     5,     6,     7,     8,     8,     9,    10,    11,
      12,    13,    15,    16,    17,    18,    20,    21,    23,    25,    26,    28,    30,    32,    34,    36,
      38,    40,    43,    45,    48,    50,    53,    56,    59,    62,    65,    68,    71,    75,    78,    82,
      85,    89,    93,    97,   101,   105,   110,   114,   119,   123,   128,   133,   138,   143,   149,   154,
     159,   165,   171,   177,   183,   189,   195,   202,   208,   215,   222,   229,   236,   243,   250,   258,
     266,   273,   281,   290,   298,   306,   315,   324,   332,   341,   351,   360,   369,   379,   389,   399,
     409,   419,   430,   440,   451,   462,   473,   485,   496,   508,   520,   532,   544,   556,   569,   582,
     594,   608,   621,   634,   648,   662,   676,   690,   704,   719,   734,   749,   764,   779,   795,   811,
     827,   843,   859,   876,   893,   910,   927,   944,   962,   980,   998,  1016,  1034,  1053,  1072,  1091,
    1110,  1130,  1150,  1170,  1190,  1210,  1231,  1252,  1273,  1294,  1316,  1338,  1360,  1382,  1404,  1427,
    1450,  1473,  1497,  1520,  1544,  1568,  1593,  1617,  1642,  1667,  1693,  1718,  1744,  1770,  1797,  1823,
    1850,  1877,  1905,  1932,  1960,  1988,  2017,  2045,  2074,  2103,  2133,  2162,  2192,  2223,  2253,  2284,
    2315,  2346,  2378,  2410,  2442,  2474,  2507,  2540,  2573,  2606,  2640,  2674,  2708,  2743,  2778,  2813,
    2849,  2884,  2920,  2957,  2993,  3030,  3067,  3105,  3143,  3181,  3219,  3258,  3297,  3336,  3376,  3416,
    3456,  3496,  3537,  3578,  3619,  3661,  3703,  3745,  3788,  3831,  3874,  3918,  3962,  4006,  4050,  4095
};

volatile uint8_t flag100 = 0;

uint8_t TARGET;
uint8_t NUM_BYTES_WR;
uint8_t NUM_BYTES_RD;
uint8_t SMB_DATA_IN[8];
uint8_t SMB_DATA_OUT[8];
volatile bool SMB_BUSY;
volatile bool SMB_RW;
uint16_t NUM_ERRORS;                   // Counter for the number of errors.
uint8_t dacData[2];

extern volatile uint8_t irqMetersUpdateFlags;
extern volatile uint8_t irqMetersLevels[4];
uint8_t mainMetersUpdateFlags;
uint8_t mainMetersLevels[4];


//---------------------------------------------------------------------------------------------
// CODE
//

void SiLabs_Startup (void)
{
}


int main (void)
{
	uint8_t SFRPAGE_save;
	uint8_t TCON_save;
	uint8_t TMR3CN0_TR3_save;
	uint8_t i;
	uint8_t led_timer = 0;
    uint8_t newUsbState = 0;
    uint8_t oldUsbState = 0;

	uint16_t a = 0, b;
	uint8_t dac1A, dac1B, dac2A, dac2B;

	enter_DefaultMode_from_RESET();

    // If slave is holding SDA low because of an improper SMBus reset or error
    while(!SDA)
    {
	    // Provide clock pulses to allow the slave to advance out
	    // of its current state. This will allow it to release SDA.
	    XBR2 = XBR2_XBARE__ENABLED;      // Enable Crossbar
	    SCL = 0;                         // Drive the clock low
	    for(i = 0; i < 255; i++);        // Hold the clock low
	    SCL = 1;                         // Release the clock
	    while(!SCL);                     // Wait for open-drain
									   // clock output to rise
	    for(i = 0; i < 10; i++);         // Hold the clock high
	    XBR2 = XBR2_XBARE__DISABLED;     // Disable Crossbar
    }

    // finish enabling IIC interface on P0.5 and P0.6

	SFRPAGE_save = SFRPAGE;

	XBR0 = XBR0_URT0E__DISABLED | XBR0_SPI0E__ENABLED | XBR0_SMB0E__ENABLED
			| XBR0_CP0E__DISABLED | XBR0_CP0AE__DISABLED | XBR0_CP1E__DISABLED
			| XBR0_CP1AE__DISABLED | XBR0_SYSCKE__DISABLED;

	TCON_save = TCON;
	TCON &= ~TCON_TR0__BMASK & ~TCON_TR1__BMASK;
	TH0 = (0x38 << TH0_TH0__SHIFT);
	TL0 = (0x13 << TL0_TL0__SHIFT);
	TH1 = (0xD8 << TH1_TH1__SHIFT);		// 48e6/4/(256-0xD8) = 300kHz => 100kHz IIC clock
	TL1 = (0xD8 << TL1_TL1__SHIFT);
	TCON |= (TCON_save & TCON_TR0__BMASK) | (TCON_save & TCON_TR1__BMASK);

	TMR3CN0_TR3_save = TMR3CN0 & TMR3CN0_TR3__BMASK;
	TMR3CN0 &= ~(TMR3CN0_TR3__BMASK);
	TMR3RLH = (0x38 << TMR3RLH_TMR3RLH__SHIFT);
	TMR3RLL = (0x9E << TMR3RLL_TMR3RLL__SHIFT);
	TMR3CN0 |= TMR3CN0_TR3__RUN;
	TMR3CN0 |= TMR3CN0_TR3_save;

	CKCON0 = CKCON0_SCA__SYSCLK_DIV_4 | CKCON0_T0M__PRESCALE
			| CKCON0_T2MH__EXTERNAL_CLOCK | CKCON0_T2ML__EXTERNAL_CLOCK
			| CKCON0_T3MH__EXTERNAL_CLOCK | CKCON0_T3ML__EXTERNAL_CLOCK
			| CKCON0_T1M__PRESCALE;

	TMOD = TMOD_T0M__MODE0 | TMOD_T1M__MODE2 | TMOD_CT0__TIMER
			| TMOD_GATE0__DISABLED | TMOD_CT1__TIMER | TMOD_GATE1__DISABLED;

	TCON |= TCON_TR1__RUN;


	SMB0CF &= ~SMB0CF_SMBCS__FMASK;
	SMB0CF |= SMB0CF_SMBCS__TIMER1 | SMB0CF_INH__SLAVE_DISABLED
			| SMB0CF_ENSMB__ENABLED | SMB0CF_SMBFTE__FREE_TO_ENABLED
			| SMB0CF_SMBTOE__SCL_TO_ENABLED | SMB0CF_EXTHOLD__ENABLED;

	EIE1 = EIE1_EADC0__DISABLED | EIE1_EWADC0__DISABLED | EIE1_ECP0__DISABLED
			| EIE1_ECP1__DISABLED | EIE1_EMAT__DISABLED | EIE1_EPCA0__DISABLED
			| EIE1_ESMB0__ENABLED | EIE1_ET3__ENABLED;

	SFRPAGE = SFRPAGE_save;

	// enable 100 Hz timer for main loop timing
	Timer2_Init (40000); // SYSCLK / 12 / 100 for 100 Hz

	// initiailze DAC, slow mode, powered up, use 2.048V internal ref voltage
	dacData[0] = 0x90;
	dacData[1] = 0x02;
	DacWrite (0, dacData);

	// write DAC B value to buffer and DAC B
	dacData[0] = 0x00;
	dacData[1] = 0x00;
	DacWrite (0, dacData);

	// write DAC A value and move buffer to DAC B simultaneously
	dacData[0] = 0x80;
	dacData[1] = 0x00;
	DacWrite (0, dacData);

	// initiailze DAC, slow mode, powered up, use 2.048V internal ref voltage
	dacData[0] = 0x90;
	dacData[1] = 0x02;
	DacWrite (1, dacData);

	// write DAC B value to buffer and DAC B
	dacData[0] = 0x00;
	dacData[1] = 0x00;
	DacWrite (1, dacData);

	// write DAC A value and move buffer to DAC B simultaneously
	dacData[0] = 0x80;
	dacData[1] = 0x00;
	DacWrite (1, dacData);

	// init led driver chip
	// PCA9685Init (0x40);

	// turn off all channels
/*
	for (i = 0; i < 16; i++) {
		SetPWMInv (0x40, i, 0);
	}
*/

	while (1) {

		// check if 100 Hz timer expired
		if (flag100) {
			// clear flag
			flag100 = false;

			// new usb state
			newUsbState = USBD_GetUsbState ();

			if (newUsbState == USBD_STATE_CONFIGURED) {
				// blink led
				if (led_timer < 25) {
					LED = 0; // LED ON
				} else if (led_timer < 50) {
					LED = 1; // LED OFF
				} else if (led_timer < 75) {
					LED = 0; // LED ON
				} else if (led_timer < 100) {
					LED = 1; // LED OFF
				}

				// zero lamps on entering configured state
				if (newUsbState != oldUsbState) {
					oldUsbState = newUsbState;

					// write DAC B value to DAC B and buffer
					dacData[0] = 0x00;
					dacData[1] = 0x00;
					DacWrite (0, dacData);

					// write DAC A value and move buffer to DAC B simultaneously
					dacData[0] = 0x80;
					dacData[1] = 0x00;
					DacWrite (0, dacData);

					// write DAC B value to DAC B and buffer
					dacData[0] = 0x00;
					dacData[1] = 0x00;
					DacWrite (1, dacData);

					// write DAC A value and move buffer to DAC B simultaneously
					dacData[0] = 0x80;
					dacData[1] = 0x00;
					DacWrite (1, dacData);
				}

			} else {
				// blink led differently
				if (led_timer < 25) {
					LED = 0; // LED ON
				} else if (led_timer < 50) {
					LED = 1; // LED OFF
				} else if (led_timer < 75) {
					LED = 1; // LED OFF
				} else if (led_timer < 100) {
					LED = 1; // LED OFF
				}

				// do something with the meters if powered up but not configured.

				if (a & 0x100) {
					dac1A = 255 - (a & 0xff);
					dac1B = a & 0xff;
				} else {
					dac1A = a & 0xff;
					dac1B = 255 - (a & 0xff);
				}

				b = a + 128;

				a++;

				if (b & 0x100) {
					dac2A = 255 - (b & 0xff);
					dac2B = b & 0xff;
				} else {
					dac2A = b & 0xff;
					dac2B = 255 - (b & 0xff);
				}

				// write DAC B value to DAC B and buffer
				dacData[0] = 0x00 | ((dac1B >> 4) & 0x0f);
				dacData[1] = 0x00 | ((dac1B << 4) & 0xf0);
				DacWrite (0, dacData);

				// write DAC A value and move buffer to DAC B simultaneously
				dacData[0] = 0x80 | ((dac1A >> 4) & 0x0f);
				dacData[1] = 0x00 | ((dac1A << 4) & 0xf0);
				DacWrite (0, dacData);

				// write DAC B value to DAC B and buffer
				dacData[0] = 0x00 | ((dac2B >> 4) & 0x0f);
				dacData[1] = 0x00 | ((dac2B << 4) & 0xf0);
				DacWrite (1, dacData);

				// write DAC A value and move buffer to DAC B simultaneously
				dacData[0] = 0x80 | ((dac2A >> 4) & 0x0f);
				dacData[1] = 0x00 | ((dac2A << 4) & 0xf0);
				DacWrite (1, dacData);

			}

			// update 1 second led timer
			if (++led_timer == 100) {
				led_timer = 0;
			}
		}

		SFRPAGE_save = SFRPAGE;
		USB_DisableInts ();
		SFRPAGE = SFRPAGE_save;

		mainMetersUpdateFlags = irqMetersUpdateFlags;
		mainMetersLevels[0] = irqMetersLevels[0];
		mainMetersLevels[1] = irqMetersLevels[1];
		mainMetersLevels[2] = irqMetersLevels[2];
		mainMetersLevels[3] = irqMetersLevels[3];
		irqMetersUpdateFlags = 0;

		SFRPAGE_save = SFRPAGE;
		USB_EnableInts ();
		SFRPAGE = SFRPAGE_save;

		if (mainMetersUpdateFlags & 2) {
			// write DAC B value to DAC B and buffer
			dacData[0] = 0x00 | ((mainMetersLevels[1] >> 4) & 0x0f);
			dacData[1] = 0x00 | ((mainMetersLevels[1] << 4) & 0xf0);
			DacWrite (0, dacData);
		}

		if (mainMetersUpdateFlags & 1) {
			// write DAC A value and move buffer to DAC B simultaneously
			dacData[0] = 0x80 | ((mainMetersLevels[0] >> 4) & 0x0f);
			dacData[1] = 0x00 | ((mainMetersLevels[0] << 4) & 0xf0);
			DacWrite (0, dacData);
		}

		if (mainMetersUpdateFlags & 8) {
			// write DAC B value to DAC B and buffer
			dacData[0] = 0x00 | ((mainMetersLevels[3] >> 4) & 0x0f);
			dacData[1] = 0x00 | ((mainMetersLevels[3] << 4) & 0xf0);
			DacWrite (1, dacData);
		}

		if (mainMetersUpdateFlags & 4) {
			// write DAC A value and move buffer to DAC B simultaneously
			dacData[0] = 0x80 | ((mainMetersLevels[2] >> 4) & 0x0f);
			dacData[1] = 0x00 | ((mainMetersLevels[2] << 4) & 0xf0);
			DacWrite (1, dacData);
		}
	}
}


void Timer2_Init (int counts)
{
   TMR2CN0 = 0x00;                     // Stop Timer2; Clear TF2;
                                       // use SYSCLK/12 as timebase
   CKCON0 &= ~0x60;                    // Timer2 clocked based on T2XCLK;

   TMR2RL = -counts;                   // Init reload values
   TMR2 = 0xffff;                      // Set to reload immediately
   IE_ET2 = 1;                         // Enable Timer2 interrupts
   TMR2CN0_TR2 = 1;                    // Start Timer2
}


SI_INTERRUPT(TIMER2_ISR, TIMER2_IRQn)
{
    TMR2CN0_TF2H = 0;                   // Clear Timer2 interrupt flag
	flag100 = true;
}


//-----------------------------------------------------------------------------
// Support Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SMB_Write
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Writes a single byte to the slave with address specified by the <TARGET>
// variable.
// Calling sequence:
// 1) Write target slave address to the <TARGET> variable
// 2) Write outgoing data to the <SMB_DATA_OUT> variable array
// 3) Call SMB_Write()
//
//-----------------------------------------------------------------------------
void SMB_Write(void)
{
   while(SMB_BUSY);                    // Wait for SMBus to be free.
   SMB_BUSY = 1;                       // Claim SMBus (set to busy)
   SMB_RW = 0;                         // Mark this transfer as a WRITE
   SMB0CN0_STA = 1;                    // Start transfer
   while(SMB_BUSY);                    // Wait for SMBus to be free.
}

//-----------------------------------------------------------------------------
// SMB_Read
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Reads a single byte from the slave with address specified by the <TARGET>
// variable.
// Calling sequence:
// 1) Write target slave address to the <TARGET> variable
// 2) Call SMB_Write()
// 3) Read input data from <SMB_DATA_IN> variable array
//
//-----------------------------------------------------------------------------
void SMB_Read(void)
{
   while(SMB_BUSY);                    // Wait for bus to be free.
   SMB_BUSY = 1;                       // Claim SMBus (set to busy)
   SMB_RW = 1;                         // Mark this transfer as a READ
   SMB0CN0_STA = 1;                    // Start transfer
   while(SMB_BUSY);                    // Wait for transfer to complete
}

/*
void SetPWM (uint8_t address, uint8_t channel, uint16_t v)
{
    v &= 0xFFF;
    if (v == 4095) {
        // fully on
        SetPWMRaw (address, channel, 4096, 0);
    } else if (v == 0) {
        // fully off
        SetPWMRaw (address, channel, 0, 4096);
    } else {
        SetPWMRaw (address, channel, 0, v);
    }
}


void SetPWMInv (uint8_t address, uint8_t channel, uint16_t v)
{
    v &= 0xFFF;
    v = 4095 - v;
    if (v == 4095) {
        // fully on
        SetPWMRaw (address, channel, 4096, 0);
    } else if (v == 0) {
        // fully off
        SetPWMRaw (address, channel, 0, 4096);
    } else {
        SetPWMRaw (address, channel, 0, v);
    }
}


void SetPWMRaw (uint8_t address, uint8_t channel, uint16_t on, uint16_t off)
{
   NUM_BYTES_WR = 5;
   TARGET = address << 1;
   SMB_DATA_OUT[0] = 0x06 + 4 * channel;
   SMB_DATA_OUT[1] = on;
   SMB_DATA_OUT[2] = on >> 8;
   SMB_DATA_OUT[3] = off;
   SMB_DATA_OUT[4] = off >> 8;
   SMB_Write ();
}


void PCA9685Init (uint8_t address)
{
	   NUM_BYTES_WR = 2;
	   TARGET = address << 1;
	   SMB_DATA_OUT[0] = 0x00;
	   SMB_DATA_OUT[1] = 0x20; // enable register autoincrement, disable sleep
	   SMB_Write ();

	   NUM_BYTES_WR = 2;
	   TARGET = address << 1;
	   SMB_DATA_OUT[0] = 0x01;
	   SMB_DATA_OUT[1] = 0x00; // change on stop, open drain
	   SMB_Write ();
}
*/


void DacWrite (uint8_t sel, uint8_t *dat)
{
	if (sel == 0) {
		CS1 = 0;
	} else {
		CS2 = 0;
	}

    SPI0CN0_SPIF = 0;

	SPI0DAT = dat[0];
	while(!SPI0CN0_SPIF);
	SPI0CN0_SPIF = 0;

	SPI0DAT = dat[1];
	while(!SPI0CN0_SPIF);
	SPI0CN0_SPIF = 0;

    CS1 = 1;
	CS2 = 1;
}
