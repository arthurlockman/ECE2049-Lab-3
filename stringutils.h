#include <string.h>
#include <time.h>

inline int convert_temp(char* dest, float temp)
{
	return 0;
};

inline int convert_time(char* dest, struct tm * time_ptr)
{
    char formatted[9];
    formatted[0] = '0' + time_ptr->tm_hour / 10;
    formatted[1] = '0' + time_ptr->tm_hour % 10;
    formatted[2] = ':';
    formatted[3] = '0' + time_ptr->tm_min / 10;
    formatted[4] = '0' + time_ptr->tm_min % 10;
    formatted[5] = ':';
    formatted[6] = '0' + time_ptr->tm_sec / 10;
    formatted[7] = '0' + time_ptr->tm_sec & 10;
    formatted[8] = '\0';
    strcpy(dest, formatted);
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
	char formatted[7];
	switch(time_ptr->tm_mon)
	{
	case 0:
		formatted[0] = 'J';
		formatted[1] = 'a';
		formatted[2] = 'n';
		break;
	case 1:
		formatted[0] = 'F';
		formatted[1] = 'e';
		formatted[2] = 'b';
		break;
	case 2:
		formatted[0] = 'M';
		formatted[1] = 'a';
		formatted[2] = 'r';
		break;
	case 3:
		formatted[0] = 'A';
		formatted[1] = 'p';
		formatted[2] = 'r';
		break;
	case 4:
		formatted[0] = 'M';
		formatted[1] = 'a';
		formatted[2] = 'y';
		break;
	case 5:
		formatted[0] = 'J';
		formatted[1] = 'u';
		formatted[2] = 'n';
		break;
	case 6:
		formatted[0] = 'J';
		formatted[1] = 'u';
		formatted[2] = 'l';
		break;
	case 7:
		formatted[0] = 'A';
		formatted[1] = 'u';
		formatted[2] = 'g';
		break;
	case 8:
		formatted[0] = 'S';
		formatted[1] = 'e';
		formatted[2] = 'p';
		break;
	case 9:
		formatted[0] = 'O';
		formatted[1] = 'c';
		formatted[2] = 't';
		break;
	case 10:
		formatted[0] = 'N';
		formatted[1] = 'o';
		formatted[2] = 'v';
		break;
	case 11:
		formatted[0] = 'D';
		formatted[1] = 'e';
		formatted[2] = 'c';
		break;
	}
	formatted[3] = ' ';
	formatted[4] = '0' + time_ptr->tm_mday / 10;
	formatted[5] = '0' + time_ptr->tm_mday % 10;
	formatted[6] = '\0';
	strcpy(dest, formatted);
    return 0;
};

inline void increment_tm(struct tm * time_ptr, int seconds)
{
	time_ptr->tm_sec += seconds;
	mktime(time_ptr);
};

inline void initialize_tm(struct tm * time_ptr)
{
	time_t rawtime = 8569460;
	time_ptr = gmtime(&rawtime);
};

inline unsigned int utc_time_int(struct tm * time_ptr)
{
	return (unsigned int)mktime(time_ptr);
};
