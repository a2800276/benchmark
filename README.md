# Mini Benchmark for C

This is a trivial port of the Go benchmark functionality to C (as well
as Java and Javascript). This has been laying around in may toolbox for
a while, I've checked through the C code and it suffered a little from
bitrot. I've not checked the Java an Javscript versions, so proceed with
caution.

## Using

You can run a simple malloc vs. realloc benchmark using:

```
	$ ${CC} -DMAIN benchmark.c && ./a.out
```

Check the source or the Go [documentation](https://pkg.go.dev/testing)
for more discussion about usage. In short, you need to provide a
benchmark struct containing a benchmark function. The library sets the
`n` member of the struct and calls the benchmark function which is
expected to run the code you are benchmarking `n` times.

Declaring a benchmark is simple:

```

void bench (B * b) {
  for (i=0; i!=b->n; ++i) {
  	// benchmark goes here
  }
}

...

void main(void) {
	
  B b;
  b.name = "my name";
  b.benchmark = &bench;
  run(&b);
}

```

in case you care to measure throughput, use `set_bytes` to indicate the
number of data is processed in each iteration. In case a substantial
setup time is required before the actual benchmark loop, you can reset
the timer using `reset_time(B*b)`

