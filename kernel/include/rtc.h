
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#pragma once

#include <types.h>

#define CMOS_SECOND		0x00
#define CMOS_MINUTE		0x02
#define CMOS_HOUR		0x04
#define CMOS_DAY		0x07
#define CMOS_MONTH		0x08
#define CMOS_YEAR		0x09
#define CMOS_STATUS_A		0x0A
#define CMOS_STATUS_B		0x0B
#define CMOS_DEFAULT_CENTURY	0x32		// should read this from ACPI FADT

typedef struct rtc_time_t
{
	uint8_t hour, minute, second;
	uint8_t day, month;
	uint16_t year;
} rtc_time_t;

void cmos_write(uint8_t, uint8_t);
uint8_t cmos_read(uint8_t);
void rtc_get_time(rtc_time_t *);





