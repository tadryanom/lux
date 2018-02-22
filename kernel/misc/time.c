
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <time.h>
#include <rtc.h>
#include <kprintf.h>

#define SECONDS_PER_DAY			86400
#define SECONDS_PER_HOUR		3600
#define SECONDS_PER_MINUTE		60

// On a non-leap year
time_t days_per_month[] = {
	31, 28, 31, 30,
	31, 30, 31, 31,
	30, 31, 30, 31
};

// get_time(): Returns current Unix time
// Param:	Nothing
// Return:	time_t - current time

time_t get_time()
{
	time_t timestamp;

	// get the time from the RTC
	rtc_time_t rtc;
	rtc_get_time(&rtc);

	uint8_t is_leap_year;
	if(rtc.year % 4 == 0)
		is_leap_year = 1;
	else
		is_leap_year = 0;

	// how many leap years have there been?
	time_t leap_years = (time_t)(rtc.year - 1970) / 4;
	time_t normal_years = (time_t)(rtc.year - 1970) - leap_years;

	time_t total_days = (normal_years * 365) + (leap_years * 366);

	uint8_t month = rtc.month - 1;
	uint8_t i = 0;

	if(month == 0)
		total_days += rtc.day - 1;
	else
	{
		while(i < month)
		{
			if(i == 1 && is_leap_year == 1)
				total_days += 29;
			else
				total_days += days_per_month[i];

			i++;
		}

		total_days += rtc.day - 1;
	}

	// now we have total days, calculate the remaining of the time
	timestamp = total_days * SECONDS_PER_DAY;

	// add the current seconds
	timestamp += rtc.second;
	timestamp += rtc.minute * SECONDS_PER_MINUTE;
	timestamp += rtc.hour * SECONDS_PER_HOUR;

	return timestamp;
}


