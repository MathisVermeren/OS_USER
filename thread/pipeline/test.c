#include <stdio.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char **argv)
{
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec);

	for (int j=0;j<4;j++)
		for (int i=0;i<2000000000;i++);

        t = time(NULL);
        tm = *localtime(&t);
        printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec);

	return 0;
}
