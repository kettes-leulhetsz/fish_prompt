/** This is a small utility to convert milliseconds into something more
 * human-readable, for use in the fish shell.
 *
 * Usage: `mstotime $CMD_DURATION`
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>	// write


#define SZ 64
char out[SZ];

int main(int argc, char* argv[]) {
	if (argc < 2) return 2;

	uint64_t i = strtoull(argv[1], NULL, 10); // saturates

	if (i < 20) return 0;

	uint64_t ms = i % 1000;		i /= 1000;
	uint64_t seconds = i % 60;	i /= 60;
	uint64_t minutes = i % 60;	i /= 60;
	uint64_t hours = i % 24;	i /= 24;
	uint64_t days = i;
	int full = 0;
	
	size_t written = snprintf(out, SZ, "[");

	if (days > 0) {
		full = 1;
		written += snprintf(out + written, SZ - written, 
			"%lud ", days);
	}

	if (full || hours > 0) {
		full = 1;
		written += snprintf(out + written, SZ - written,
			"%02lu:", hours);
	}
	
	if (full || minutes > 0) {
		written += snprintf(out + written, SZ - written,
			"%02lu:", minutes);
	}
	
	written += snprintf(out + written, SZ - written, 
			"%02lu.%03lu] ", seconds, ms);

	write(1, out, written);	
	return 0;
}
