#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int lora_transmit(void);
int lora_receive(void);
int lora_read_reg(void);
int lora_write_reg(void);
int lora_init(void);
