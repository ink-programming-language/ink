import std.CRT.io as CRT;

// Reads bytes from standard input; returns zero at end of input and -1 on a system error.
public func input(buffer: byte[]) -> int {
    return CRT.read(CRT.GetStdHandle(CRT.StandardHandle.input), buffer);
}

// Writes bytes to standard output; returns -1 on a system error.
public func output(buffer: const byte[]) -> int {
    return CRT.write(CRT.GetStdHandle(CRT.StandardHandle.output), buffer);
}
