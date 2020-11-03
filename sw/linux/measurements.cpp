#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <signal.h>
#include <errno.h>

int OpenSocket (char *address, uint16_t port);
void CloseSocket (int socket);
void PrintIdentity (int sockfd);
void ConfigureCurrentMeasurement (int sockfd);
void ConfigureVoltageMeasurement (int sockfd);
double GetMeasurement (int sockfd);

int main (int argc, char *argv[])
{
	int i;
	int dmmFd;
	int metersFd;
	double meas;
	uint8_t buf[256];

	// open socket
	dmmFd = OpenSocket ((const char *)"192.168.180.189", 5025);
	if (dmmFd < 0) {
		printf ("Error: OpenSocket\n");
		return -1;
	}

	metersFd = open((const char *)"/dev/hidraw0", O_RDWR|O_NONBLOCK);

	PrintIdentity (dmmFd);
	ConfigureCurrentMeasurement (dmmFd);

	for (i = 0; i < 256; i++) {
        buf[0] = 0x02;              // report id
        buf[1] = 0;       			// meter number
        buf[2] = i;        			// meter level
        write(metersFd, buf, 3);
		
		usleep (100000);
		meas = GetMeasurement (dmmFd);
		printf ("%3d %10.6f\n", i, meas);
		usleep (100000);
	}

	CloseSocket (dmmFd);
	close (metersFd);

	return 0;
}


#ifdef NOPE
void Quit (int sig)
{
	if (ftHandle != NULL) {
		FT_Close (ftHandle);
		ftHandle = NULL;
		printf ("Closed device.\n");
    }
	if (fdSocket != -1) {
		close (fdSocket);
		fdSocket = -1;
		printf ("Closed socket.\n");
	}

	exit (-1);
}


int ListConnectedDevices (void)
{
	FT_STATUS ftStatus;
    char *pcBufLD[MAX_DEVICES + 1];
    char cBufLD[MAX_DEVICES][64];
	int i, iNumDevs;

    for (i = 0; i < MAX_DEVICES; i++) {
        pcBufLD[i] = cBufLD[i];
    }
    pcBufLD[MAX_DEVICES] = NULL;
	
    ftStatus = FT_ListDevices(pcBufLD, &iNumDevs, FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);
	if (ftStatus != FT_OK) {
        printf ("Error: FT_ListDevices: %d\n", ftStatus);
        return -1;
    }

    for (i = 0; ((i <MAX_DEVICES) && (i < iNumDevs)); i++) {
        printf ("Device %d Serial Number - %s\n", i, cBufLD[i]);
    }
}


int RunShow (char *device, char *address, uint16_t port, int measurement)
{
	int status = -1;
	FT_STATUS ftStatus;
	unsigned int bytesToRead;
	unsigned int bytesToWrite;
	unsigned int bytesRead;
	unsigned int bytesWritten;
	unsigned char rxBuffer[2048];
	unsigned char txBuffer[2048];

	double value;

	do {

		// open device
		ftStatus = OpenDevice (device, &ftHandle);
		if (ftStatus != FT_OK) {
			printf ("Error: OpenDevice: %d\n", ftStatus);
			break;
		}


		do {

			// print dmm identity
			PrintIdentity (fdSocket);

			// configure dmm measurement type
			if (measurement == 1) {
				// configure for current measurement
				ConfigureCurrentMeasurement (fdSocket);
			} else if (measurement == 2) {
				// configure for voltage measurement
				ConfigureVoltageMeasurement (fdSocket);
			}

			// strip 0, bank 0, address 0, 150 lights, red
			printf ("Red:       "); fflush (stdout);
			SendColor (ftHandle, 0, 0, 0, 150, 0xff, 0x00, 0x00);	
			sleep (1);
			value = GetMeasurement (fdSocket);
			printf ("%10.6f\n", value);
			sleep (1);

			// strip 0, bank 0, address 0, 150 lights, green
			printf ("Green:     "); fflush (stdout);
			SendColor (ftHandle, 0, 0, 0, 150, 0x00, 0xff, 0x00);	
			sleep (1);
			value = GetMeasurement (fdSocket);
			printf ("%10.6f\n", value);
			sleep (1);

			// strip 0, bank 0, address 0, 150 lights, blue
			printf ("Blue:      "); fflush (stdout);
			SendColor (ftHandle, 0, 0, 0, 150, 0x00, 0x00, 0xff);	
			sleep (1);
			value = GetMeasurement (fdSocket);
			printf ("%10.6f\n", value);
			sleep (1);

			// strip 0, bank 0, address 0, 150 lights, cyan
			printf ("Cyan:      "); fflush (stdout);
			SendColor (ftHandle, 0, 0, 0, 150, 0x00, 0xff, 0xff);	
			sleep (1);
			value = GetMeasurement (fdSocket);
			printf ("%10.6f\n", value);
			sleep (1);

			// strip 0, bank 0, address 0, 150 lights, magenta
			printf ("Magenta:   "); fflush (stdout);
			SendColor (ftHandle, 0, 0, 0, 150, 0xff, 0x00, 0xff);	
			sleep (1);
			value = GetMeasurement (fdSocket);
			printf ("%10.6f\n", value);
			sleep (1);

			// strip 0, bank 0, address 0, 150 lights, yellow
			printf ("Yellow:    "); fflush (stdout);
			SendColor (ftHandle, 0, 0, 0, 150, 0xff, 0xff, 0x00);	
			sleep (1);
			value = GetMeasurement (fdSocket);
			printf ("%10.6f\n", value);
			sleep (1);

			// strip 0, bank 0, address 0, 150 lights, white
			printf ("White:     "); fflush (stdout);
			SendColor (ftHandle, 0, 0, 0, 150, 0xff, 0xff, 0xff);	
			sleep (1);
			value = GetMeasurement (fdSocket);
			printf ("%10.6f\n", value);
			sleep (1);

			// strip 0, bank 0, address 0, 150 lights, off
			printf ("Off:       "); fflush (stdout);
			SendColor (ftHandle, 0, 0, 0, 150, 0x00, 0x00, 0x00);	
			sleep (1);
			value = GetMeasurement (fdSocket);
			printf ("%10.6f\n", value);
			sleep (1);

		} while (0);

		// purge receive buffer
		ftStatus = PurgeReceiveBuffer (ftHandle);
		if (ftStatus != FT_OK) {
			printf ("Error: PurgeReceiveBuffer: %d\n", ftStatus);
			break;
		}

		// close device
		CloseDevice (&ftHandle);

		// close socket
		CloseSocket (fdSocket);

		status = 0;

	} while (0);

	return status;
}


FT_STATUS OpenDevice (char *device, FT_HANDLE *ftHandle)
{
	FT_STATUS ftStatus;
	char rxBuffer[128];
	char txBuffer[128];
	unsigned int bytesToRead, bytesRead;
	unsigned int bytesToWrite, bytesWritten;

	do {

		// open device
		ftStatus = FT_OpenEx (device, FT_OPEN_BY_SERIAL_NUMBER, ftHandle);
		if (ftStatus != FT_OK) {
			printf ("Error: FT_OpenEX: %d\n", ftStatus);
			break;
		}
		printf ("Opened device.\n");

		// reset device
		ftStatus = FT_ResetDevice (*ftHandle);
		if (ftStatus != FT_OK) {
			printf ("Error: FT_ResetDevice: %d\n", ftStatus);
			break;
		}
		
		// purge receive buffer
		ftStatus = PurgeReceiveBuffer (*ftHandle);
		if (ftStatus != FT_OK) {
			printf ("Error: PurgeReceiveBuffer: %d\n", ftStatus);
			break;
		}

		// set transfer sizes to 65536 bytes
		ftStatus = FT_SetUSBParameters (*ftHandle, 65536, 65535);
		if (ftStatus != FT_OK) {
			printf ("Error: FT_SetUSBParameters: %d\n", ftStatus);
			break;
		}

		// disable event and error characters
		ftStatus = FT_SetChars (*ftHandle, false, 0, false, 0);
		if (ftStatus != FT_OK) {
			printf ("Error: FT_SetChars: %d\n", ftStatus);
			break;
		}

		// set read and write timeouts in milliseconds
		ftStatus = FT_SetTimeouts (*ftHandle, 0, 5000);
		if (ftStatus != FT_OK) {
			printf ("Error: FT_SetTimeouts: %d\n", ftStatus);
			break;
		}

		// set latency timer to 1ms
		ftStatus = FT_SetLatencyTimer (*ftHandle, 1);
		if (ftStatus != FT_OK) {
			printf ("Error: FT_SetLatencyTimer: %d\n", ftStatus);
			break;
		}
		
		// disable flow control
		ftStatus = FT_SetFlowControl (*ftHandle, FT_FLOW_NONE, 0x00, 0x00);
		if (ftStatus != FT_OK) {
			printf ("Error: FT_SetFlowControl: %d\n", ftStatus);
			break;
		}

		// reset controller
		ftStatus = FT_SetBitMode (*ftHandle, 0x0, 0x00);
		if (ftStatus != FT_OK) {
			printf ("Error: FT_SetBitMode: %d\n", ftStatus);
			break;
		}

		// set baud rate
		ftStatus = FT_SetBaudRate (*ftHandle, 12000000);
		if (ftStatus != FT_OK) {
			printf ("Error: FT_SetBaudRate: %d\n", ftStatus);
			break;
		}

	} while (0);

	return ftStatus;
}


void CloseDevice (FT_HANDLE *ftHandle)
{
	// if device is opened, close it
	if (ftHandle != NULL) {
		FT_Close (*ftHandle);
		*ftHandle = NULL;
		printf ("Closed device.\n");
	}
}




FT_STATUS PurgeReceiveBuffer (FT_HANDLE ftHandle)
{
	FT_STATUS ftStatus;
	char rxBuffer[128];
	unsigned int bytesToRead, bytesRead;

	do {

		// purge receive buffer
		ftStatus = FT_GetQueueStatus (ftHandle, &bytesToRead);
		if (ftStatus != FT_OK) {
			printf ("Error: FT_GetQueueStatus: %d\n", ftStatus);
			break;
		}
		while (bytesToRead > 0) {
			ftStatus = FT_Read (ftHandle, rxBuffer, 1, &bytesRead);
			if (ftStatus != FT_OK) {
				break;
			}
			bytesToRead -= bytesRead;
		}
		if (ftStatus != FT_OK) {
			printf ("Error: FT_Read: %d\n", ftStatus);
			break;
		}
		printf ("Purged device RX channel.\n");

	} while (0);

	return ftStatus;
}


void SendColor (FT_HANDLE ftHandle, uint8_t strip, uint8_t bank, uint8_t first, uint8_t count, 
		uint8_t red, uint8_t green, uint8_t blue)
{
	uint8_t color[6];
	uint8_t txBuffer[2048];
	uint8_t *txPtr;
	unsigned int bytesWritten;

	txPtr = txBuffer;
	
	*txPtr++ = 'w';
	sprintf ((char *)txPtr, "%02x%02x%02x", strip, bank, first);
	txPtr += 6;

	sprintf ((char *)color, "%02x%02x%02x", red, green, blue);

	for (int i = 0; i < count; i++) {
		memcpy (txPtr, color, 6);
		txPtr += 6;
	}

	*txPtr++ = '\n';
	*txPtr++ = 0;

	FT_Write (ftHandle, txBuffer, strlen ((char *)txBuffer), &bytesWritten);
}
#endif


int OpenSocket (char *address, uint16_t port)
{
	int sockfd;
	struct sockaddr_in serv_addr;

    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        printf ("socket failed\n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons (port);

    if (inet_pton (AF_INET, address, &serv_addr.sin_addr) <= 0)
    {
        printf ("inet_pton failed\n");
        return -1;
    }

    int result;
    int flag = 1;
    result = setsockopt (sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof (int));
    if (result < 0) {
        printf ("set sock opt failed\n");
        return -1;
    }

    if (connect (sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
       printf ("connect failed\n");
       return -1;
    }

	return sockfd;
}


void CloseSocket (int socket)
{
	if (socket != -1) {
		close (socket);
		socket = -1;
	}
}


void PrintIdentity (int sockfd)
{
	char buffer[1024];
	char *identify = (char *)"*IDN?\n";
	int actual;
	send (sockfd, identify, strlen (identify), 0);
	actual = recv (sockfd, buffer, sizeof (buffer), 0);
	buffer[27] = 0;

	printf ("%s\n", buffer);
}


void ConfigureCurrentMeasurement (int sockfd)
{
	char *commands[] = {
		(char *)"CONF:CURR:DC\n",
		(char *)"CURR:DC:TERM 3\n",
		(char *)"CURR:DC:RANG:AUTO ON\n",
		(char *)"SAMP:COUN 1\n",
		(char *)""
	};

	int i = 0;
	while (strlen (commands[i]) != 0)  {
		send (sockfd, commands[i], strlen (commands[i]), 0);
		i++;
	}
}


void ConfigureVoltageMeasurement (int sockfd)
{
	char *commands[] = {
		(char *)"CONF:VOLT:DC\n",
		(char *)"VOLT:DC:RANG:AUTO ON\n",
		(char *)"SAMP:COUN 1\n",
		(char *)""
	};

	int i = 0;
	while (strlen (commands[i]) != 0)  {
		send (sockfd, commands[i], strlen (commands[i]), 0);
		i++;
	}
}


double GetMeasurement (int sockfd)
{
	double value;
	char buffer[1024];
	char *read = (char *)"READ?\n";
	int actual;
	send (sockfd, read, strlen (read), 0);
	actual = recv (sockfd, buffer, sizeof (buffer), 0);
	buffer[actual] = 0;

	sscanf (buffer, "%lf", &value);
	return value;
}
