This is a small utility to display a prompt much like the fish default.

### Features

- makes exactly 3 syscalls in `main()`: a single `getcwd`, `geteuid` and `write` each.
- Hardcoded for `TERM=xterm-256color` 
- When in `$PREFIX`, abbreviates it to `$P/...`.

### Usage

- compile & install:

```
clang -Os fish_prompt.c -o fish_prompt
musl-gcc -static -Os fish_prompt.c -o fish_prompt

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

If you just want a human-readable `$CMD_DURATION`, there's `mstotime.c`
