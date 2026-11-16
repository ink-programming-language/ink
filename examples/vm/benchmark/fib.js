"use strict";

function fib(n) {
    if (n < 2) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

const n = process.argv.length > 2 ? Number(process.argv[2]) : 30;
const samples = process.argv.length > 3 ? Number(process.argv[3]) : 11;
const warmup = process.argv.length > 4 ? Number(process.argv[4]) : 3;
if (process.argv.length > 5 || !Number.isInteger(n) || n < 0 || n > 40 || !Number.isInteger(samples) || samples < 1 || samples > 1000 || !Number.isInteger(warmup) || warmup < 0 || warmup > 1000) {
    throw new Error("Expected n=0..40, samples=1..1000, warmup=0..1000");
}
let previous = 0;
let current = 1;
for (let index = 0; index < n; ++index) {
    [previous, current] = [current, previous + current];
}
let result = 0;
const timings = [];
for (let index = 0; index < warmup + samples; ++index) {
    const start = process.hrtime.bigint();
    result = fib(n);
    const end = process.hrtime.bigint();
    if (result !== previous) {
        throw new Error(`Expected ${previous}, got ${result}`);
    }
    if (index >= warmup) {
        timings.push(Number(end - start) / 1_000_000);
    }
}
console.log(JSON.stringify({ runtime: "JavaScript", n, result, samples_ms: timings, warmup }));
