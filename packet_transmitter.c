#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <lora_raspi.h>
#include <packet_transmitter.h>

static long int get_tick(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	
	return (ts.tv_sec) * 1000 + ts.tv_nsec / 10000000L;
}

void generate_firmware_packet(struct packet* pkt, uint8_t* data_buf, size_t bytes_read) {
	pkt->header = PACKET_OTA_BYTE;
	memcpy(pkt->data, data_buf, bytes_read);
	/* Pad with 0xFF if last packed not 200 */
	if (bytes_read < CHUNK_SIZE) { 
		uint8_t pad_idx = (uint8_t) (CHUNK_SIZE - bytes_read);
		memset(&pkt->data[bytes_read], 0xFF, pad_idx);
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
		//lora_transmit(lora, tx, tx_len);
		do {
			read_reg(RegIrqFlags, &irq);
			usleep(1);
		} while ((irq & 0x40U) == 0 || (get_tick() - ack_timer) > 2000);
		//lora_receive(lora, &rx_buf);

		if (rx_buf == ACK_CODE || rx_buf == PKT_PASS || rx_buf == PKT_COMPLETE) {
			ack_status = 1;
			break;
		}
		printf("Error receiving ack, retrying...\n");
		err_count++;
	}

	return ack_status;
}


