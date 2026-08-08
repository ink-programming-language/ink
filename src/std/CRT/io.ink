public enum StandardHandle {
    input,
    output
}

private extern "C" func GetStdHandle(handle: u32) -> void*;
private extern "C" func ReadFile(file: void*, buffer: byte*, size: u32, transferred: u32*, overlapped: void*) -> i32;
private extern "C" func WriteFile(file: void*, buffer: const byte*, size: u32, transferred: u32*, overlapped: void*) -> i32;
private extern "C" func read(file: i32, buffer: byte*, size: ptrsize) -> int;
private extern "C" func write(file: i32, buffer: const byte*, size: ptrsize) -> int;

// Reads bytes from a platform handle; returns zero at end of input and -1 on a system error.
public func read(handle: ptrsize, buffer: byte[]) -> int {
    comptime if (target.os == Os.windows) {
        var size = buffer.length;
        if (size > cast::<ptrsize>(4294967295u32)) {
            size = cast::<ptrsize>(4294967295u32);
        }

        var transferred: u32 = 0;
        if (ReadFile(ptrcast::<void*>(handle), buffer.data, cast::<u32>(size), &transferred, null) == 0) {
            return -1;
        }
        return cast::<int>(transferred);
    } else {
        return read(cast::<i32>(handle), buffer.data, buffer.length);
    }
}

// Writes bytes to a platform handle; returns -1 on a system error.
public func write(handle: ptrsize, buffer: const byte[]) -> int {
    comptime if (target.os == Os.windows) {
        var size = buffer.length;
        if (size > cast::<ptrsize>(4294967295u32)) {
            size = cast::<ptrsize>(4294967295u32);
        }

        var transferred: u32 = 0;
        if (WriteFile(ptrcast::<void*>(handle), buffer.data, cast::<u32>(size), &transferred, null) == 0) {
            return -1;
        }
        return cast::<int>(transferred);
    } else {
        return write(cast::<i32>(handle), buffer.data, buffer.length);
    }
}

// Returns the selected platform standard handle.
public func GetStdHandle(handle: StandardHandle) -> ptrsize {
    comptime if (target.os == Os.windows) {
        const systemHandle: u32 = match (handle) {
            .input => 4294967286u32,
            .output => 4294967285u32,
        };
        return ptrcast::<ptrsize>(GetStdHandle(systemHandle));
    } else {
        return match (handle) {
            .input => cast::<ptrsize>(0),
            .output => cast::<ptrsize>(1),
        };
    }
}
