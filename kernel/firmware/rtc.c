
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <rtc.h>
#include <io.h>

uint8_t bcd_to_dec(uint8_t);

// cmos_write(): Writes a CMOS register
// Param:	uint8_t index - index of register
// Param:	uint8_t value - value to write
// Return:	Nothing

void cmos_write(uint8_t index, uint8_t value)
{
	outb(0x70, index);
	iowait();
	outb(0x71, value);
	iowait();
}

// cmos_read(): Reads a CMOS register
// Param:	uint8_t index - index of register
// Return:	uint8_t - value from register

uint8_t cmos_read(uint8_t index)
{
	outb(0x70, index);
	iowait();
	return inb(0x71);
}

// bcd_to_dec(): Converts a binary-coded decimal to a decimal
// Param:	uint8_t bcd - BCD value
// Return:	uint8_t - decimal value

inline uint8_t bcd_to_dec(uint8_t bcd)
{
	return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

// rtc_get_time(): Gets the current 24-hour time and date from the RTC
// Param:	rtc_time_t *time - structure to store time/date
// Return:	Nothing

void rtc_get_time(rtc_time_t *time)
{
	uint8_t hour;		// for checking AM/PM

	// date first, because it's easier
	time->day = bcd_to_dec(cmos_read(CMOS_DAY));
	time->month = bcd_to_dec(cmos_read(CMOS_MONTH));
	time->year = (uint16_t)(bcd_to_dec(cmos_read(CMOS_YEAR)) + (bcd_to_dec(cmos_read(CMOS_DEFAULT_CENTURY)) * 100));

	time->minute = bcd_to_dec(cmos_read(CMOS_MINUTE));
	time->second = bcd_to_dec(cmos_read(CMOS_SECOND));

	hour = cmos_read(CMOS_HOUR);

	// test 24-hour or 12-hour time
	if(cmos_read(CMOS_STATUS_B) & 2)
	{
		// 24-hour time
		time->hour = bcd_to_dec(hour);
	} else
	{
		// 12-hour time
		if(hour & 0x80)
		{
			// PM
			hour &= 0x7F;
			time->hour = bcd_to_dec(hour) + 12;
		} else
		{
			// AM
			if(hour == 0x12)		// 12 midnight?
				time->hour = 0;

			else
				time->hour = bcd_to_dec(hour);
		}
	}
}






