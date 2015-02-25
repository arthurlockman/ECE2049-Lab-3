#include <time.h>

inline int convert_temp(char* dest, float temp, int units)
{
    int temp_i = (int)temp;
    dest[0] = '0' + temp_i / 100;
    dest[1] = '0' + temp_i / 10;
    dest[2] = '0' + temp_i % 10;
    dest[3] = '.';
    temp_i = (int)(temp * 10);
    dest[4] = '0' + temp_i % 10;
    dest[5] = ' ';
    if (units == 1)
    	dest[6] = 'F';
    else
    	dest[6] = 'C';
    dest[7] = '\0';
    return 0;
};

inline int convert_time(char* dest, struct tm * time_ptr)
{
    dest[0] = '0' + time_ptr->tm_hour / 10;
    dest[1] = '0' + time_ptr->tm_hour % 10;
    dest[2] = ':';
    dest[3] = '0' + time_ptr->tm_min / 10;
    dest[4] = '0' + time_ptr->tm_min % 10;
    dest[5] = ':';
    dest[6] = '0' + time_ptr->tm_sec / 10;
    dest[7] = '0' + time_ptr->tm_sec % 10;
    dest[8] = '\0';
    return 0;
};

/**
 * @brief Creates the formatted date.
 *
 * @param dest The destination date. Must be already allocated.
 * @param time_ptr The time count.
 *
 * @return 0 if success.
 */
inline int convert_date(char* dest, struct tm * time_ptr)
{
    switch (time_ptr->tm_mon)
    {
    case 0:
    	dest[0] = 'J';
    	dest[1] = 'a';
    	dest[2] = 'n';
        break;
    case 1:
    	dest[0] = 'F';
    	dest[1] = 'e';
    	dest[2] = 'b';
        break;
    case 2:
    	dest[0] = 'M';
    	dest[1] = 'a';
        dest[2] = 'r';
        break;
    case 3:
    	dest[0] = 'A';
    	dest[1] = 'p';
    	dest[2] = 'r';
        break;
    case 4:
    	dest[0] = 'M';
    	dest[1] = 'a';
    	dest[2] = 'y';
        break;
    case 5:
    	dest[0] = 'J';
    	dest[1] = 'u';
    	dest[2] = 'n';
        break;
    case 6:
    	dest[0] = 'J';
    	dest[1] = 'u';
    	dest[2] = 'l';
        break;
    case 7:
    	dest[0] = 'A';
        dest[1] = 'u';
        dest[2] = 'g';
        break;
    case 8:
    	dest[0] = 'S';
    	dest[1] = 'e';
    	dest[2] = 'p';
        break;
    case 9:
    	dest[0] = 'O';
    	dest[1] = 'c';
    	dest[2] = 't';
        break;
    case 10:
    	dest[0] = 'N';
    	dest[1] = 'o';
    	dest[2] = 'v';
        break;
    case 11:
    	dest[0] = 'D';
    	dest[1] = 'e';
    	dest[2] = 'c';
        break;
    }
    dest[3] = ' ';
    dest[4] = '0' + time_ptr->tm_mday / 10;
    dest[5] = '0' + time_ptr->tm_mday % 10;
    dest[6] = '\0';
    return 0;
};

inline void increment_tm(struct tm * time_ptr, int seconds)
{
    time_ptr->tm_sec += seconds;
    mktime(time_ptr);
};

inline void initialize_tm(struct tm * time_ptr)
{
    unsigned long rawtime = 8569460;
    time_ptr->tm_yday = rawtime / 86400;
    rawtime -= rawtime * 86400;
    time_ptr->tm_hour = rawtime / 3600;
    rawtime -= rawtime * 3600;
    time_ptr->tm_min = rawtime / 60;
    rawtime -= rawtime * 60;
    time_ptr->tm_sec = rawtime;
    time_ptr->tm_year = 0;
    int yday = time_ptr->tm_yday;
	if (yday <= 31)
		time_ptr->tm_mon = 0;
	else if (yday -= 31 <= 28)
		time_ptr->tm_mon = 1;
	else if (yday -= 28 <= 31)
		time_ptr->tm_mon = 2;
	else if (yday -= 31 <= 30)
		time_ptr->tm_mon = 3;
	else if (yday -= 30 <= 31)
		time_ptr->tm_mon = 4;
	else if (yday -= 31 <= 30)
		time_ptr->tm_mon = 5;
	else if (yday -= 30 <= 31)
		time_ptr->tm_mon = 6;
	else if (yday -= 31 <= 31)
		time_ptr->tm_mon = 7;
	else if (yday -= 31 <= 30)
		time_ptr->tm_mon = 8;
	else if (yday -= 30 <= 31)
		time_ptr->tm_mon = 9;
	else if (yday -= 31 <= 30)
		time_ptr->tm_mon = 10;
	else if (yday -= 30 <= 31)
		time_ptr->tm_mon = 11;
	time_ptr->tm_mday = yday;
    mktime(time_ptr);
};

inline unsigned int utc_time_int(struct tm * time_ptr)
{
    return time_ptr->tm_yday * 86400 + time_ptr->tm_hour * 3600
    		+ time_ptr->tm_min * 60 + time_ptr->tm_sec;
};

inline int map(int x, int in_min, int in_max, int out_min, int out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};
