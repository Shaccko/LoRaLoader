#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <lora_raspi.h>
#include <packet_transmitter.h>

static uint8_t chunk_num = 1;

static inline uint8_t get_chunk_num(void) {
	return chunk_num;
}

static long int get_tick(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	
	return (ts.tv_sec) * 1000 + ts.tv_nsec / 10000000L;
}

void generate_firmware_packet(struct packet* pkt, uint8_t* data_buf, size_t bytes_read) {
	pkt->header = OTA_BYTE;
	pkt->chunk_size = (uint8_t)bytes_read;
	pkt->chunk_num = get_chunk_num();
	memcpy(pkt->data, data_buf, bytes_read);
	pkt->checksum = 0;

	size_t i;
	for (i = 0; i < bytes_read; i++) {
		pkt->checksum ^= data_buf[i];
	}
}

uint8_t send_tx_wait_ack(struct lora* lora, uint8_t* tx, size_t tx_len) {
	uint8_t irq, err_count = 0, ack_status = 0;
	uint8_t rx_buf;

	long int ack_timer;
	while (ack_status != 1) {
		ack_timer = get_tick();
		if (err_count >= 5) {
			ack_status = 0;
			break;
		}

		printf("Sent transmission\n");
		lora_transmit(lora, tx, tx_len);
		usleep(1);
		uint32_t counter = 0;
		do {
			lora_read_reg(RegIrqFlags, &irq);
			counter++;
		} while ((irq & 0x40U) == 0 || (get_tick() - ack_timer) > PACKET_TIMEOUT);
		lora_receive(lora, &rx_buf);
		printf("irq flag counter: %d\n", counter);

		if (rx_buf == ACK_CODE || rx_buf == PKT_PASS || rx_buf == PKT_COMPLETE) {
			printf("Got good ack\n");
			ack_status = 1;
			break;
		}
		printf("Error receiving ack, retrying...\n");
		printf("err_count: %d\n", err_count);
		err_count++;
	}

	return ack_status;
}

void increment_chunk_num(void) {
	chunk_num++;
}


