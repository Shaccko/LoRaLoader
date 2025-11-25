#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

int main() {
	const char* dev = "/dev/spidev0.0";
	int fd = open(device, O_RDWR);


