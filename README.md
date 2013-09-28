bf
==

This is an optimizing compiler for brainfuck. It's mostly a toy
for playing with optimization and code generation.

Usage
-----
bf reads a brainfuck program from stdin and writes AT&T assembler to stdout.

`bf < program.bf > program.s`

The assembler can be assembled with gcc (and possibly others, but that's completely untested).

`gcc -nostdlib program.s -o program`

The `-nostdlib` flag is required since `_start` is defined directly in the assembler and the
stdlib typically tries to define its own.


Optimizations
-------------

* Combining adjacent operations
  * For example, ++-++-++- becomes +3.
* Making "fixed loops" run in a single pass.
  * A "fixed loop" is a loop without IO where the cursor is at the same position for each instruction for each iteration of the loop.
  * This means operations can be easily combined across loop iterations.
  * For example, `+[>+<+]` would become `+ > +255 < +255`.
* Removing cursor movements
  * Instead, instructions can be encoded with offsets.
  * For example, `+>>>>+<<<<` would become `+(0) +(4)`
