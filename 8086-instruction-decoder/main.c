#include <stdint.h>
#include <stdio.h>

#define INSTR_MASK 0b11111100 // 6 bits
#define MOV_MASK 0b10001000 // 6 bits
#define D_MASK 0b10 // 1 bits
#define W_MASK 0b1 // 1 bits
#define MOD_MASK 0b11000000 // 6 bits
#define REG_MASK 0b00111000 // 6 bits
#define RM_MASK 0b00000111 // 6 bits

char *register_word_table[8] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
char *register_byte_table[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

int main(int argc, char **argv) {
	FILE *fptr;
	char instruction[2];

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

		char ismov = (instruction[0] & INSTR_MASK) == MOV_MASK;

		if (ismov) {
			char d_field = instruction[0] & D_MASK;
			char w_field = instruction[0] & W_MASK;
			char mod_field = instruction[1] & MOD_MASK;
			char reg_field = (instruction[1] & REG_MASK) >> 3;
			char rm_field = instruction[1] & RM_MASK;
			char *reg;
			char *rm;
			if (d_field) {
				reg = w_field ? register_word_table[reg_field] : register_byte_table[reg_field];
				rm = w_field ? register_word_table[rm_field] : register_byte_table[rm_field];
			} else {
				reg = w_field ? register_word_table[rm_field] : register_byte_table[rm_field];
				rm = w_field ? register_word_table[reg_field] : register_byte_table[reg_field];
			}

			printf("mov %s, %s", reg, rm);
		}
	}

	return 0;
}
