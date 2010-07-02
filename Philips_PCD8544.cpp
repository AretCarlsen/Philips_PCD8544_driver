// Philips PCD8544 graphic LCD driver (C++).
// Licensed under GPLv3. See license.txt or <http://www.gnu.org/licenses/>.

/*
 * Name         :  LcdInit
 * Description  :  Performs MCU LCD controller initialization.
 * Argument(s)  :  None.
 * Return value :  None.
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> void Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::init ( void ) {
/*
    // Pull-up on reset pin.
    LCD_RST_pin.enable_pullup();
*/

    Delay();

    /* Toggle display reset pin. */
    LCD_RST_pin.set_output_low();
    Delay();
    LCD_RST_pin.set_output_high();

    /* Disable LCD controller */
    LCD_CE_pin.set_output_high();

    send( 0x21, LCD_CMD ); /* LCD Extended Commands. */
    send( 0xC8, LCD_CMD ); /* Set LCD Vop (Contrast).*/
    send( 0x06, LCD_CMD ); /* Set Temp coefficent. */
    send( 0x13, LCD_CMD ); /* LCD bias mode 1:48. */
    send( 0x20, LCD_CMD ); /* LCD Standard Commands,Horizontal addressing mode */
    send( 0x0C, LCD_CMD ); /* LCD in normal mode. */

    /* Reset watermark pointers to empty */
    LoWaterMark = CACHE_SIZE;
    HiWaterMark = 0;

    /* Clear display on first time use */
    clear();
    update();
}

/*
 * Name         :  LcdContrast
 * Description  :  Set display contrast.
 * Argument(s)  :  contrast -> Contrast value from 0x00 to 0x7F.
 * Return value :  None.
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> void Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::contrast ( byte contrast ) {
    /* LCD Extended Commands. */
    send( 0x21, LCD_CMD );

    /* Set LCD contrast level. */
    send( 0x80 | contrast, LCD_CMD );

    /* LCD Standard Commands, horizontal addressing mode. */
    send( 0x20, LCD_CMD );
}

/*
 * Name         :  clear
 * Description  :  Clears the display. update must be called next.
 * Argument(s)  :  None.
 * Return value :  None.
 * Note         :  Based on Sylvain Bissonette's code
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> void Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::clear ( void ) {
// Removed in version 0.2.6, March 14 2009
// Optimized by Jakub Lasinski
//    int i;
//
//    /* Set 0x00 to all screenCache's contents */
//    for ( i = 0; i < CACHE_SIZE; i++ )
//    {
//        screenCache[ i ] = 0x00;
//    }
    memset(screenCache,0x00,CACHE_SIZE);  //Suggestion - its faster and its 10 bytes less in program mem
    /* Reset watermark pointers to full */
    LoWaterMark = 0;
    HiWaterMark = CACHE_SIZE - 1;

    /* Set update flag to be true */
    updateActive = TRUE;
}

/*
 * Name         :  gotoXYFont
 * Description  :  Sets cursor location to xy location corresponding to basic
 *                 font size.
 * Argument(s)  :  x, y -> Coordinate for new cursor position. Range: 1,1 .. 14,6
 * Return value :  see return value in pcd8544.h
 * Note         :  Based on Sylvain Bissonette's code
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> byte Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::gotoXYFont ( byte x, byte y ) {
    /* Boundary check, slow down the speed but will guarantee this code wont fail */
    /* Version 0.2.5 - Fixed on Dec 25, 2008 (XMAS) */
    if( x > MAX_X_FONT)
        return OUT_OF_BORDER;
    if( y > MAX_Y_FONT)
        return OUT_OF_BORDER;
    /*  Calculate index. It is defined as address within 504 bytes memory */

    CacheIdx = ( x - 1 ) * 6 + ( y - 1 ) * 84;
    return OK;
}

/*
 * Name         :  chr
 * Description  :  Displays a character at current cursor location and
 *                 increment cursor location.
 * Argument(s)  :  size -> Font size. See enum in pcd8544.h.
 *                 ch   -> Character to write.
 * Return value :  see pcd8544.h about return value
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> byte Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::chr ( LcdFontSize size, byte ch ) {
    byte i, c;
    byte b1, b2;
    CacheIndex_t  tmpIdx;

    if ( CacheIdx < LoWaterMark ){
        /* Update low marker. */
        LoWaterMark = CacheIdx;
    }

    if ( (ch < 0x20) || (ch > 0x7b) ){
        /* Convert to a printable character. */
        ch = 92;
    }

    if ( size == FONT_1X ){
        for ( i = 0; i < 5; i++ )
        {
            /* Copy lookup table from Flash ROM to screenCache */
            screenCache[CacheIdx++] = get_font_byte(ch - 32, i) << 1;
        }
    }else if ( size == FONT_2X ){
// Modified to eliminate signedness [ANC 2010-04-24]
        if(CacheIdx < 84){
          LoWaterMark = 0;
          return OUT_OF_BORDER;
        }

        tmpIdx = CacheIdx - 84;

        if ( tmpIdx < LoWaterMark )
            LoWaterMark = tmpIdx;

        for ( i = 0; i < 5; i++ )
        {
            /* Copy lookup table from Flash ROM to temporary c */
            c = get_font_byte(ch - 32, i) << 1;
            /* Enlarge image */
            /* First part */
            b1 =  (c & 0x01) * 3;
            b1 |= (c & 0x02) * 6;
            b1 |= (c & 0x04) * 12;
            b1 |= (c & 0x08) * 24;

            c >>= 4;
            /* Second part */
            b2 =  (c & 0x01) * 3;
            b2 |= (c & 0x02) * 6;
            b2 |= (c & 0x04) * 12;
            b2 |= (c & 0x08) * 24;

            /* Copy two parts into screenCache */
            screenCache[tmpIdx++] = b1;
            screenCache[tmpIdx++] = b1;
            screenCache[tmpIdx + 82] = b2;
            screenCache[tmpIdx + 83] = b2;
        }

        /* Update x cursor position. */
        /* Version 0.2.5 - Possible bug fixed on Dec 25,2008 */
        CacheIdx = (CacheIdx + 11) % CACHE_SIZE;
    }

    if ( CacheIdx > HiWaterMark ){
        /* Update high marker. */
        HiWaterMark = CacheIdx;
    }

    /* Horizontal gap between characters. */
    /* Version 0.2.5 - Possible bug fixed on Dec 25,2008 */
    screenCache[CacheIdx] = 0x00;
    /* At index number CACHE_SIZE - 1, wrap to 0 */
    if(CacheIdx == (CACHE_SIZE - 1) )
    {
        CacheIdx = 0;
        return OK_WITH_WRAP;
    }
    /* Otherwise just increment the index */
    CacheIdx++;
    return OK;
}

/*
 * Name         :  str
 * Description  :  Displays a character at current cursor location and increment
 *                 cursor location according to font size. This function is
 *                 dedicated to print string laid in SRAM
 * Argument(s)  :  size      -> Font size. See enum.
 *                 dataArray -> Array contained string of char to be written
 *                              into screenCache.
 * Return value :  see return value on pcd8544.h
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> byte Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::str ( LcdFontSize size, byte dataArray[] ) {
    byte tmpIdx=0;
    byte response;
    while( dataArray[ tmpIdx ] != '\0' ){
        /* Send char */
      response = chr( size, dataArray[ tmpIdx ] );
        /* Just in case OUT_OF_BORDER occured */
        /* Dont worry if the signal == OK_WITH_WRAP, the string will
        be wrapped to starting point */
      if( response == OUT_OF_BORDER)
        return OUT_OF_BORDER;
      /* Increase index */
      tmpIdx++;
    }
    return OK;
}

/*
 * Name         :  fStr
 * Description  :  Displays a characters at current cursor location and increment
 *                 cursor location according to font size. This function is
 *                 dedicated to print string laid in Flash ROM
 * Argument(s)  :  size    -> Font size. See enum.
 *                 dataPtr -> Pointer contained string of char to be written
 *                            into screenCache.
 * Return value :  see return value on pcd8544.h
 * Example      :  fStr(FONT_1X, PSTR("Hello World"));
 *                 fStr(FONT_1X, &name_of_string_as_array);
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> byte Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::fStr ( LcdFontSize size, const byte *dataPtr ) {
    byte c;
    byte response;
    for ( c = pgm_read_byte( dataPtr ); c; ++dataPtr, c = pgm_read_byte( dataPtr ) )
    {
        /* Put char */
        response = chr( size, c );
        if(response == OUT_OF_BORDER)
            return OUT_OF_BORDER;
    }
	/* Fixed by Jakub Lasinski. Version 0.2.6, March 14, 2009 */
    return OK;
}

/*
 * Name         :  pixel
 * Description  :  Displays a pixel at given absolute (x, y) location.
 * Argument(s)  :  x, y -> Absolute pixel coordinates
 *                 mode -> Off, On or Xor. See enum in pcd8544.h.
 * Return value :  see return value on pcd8544.h
 * Note         :  Based on Sylvain Bissonette's code
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> byte Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::pixel ( byte x, byte y, PixelMode mode ) {
    CacheIndex_t  index;
    byte  offset;
    byte  data;

    /* Prevent from getting out of border */
    if ( x > X_RES ) return OUT_OF_BORDER;
    if ( y > Y_RES ) return OUT_OF_BORDER;

    /* Recalculating index and offset */
    index = ( ( y / 8 ) * 84 ) + x;
    offset  = y - ( ( y / 8 ) * 8 );

    data = screenCache[ index ];

    /* Bit processing */

	/* Clear mode */
    if ( mode == PIXEL_OFF )
        data &= ( ~( 0x01 << offset ) );

    /* On mode */
    else if ( mode == PIXEL_ON )
        data |= ( 0x01 << offset );

    /* Xor mode */
    else if ( mode  == PIXEL_XOR )
        data ^= ( 0x01 << offset );

    /* Final result copied to screenCache */
    screenCache[ index ] = data;

    if ( index < LoWaterMark )
        /*  Update low marker. */
        LoWaterMark = index;

    if ( index > HiWaterMark )
        /*  Update high marker. */
        HiWaterMark = index;

    return OK;
}

/*
 * Name         :  line
 * Description  :  Draws a line between two points on the display.
 * Argument(s)  :  x1, y1 -> Absolute pixel coordinates for line origin.
 *                 x2, y2 -> Absolute pixel coordinates for line end.
 *                 mode   -> Off, On or Xor. See enum in pcd8544.h.
 * Return value :  see return value on pcd8544.h
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> byte Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::line ( byte x1, byte x2, byte y1, byte y2, PixelMode mode ) {
    int dx, dy, stepx, stepy, fraction;
    byte response;

    /* Calculate differential form */
    /* dy   y2 - y1 */
    /* -- = ------- */
    /* dx   x2 - x1 */

    /* Take differences */
    dy = y2 - y1;
    dx = x2 - x1;

    /* dy is negative */
    if ( dy < 0 )
    {
        dy    = -dy;
        stepy = -1;
    }
    else
    {
        stepy = 1;
    }

    /* dx is negative */
    if ( dx < 0 )
    {
        dx    = -dx;
        stepx = -1;
    }
    else
    {
        stepx = 1;
    }

    dx <<= 1;
    dy <<= 1;

    /* Draw initial position */
    response = pixel( x1, y1, mode );
    if(response)
        return response;

    /* Draw next positions until end */
    if ( dx > dy )
    {
        /* Take fraction */
        fraction = dy - ( dx >> 1);
        while ( x1 != x2 )
        {
            if ( fraction >= 0 )
            {
                y1 += stepy;
                fraction -= dx;
            }
            x1 += stepx;
            fraction += dy;

            /* Draw calculated point */
            response = pixel( x1, y1, mode );
            if(response)
                return response;

        }
    }
    else
    {
        /* Take fraction */
        fraction = dx - ( dy >> 1);
        while ( y1 != y2 )
        {
            if ( fraction >= 0 )
            {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;

            /* Draw calculated point */
            response = pixel( x1, y1, mode );
            if(response)
                return response;
        }
    }

    /* Set update flag to be true */
    updateActive = TRUE;
    return OK;
}

/*
 * Name         :  singleBar
 * Description  :  Display single bar.
 * Argument(s)  :  baseX  -> absolute x axis coordinate
 *                 baseY  -> absolute y axis coordinate
 *				   height -> height of bar (in pixel)
 *				   width  -> width of bar (in pixel)
 *				   mode   -> Off, On or Xor. See enum in pcd8544.h.
 * Return value :  see return value on pcd8544.h
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> byte Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::singleBar ( byte baseX, byte baseY, byte height, byte width, PixelMode mode ) {
	byte tmpIdxX,tmpIdxY,tmp;

    byte response;

    /* Checking border */
	if ( ( baseX > X_RES ) || ( baseY > Y_RES ) ) return OUT_OF_BORDER;

	if ( height > baseY )
		tmp = 0;
	else
		tmp = baseY - height;

    /* Draw lines */
	for ( tmpIdxY = tmp; tmpIdxY < baseY; tmpIdxY++ )
	{
		for ( tmpIdxX = baseX; tmpIdxX < (baseX + width); tmpIdxX++ )
        {
			response = pixel( tmpIdxX, tmpIdxY, mode );
            if(response)
                return response;

        }
	}

    /* Set update flag to be true */
	updateActive = TRUE;
    return OK;
}

/*
 * Name         :  bars
 * Description  :  Display multiple bars.
 * Argument(s)  :  data[] -> data which want to be plotted
 *                 numbBars  -> number of bars want to be plotted
 *				   width  -> width of bar (in pixel)
 * Return value :  see return value on pcd8544.h
 * Note         :  Please check EMPTY_SPACE_BARS, BAR_X, BAR_Y in pcd8544.h
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> byte Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::bars ( byte data[], byte numbBars, byte width, byte multiplier ) {
	byte b;
	byte tmpIdx = 0;
    byte response;

	for ( b = 0;  b < numbBars ; b++ )
	{
        /* Preventing from out of border (X_RES) */
		if ( tmpIdx > X_RES ) return OUT_OF_BORDER;

		/* Calculate x axis */
		tmpIdx = ((width + EMPTY_SPACE_BARS) * b) + BAR_X;

		/* Draw single bar */
		response = singleBar( tmpIdx, BAR_Y, data[ b ] * multiplier, width, PIXEL_ON);
        if(response == OUT_OF_BORDER)
            return response;
	}

	/* Set update flag to be true */
	updateActive = TRUE;
    return OK;

}
/*
 * Name         :  rect
 * Description  :  Display a rectangle.
 * Argument(s)  :  x1   -> absolute first x axis coordinate
 *                 y1   -> absolute first y axis coordinate
 *				   x2   -> absolute second x axis coordinate
 *				   y2   -> absolute second y axis coordinate
 *				   mode -> Off, On or Xor. See enum in pcd8544.h.
 * Return value :  see return value on pcd8544.h.
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> byte Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::rect ( byte x1, byte x2, byte y1, byte y2, PixelMode mode ) {
	byte tmpIdxX,tmpIdxY;
    byte response;

	/* Checking border */
	if ( ( x1 > X_RES ) ||  ( x2 > X_RES ) || ( y1 > Y_RES ) || ( y2 > Y_RES ) )
		/* If out of border then return */
		return OUT_OF_BORDER;

	if ( ( x2 > x1 ) && ( y2 > y1 ) )
	{
		for ( tmpIdxY = y1; tmpIdxY < y2; tmpIdxY++ )
		{
			/* Draw line horizontally */
			for ( tmpIdxX = x1; tmpIdxX < x2; tmpIdxX++ )
            {
				/* Draw a pixel */
				response = pixel( tmpIdxX, tmpIdxY, mode );
                if(response)
                    return response;
            }
		}

		/* Set update flag to be true */
		updateActive = TRUE;
	}
    return OK;
}
/*
 * Name         :  image
 * Description  :  Image mode display routine.
 * Argument(s)  :  Address of image in hexes
 * Return value :  None.
 * Example      :  image(&sample_image_declared_as_array);
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> void Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::image ( const byte *imageData ) {
	/* Initialize screenCache index to 0 */
//	CacheIdx = 0;
//	/* While within screenCache range */
//    for ( CacheIdx = 0; CacheIdx < CACHE_SIZE; CacheIdx++ )
//    {
//		/* Copy data from pointer to screenCache buffer */
//        screenCache[CacheIdx] = pgm_read_byte( imageData++ );
//    }
	/* optimized by Jakub Lasinski, version 0.2.6, March 14, 2009 */
    memcpy_P(screenCache,imageData,CACHE_SIZE);	//Same as aboeve - 6 bytes less and faster instruction
	/* Reset watermark pointers to be full */
    LoWaterMark = 0;
    HiWaterMark = CACHE_SIZE - 1;

	/* Set update flag to be true */
    updateActive = TRUE;
}

/*
 * Name         :  update
 * Description  :  Copies the LCD screenCache into the device RAM.
 * Argument(s)  :  None.
 * Return value :  None.
 */
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> void Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::update ( void ) {
    CacheIndex_t i;

    if ( LoWaterMark >= CACHE_SIZE )
        LoWaterMark = CACHE_SIZE - 1;

    if ( HiWaterMark >= CACHE_SIZE )
        HiWaterMark = CACHE_SIZE - 1;

    /*  Set base address according to LoWaterMark. */
    send( 0x80 | ( LoWaterMark % X_RES ), LCD_CMD );
    send( 0x40 | ( LoWaterMark / X_RES ), LCD_CMD );

    /*  Serialize the display buffer. */
    for ( i = LoWaterMark; i <= HiWaterMark; i++ )
        send( screenCache[ i ], LCD_DATA );

    /*  Reset watermark pointers. */
    LoWaterMark = CACHE_SIZE - 1;
    HiWaterMark = 0;

    /* Set update flag to be true */
    updateActive = FALSE;
}

/*
 * Name         :  send
 * Description  :  Sends data to display controller.
 * Argument(s)  :  data -> Data to be sent
 *                 cd   -> Command or data (see enum in pcd8544.h)
 * Return value :  None.
 */
// Was static
template<typename SPI_bus_t, typename LCD_DC_pin_t, typename LCD_CE_pin_t, typename LCD_RST_pin_t, int X_RES, int Y_RES> void Philips_PCD8544<SPI_bus_t, LCD_DC_pin_t, LCD_CE_pin_t, LCD_RST_pin_t, X_RES, Y_RES>::send ( byte data, LcdCmdData cd ) {
    /*  Enable display controller (active low). */
    LCD_CE_pin.set_output_low();

    if ( cd == LCD_DATA )
        LCD_DC_pin.set_output_high();
    else
        LCD_DC_pin.set_output_low();

    /*  Send data to display controller. */
    SPI_bus.transceive(data);

    /* Disable display controller. */
    LCD_CE_pin.set_output_high();
}

