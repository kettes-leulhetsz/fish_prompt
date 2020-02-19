#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>	// write

enum size_t { SZ = 32768 };
char out[SZ];
size_t written = 0;


const char cyellow[] = "\e[1m\e[33m"; 	// set_color yellow --bold 
const char cred[] = "\e[1m\e[31m";	// set_color red --bold
const char cgreen[] = "\e[1m\e[32m";	// set_color green --bold
const char cwhite[] = "\e[1m\e[37m";	// set_color white --bold
const char cnormal[] = "\e[30m\e(B\e[m";	// set_color normal

void print(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	written += vsnprintf(out + written, SZ - written, fmt, args); 
	va_end(args);
	if (written >= SZ - 1) {
		fprintf(stderr, "Buffer too small! How did you do that?!");
		exit(127);
	}
}

void username() {
	const char nouser[] = "<no username>";
	char* user = getenv("USER");
	print("%s%s ", cnormal, user ? user : nouser); 
}

void status(const char *s) {
	int64_t status = strtoll(s, NULL, 10);
	if (status) print("%s%lld ", cred, status);
}

void cmd_duration(const char *s) {
	uint64_t i = strtoull(s, NULL, 10); // saturates

	if (i < 100) return;

	uint64_t ms = i % 1000;		i /= 1000;
	uint64_t seconds = i % 60;	i /= 60;
	uint64_t minutes = i % 60;	i /= 60;
	uint64_t hours = i % 24;	i /= 24;
	uint64_t days = i;
	int full = 0;
	
	print("%s[", cyellow);

	if (days > 0) {
		full = 1;
		print("%lud ", days);
	}

	if (full || hours > 0) {
		full = 1;
		print("%02lu:", hours);
	}
	
	if (full || minutes > 0) {
		print("%02lu:", minutes);
	}
	
	print("%02lu.%03lu] ", seconds, ms);

}

void cwd(int uid) {
	enum size_t { MAX = 32768 };
	static char _cwd[MAX]; // PATH_MAX is broken.
	char *cwd = getcwd(_cwd, MAX);
	
	if (!cwd) return;

	char *home = getenv("HOME");
	size_t homelen = 0;
	char *prefix = getenv("PREFIX");
	size_t prefixlen = 0;

	if(home) homelen = strlen(home);
	if(prefix) prefixlen = strlen(prefix);
	
	
	print(uid ? cgreen : cred);
	if(home && homelen && !strncmp(cwd, home, homelen)) {
		print("~");
		cwd += homelen;
	} else if (prefix && prefixlen && !strncmp(cwd, prefix, prefixlen)) {
		print("$P");
		cwd += prefixlen;
	}
	
	char *slash;
	while ((slash = strchr(cwd, '/'))) {
		print("/");
		while(*slash == '/') slash++;
		if(*slash) {
			print("%c", *slash);
			slash++;
		}
		cwd = slash;
	}
	if (cwd && *cwd) print("%s", cwd);
	
}

void prompt(int uid) {
	if (uid) {
		print("%s> %s", cwhite, cnormal);
	} else {
		print("%s # %s", cwhite, cnormal);
	}
}

int main(int argc, char *argv[]) {
//	username();
	if (argc > 1) status(argv[1]);
	if (argc > 2) cmd_duration(argv[2]);
	int uid = geteuid();
	cwd(uid);
	prompt(uid);
	
	write(1, out, written);
	return 0;
}
