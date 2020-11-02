#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

#include <stdint.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	int fd;
	int i, j, k, n, res, desc_size = 0;
	uint8_t buf[256];
	struct hidraw_report_descriptor rpt_desc;
	struct hidraw_devinfo info;
	char *device = "/dev/hidraw0";

	if (argc > 1) {
		device = argv[1];
	}

	fd = open(device, O_RDWR|O_NONBLOCK);

	if (fd < 0) {
		perror("Unable to open device");
		return 1;
	}

	memset(&rpt_desc, 0x0, sizeof(rpt_desc));
	memset(&info, 0x0, sizeof(info));
	memset(buf, 0x0, sizeof(buf));

	// Get Raw Name
	res = ioctl(fd, HIDIOCGRAWNAME(256), buf);
	if (res < 0) {
		perror("HIDIOCGRAWNAME");
	} else {
		printf("Connected to %s.\n", buf);
	}


	int allMeters = false;
	int meterNumber = 0;
	int meterLevel = 0;

	if (argc > 2) {
		if (!strcmp (argv[2], "all")) {
			allMeters = true;
		} else {
			meterNumber = atoi (argv[2]);
			if ((meterNumber < 0) || (meterNumber > 3)) {
				printf ("bad meter number.\n");
				return -1;
			}
		}
	} else {
		printf ("usage: four-meters <1-4|all> <0-255>\n");
		return -1;
	}

	if (argc > 3) {
		meterLevel = atoi (argv[3]);
		if ((meterLevel < 0) || (meterLevel > 255)) {
			printf ("bad meter number.\n");
			return -1;
		}
	} else {
		printf ("usage: four-meters <1-4|all> <0-255>\n");
		return -1;
	}

	if (allMeters) {
		printf ("Setting all meters to %d...\n", meterLevel);
		buf[0] = 0x01;			// report id
		buf[1] = meterLevel;	// meter 1 level (1A)
		buf[2] = meterLevel;	// meter 2 level (1B)
		buf[3] = meterLevel;	// meter 3 level (2A)
		buf[4] = meterLevel;	// meter 4 level (2B)
		res = write(fd, buf, 5);
		if (res < 0) {
			printf("Error: %d\n", errno);
			perror("write");
		} else {
			printf("write() wrote %d bytes\n", res);
		}
	} else {
		printf ("Setting meter %d to %d...\n", meterNumber, meterLevel);
		buf[0] = 0x02;				// report id
		buf[1] = meterNumber;		// meter number
		buf[2] = meterLevel;		// meter level
		res = write(fd, buf, 3);
		if (res < 0) {
			printf("Error: %d\n", errno);
			perror("write");
		} else {
			printf("write() wrote %d bytes\n", res);
		}
	}

	close(fd);

	return 0;
}
