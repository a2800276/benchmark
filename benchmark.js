
// Fairly straightforward port of go's benchmark impl.
// see: http://golang.org/pkg/testing/
// todo: result formatting facilities ...



var Benchmark = function () {
}

function ns () {
  // this is for ease of porting, JS doesn't
  // support sub ms accurracy ...
  return new Date().getTime() * 1e6
}

function startTimer () {
  if (!this.start) {
    this.start = ns() 
  }
}

function stopTimer () {
  if (this.start > 0) {
    this.ns = ns() - this.start 
  }
  this.start = 0
}

function resetTimer() {
  if (this.start > 0) {
    this.start = ns()
  }
  this.ns = 0
}

// can be used from inside of the benchmark function
// to indicate the number of bytes processed in order
// for the information to be used in reports.
function setBytes(n) {
  this.bytes = n
}

function nsPerOp() {
  if (this.N <= 0) {
    return 0  
  }
  return this.ns / this.N
}

function runN(n) {
  this.N = n
  this.resetTimer()
  this.startTimer()
  this.F()
  this.stopTimer()
}


function roundDown10(n) {
  var tens = 0
  for ( ; n>10 ; ) {
    n /= 10
    ++tens
  }
  var result = 1
  for (var i =0; i<tens ; ++i) {
    result *= 10
  }
  return result
}

function roundUp(n) {
  var base = roundDown10(n)
  if (n < (2 * base)) {
    return 2*base
  } 
  if (n < (5 * base)) {
    return 5 * base
  }

  return 10 * base
}

function run () {
  var n = 1,
      min = Math.min,
      max = Math.max

  this.runN(n)
  var time = 1e9 // we want the benchmark to run at least this long (1s)
  for ( ; this.ns < time && n < 1e9; ) {
    var last = n
    if (this.nsPerOp() === 0) { 
      n = 1e9
    } else {
      n = time/this.nsPerOp()
    }
    n = max(min(n+n/2, 100*last), last+1)
    n = roundUp(n)
    this.runN(n)
  }
  return {
      N : this.N
    , ns: this.ns
    , bytes: this.bytes
  }
}


Benchmark.prototype.startTimer = startTimer
Benchmark.prototype.stopTimer  = stopTimer
Benchmark.prototype.resetTimer  = resetTimer
Benchmark.prototype.setBytes  = setBytes
Benchmark.prototype.nsPerOp  = nsPerOp
Benchmark.prototype.runN  = runN
Benchmark.prototype.run = run


function bench (f) {
  var b = new Benchmark()
  b.F = function () {
    for (b.i=0; b.i<b.N; ++b.i) {
      f(b)
    }
  }
  return b.run()
}

function f(b) {
  var d = new Date().toString()
  var dd = new Date(d)
}

exports.Benchmark = Benchmark
exports.bench = bench

console.log(bench(f))
