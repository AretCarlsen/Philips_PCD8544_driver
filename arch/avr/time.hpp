
#pragma once

namespace Philips_PCD8544 {

/*
 * Name         :  Delay
 * Description  :  Uncalibrated delay for LCD init routine.
 * Argument(s)  :  None.
 * Return value :  None.
 */
inline static void Delay ( void ) {
    for ( int16_t i = -32000; i < 32000; i++ ) nop();
}

// End namespace: Philips_PCD8544
}

