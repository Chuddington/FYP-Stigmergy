/*! \file
 * \ingroup camera1
 * \brief Manage camera's interrupts (two timers)
 * \author Philippe Rétornaz
 * \verbinclude interrupt.s
 */


#include <p30f6014A.h>
#include "../../motor_led/e_epuck_ports.h"
#include "e_po3030k.h"



/*! The buffer to write to */
char * _po3030k_buffer;

/*! The flag to tell, the image is ready or not
 * Zero mean capture is in progress, non-zero mean capture done.
 * \sa e_po3030k_is_img_ready
 */
int _po3030k_img_ready;

static int blank_row_betw;

int __attribute__ ((near)) _po3030k_current_row;
int __attribute__ ((near)) _po3030k_row;

char _po3030k_line_conf[330];

/*! \brief The VSYNC interrupt.
 * This interrupt is called every time the Vertical sync signal is asserted
 * This mean that the picture is comming from the camera ( we will have the first line soon )
 */
void __attribute__((interrupt, auto_psv))
_T5Interrupt(void) {
	IFS1bits.T5IF = 0;
	/* let's enable Hsync */
	T4CONbits.TON = 1;
	/* single shot */
	T5CONbits.TON = 0;
}


static void init_timer5(void) {
	/* external pin, 1:1 prescaler */
	T5CON = 0x2;
	TMR5 = 0;
	PR5 = 1;
	IFS1bits.T5IF = 0;
	IEC1bits.T5IE = 1; 
	T5CONbits.TON = 1;
}

static void init_timer4(void) {
	T4CON = 0x2;
	TMR4 = blank_row_betw;
	PR4 = blank_row_betw + 1;
	IFS1bits.T4IF = 0;
	T4CONbits.TON = 0;
	IEC1bits.T4IE = 1;
}



/*! Launch a capture in the \a buf buffer
 * \param buf The buffer to write to
 * \sa e_po3030k_config_cam and e_po3030k_is_img_ready
 */
void e_po3030k_launch_capture(char * buf) {
	_po3030k_current_row = 0;
	_po3030k_buffer = buf;
	_po3030k_img_ready = 0;
	init_timer4();
	/* Timer5 must ALWAY be initialized as the last one */
	init_timer5();
	
}

/*! Modify the interrupt configuration
 * \warning This is an internal function, use \a e_po3030k_config_cam
 * \param pixel_row The number of row to take
 * \param pixel_col The number of pixel to take each \a pixel_row
 * \param bpp The number of byte per pixel
 * \param pbp The number of pixel to ignore between each pixel
 * \param bbl The number of row to ignore between each line
 * \return Zero if OK, non-zero if the mode exceed internal data representation
 * \sa e_po3030k_get_bytes_per_pixel and e_po3030k_config_cam
 */
int e_po3030k_apply_timer_config(int pixel_row, int pixel_col, int bpp, int pbp, int bbl) {
	int i;
	int pos = 0;
	if(pixel_col * bpp * (1+pbp) + 1 > sizeof(_po3030k_line_conf))
		return -1;

	for(i = 0; i < pixel_col; i++) {
		int j;
		for(j = 0; j < bpp; j++) {
			_po3030k_line_conf[pos++] = 1;
		}
		for(j = 0; j < pbp*bpp; j++)  {
			_po3030k_line_conf[pos++] = 0;
		}
	}
	_po3030k_line_conf[pos] = 2;  /* flag to tell "line end here" */
	blank_row_betw = bbl;
	_po3030k_row = pixel_row;

	return 0;
}

/*! Check if the current capture is finished
 * \return Zero if the current capture is in progress, non-zero if the capture is done.
 * \sa e_po3030k_launch_capture
 */
int e_po3030k_is_img_ready(void) {
	return _po3030k_img_ready;
}
