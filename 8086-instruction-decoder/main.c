#include <stdint.h>
#include <stdio.h>

#define INSTR_MASK 0b11111100
#define MOV_INSTR 0b10001000
#define IMREG_INSTR 0b10110000
#define D_MASK 0b10
#define MOD_MASK 0b11000000
#define RM_MASK 0b00000111

char *register_word_table[8] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
char *register_byte_table[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

int main(int argc, char **argv) {
	FILE *fptr;
	char instruction[4];

	if (argc == 1) {
		printf("please specify a file!\n");
		return 1;
	}

	char *file_name = argv[1];

	fptr = fopen(file_name, "rb");

	if (!fptr) {
		printf("couldn't open file!\n");
		return 1;
	}

	printf("bits 16\n");

	while (!feof(fptr)) {
		instruction[0] = fgetc(fptr);
		instruction[1] = fgetc(fptr);

		if (instruction[0] == -1 || instruction[1] == -1) {
			break;
		}

		printf("\n");

		unsigned char cmd = (instruction[0] & INSTR_MASK);

		if (cmd == MOV_INSTR) {
			char d_field = instruction[0] & D_MASK;
			char w_field = instruction[0] & 0b1;
			char mod_field = (instruction[1] & MOD_MASK) >> 6;
			char reg_field = (instruction[1] & 0b00111000) >> 3;
			char rm_field = instruction[1] & RM_MASK;
			char *dest;
			char *src;
			switch (mod_field) {
			case 0b11: {
				if (d_field) {
					dest = w_field ? register_word_table[reg_field]
								   : register_byte_table[reg_field];
					src = w_field ? register_word_table[rm_field]
								  : register_byte_table[rm_field];
				} else {
					dest = w_field ? register_word_table[rm_field]
								   : register_byte_table[rm_field];
					src = w_field ? register_word_table[reg_field]
								  : register_byte_table[reg_field];
				}
			} break;
			// Memory mode - 8 bit displacement follows
			case 0b01: {
				instruction[2] = fgetc(fptr);
			} break;
			case 0b10: {
				instruction[2] = fgetc(fptr);
				instruction[3] = fgetc(fptr);
			} break;
			}

			printf("mov %s, %s", dest, src);
		} else {
			// Immediate cmd is only 4 bits - 0b1011
			cmd = cmd & 0b11110000;

			if (cmd == IMREG_INSTR) {
				char w_field = instruction[0] & 0b1000;
				char reg_field = instruction[0] & 0b111;

				char *dest = w_field ? register_word_table[reg_field]
							   : register_byte_table[reg_field];
				int16_t imval;
				if (w_field) {
					instruction[2] = fgetc(fptr);

					unsigned char low = instruction[1];
					int16_t high = instruction[2] << 8;
					imval = low | high;
				} else {
					imval = instruction[1];
				}
				printf("mov %s, %d", dest, imval);
			}
		}
	}

	return 0;
}
