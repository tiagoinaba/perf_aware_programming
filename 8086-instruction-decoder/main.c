#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INSTR_MASK 0b11111100
#define MOV_INSTR 0b10001000
#define IMREG_INSTR 0b10110000
#define D_MASK 0b10
#define MOD_MASK 0b11000000
#define RM_MASK 0b00000111

char *register_word_table[8] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
char *register_byte_table[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
char *rm_mem_mode[8] = {"bx + si", "bx + di", "bp + si", "bp + di",
						"si",	   "di",	  "bp",		 "bx"};

char *surround_string(char *to_surround, char *format) {
	char *buffer = (char *)malloc(strlen(to_surround) + strlen(format));
	sprintf(buffer, format, to_surround);
	return buffer;
}

int16_t two_byte_integer(char high, unsigned char low) {
	int16_t res = high << 8;
	return res | low;
}

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
				uint8_t displacement = instruction[2];
				if (d_field) {
					dest = w_field ? register_word_table[reg_field]
								   : register_byte_table[reg_field];
					src = rm_mem_mode[rm_field];
					char *temp = malloc(strlen(src) + 8);
					if (displacement > 0)
						sprintf(temp, "%s + %d", src, displacement);
					else strcpy(temp, src);
					src = surround_string(temp, "[%s]");
					free(temp);
				} else {
					dest = rm_mem_mode[rm_field];
					src = w_field ? register_word_table[reg_field]
								  : register_byte_table[reg_field];
					char *temp = malloc(strlen(dest) + 8);
					if (displacement > 0)
						sprintf(temp, "%s + %d", dest, displacement);
					else strcpy(temp, dest);
					dest = surround_string(temp, "[%s]");
					free(temp);
				}
			} break;
			// Memory mode - 16 bit displacement follows
			case 0b10: {
				instruction[2] = fgetc(fptr);
				instruction[3] = fgetc(fptr);
				uint16_t displacement =
					two_byte_integer(instruction[3], instruction[2]);
				if (d_field) {
					dest = w_field ? register_word_table[reg_field]
								   : register_byte_table[reg_field];
					src = rm_mem_mode[rm_field];
					char *temp = malloc(strlen(src) + 8);
					if (displacement > 0)
						sprintf(temp, "%s + %d", src, displacement);
					else strcpy(temp, src);
					src = surround_string(temp, "[%s]");
					free(temp);
				} else {
					dest = rm_mem_mode[rm_field];
					src = w_field ? register_word_table[reg_field]
								  : register_byte_table[reg_field];
					char *temp = malloc(strlen(dest) + 8);
					if (displacement > 0)
						sprintf(temp, "%s + %d", dest, displacement);
					else strcpy(temp, dest);
					dest = surround_string(temp, "[%s]");
					free(temp);
				}
			} break;
			// No displacement - except when R/M == 0b110, then 16 bit
			// displacement follows
			case 0b00: {
				// Direct address - 16 bit - TODO
				if (rm_field == 0b110) {
					instruction[2] = fgetc(fptr);
					instruction[3] = fgetc(fptr);
					break;
				}
				if (d_field) {
					dest = w_field ? register_word_table[reg_field]
								   : register_byte_table[reg_field];
					src = rm_mem_mode[rm_field];
					src = surround_string(src, "[%s]");
				} else {
					dest = rm_mem_mode[rm_field];
					src = w_field ? register_word_table[reg_field]
								  : register_byte_table[reg_field];
					dest = surround_string(dest, "[%s]");
				}
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
					char high = instruction[2];
					imval = two_byte_integer(high, low);
				} else {
					imval = instruction[1];
				}
				printf("mov %s, %d", dest, imval);
			}
		}
	}

	return 0;
}
