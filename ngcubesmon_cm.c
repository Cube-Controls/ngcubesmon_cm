#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define GPIO5   5 
#define GPIO20  20
#define GPIO6   6

static int GPIOExport(int pin)
{
	#define BUFFER_MAX 3
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) 
	{
		fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

static int GPIODirection(int pin, int dir)
{
	static const char s_directions_str[]  = "in\0out";

	#define DIRECTION_MAX 35
	char path[DIRECTION_MAX];
	int fd;

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) 
	{
		fprintf(stderr, "Failed to open gpio direction for writing!\n");
		return(-1);
	}

	if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) 
	{
		fprintf(stderr, "Failed to set direction!\n");
		return(-1);
	}

	close(fd);
	return(0);
}

static int GPIORead(int pin)
{
	#define VALUE_MAX 30
	char path[VALUE_MAX];
	char value_str[3];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if (-1 == fd) 
	{
		fprintf(stderr, "Failed to open gpio value for reading!\n");
		return(-1);
	}

	if (-1 == read(fd, value_str, 3)) 
	{
		fprintf(stderr, "Failed to read value!\n");
		return(-1);
	}

	close(fd);

	return(atoi(value_str));
}

static int GPIOWrite(int pin, int value)
{
	static const char s_values_str[] = "01";

	char path[VALUE_MAX];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) 
	{
		fprintf(stderr, "Failed to open gpio value for writing!\n");
		return(-1);
	}

	if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) 
	{
		fprintf(stderr, "Failed to write value!\n");
		return(-1);
	}

	close(fd);
	return(0);
}

//*********************************************************************************
// Main loop
//*********************************************************************************


int main(int argc, char *argv[])
{
	int lastGPIO5 = 0;
	int valueGPIO20 = 0;
	int counterGPIO5 = 0;
	
	// export pins
	if (GPIOExport(GPIO5) == -1) return(1);	
	if (GPIOExport(GPIO6) == -1) return(1);	
	if (GPIOExport(GPIO20) == -1) return(1);
	// setup direction
	if (GPIODirection(GPIO5, IN) == -1) return(1);
	if (GPIODirection(GPIO20, OUT) == -1) return(1);
	if (GPIODirection(GPIO6, OUT) == -1) return(1);
	// Enable LAN
        GPIOWrite(GPIO6,1);
	
	for(;;)
	{
		if (GPIORead(GPIO5) != lastGPIO5)
		{
			lastGPIO5 = GPIORead(GPIO5);
		}	
		
		// count if power is off
		if (GPIORead(GPIO5) == 0) counterGPIO5++;
		else 					  counterGPIO5=0;
		
		if (counterGPIO5 >= 4)
		{
			printf("Shuting down the system!");
			// Enable LAN
		        GPIOWrite(GPIO6,0);
			system("sudo shutdown -h now");
		}
		
		
		// sleep 2 seconds
		sleep(2);
		GPIOWrite(GPIO20,valueGPIO20);
		
		// flip it
		valueGPIO20 = !valueGPIO20;
		
	
	}// for

	return 0;
}