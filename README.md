This is a small utility to display a prompt much like the fish default.

### Features

- makes exactly 3 syscalls in `main()`: a single `getcwd`, `geteuid` and `write` each.
- Hardcoded for `TERM=xterm-256color` 
- When in `$PREFIX`, abbreviates it to `$P/...`.


### Usage

- compile & install:

```sh
musl-gcc -static fish_prompt.c -o fish_prompt

cp fish_prompt $SOMEWHERE/bin/
```



- use as your prompt:

```fish
# ~/.config/fish/functions/fish_prompt.fish

function fish_prompt --description "Write out the prompt"
	# fish doesn't export these
	command fish_prompt $status $CMD_DURATION
end
```

### libunistring

We use code verbatim from [libunistring-1.0](https://www.gnu.org/software/libunistring/), which
is licensed GPL3 and LGPL3. This is done to enable fully static linking with musl: some platforms
(Android) have a huge overhead otherwise.

How this interacts with the EUPL is left as an excercise for the reader.




