// Windows x64 C ABI example; see printf.md for the string/ABI assumptions.
// These byte-pointer bindings describe C void* and char* buffers at the ABI boundary.
extern "C" func malloc(Size: uint64) -> ptr uint8;
extern "C" func free(Buffer: ptr uint8) -> void;
extern "C" func snprintf(Buffer: ptr uint8, Capacity: uint64, Format: const ptr uint8, ...) -> int32;
extern "C" func _write(Descriptor: int32, Buffer: const ptr uint8, Count: uint32) -> int32;

// Called only by comptime: inspect types without reading the runtime argument values.
// A set bit means float/double (%g); a clear bit means int32 (%d). At most 64 arguments.
func printfFloatKinds[Ts: type...]() -> (bool, uint64)
{
    if (Ts.length > 64)
    {
        return (false, 0);
    }

    var Kinds: uint64 = 0;
    for (var Index: uint64 = 0; Index < Ts.length; Index += 1)
    {
        if (Ts[Index] == float || Ts[Index] == double)
        {
            Kinds |= uint64(1) << Index;
        }
        else if (Ts[Index] != int32)
        {
            return (false, 0);
        }
    }
    return (true, Kinds);
}

// Format is a NUL-terminated byte string. Return the output byte count, or -1 on failure.
// Supported fields: {}; escaped braces: {{ and }}. A literal % remains literal.
func printf[Ts: type...](Format: const ptr uint8, Args: Ts...) -> int32
{
    let (TypesSupported, FloatKinds): (bool, uint64) = comptime printfFloatKinds::[Ts...]();
    if (!TypesSupported || Format == null)
    {
        return -1;
    }

    var FormatLength: uint64 = 0;
    while (Format[FormatLength] != 0)
    {
        if (FormatLength == 2147483647)
        {
            return -1;
        }
        FormatLength += 1;
    }

    // Escaping every % doubles the length at most; reserve one more byte for NUL.
    let CFormat: ptr uint8 = malloc(FormatLength * 2 + 1);
    if (CFormat == null)
    {
        return -1;
    }
    defer free(CFormat);

    var InputIndex: uint64 = 0;
    var OutputIndex: uint64 = 0;
    var ArgumentIndex: uint64 = 0;
    while (InputIndex < FormatLength)
    {
        let Byte: uint8 = Format[InputIndex];
        if (Byte == 123)
        {
            if (Format[InputIndex + 1] == 123)
            {
                CFormat[OutputIndex] = 123;
                OutputIndex += 1;
                InputIndex += 2;
            }
            else if (Format[InputIndex + 1] == 125)
            {
                if (ArgumentIndex >= Ts.length)
                {
                    return -1;
                }
                CFormat[OutputIndex] = 37;
                CFormat[OutputIndex + 1] = ((FloatKinds >> ArgumentIndex) & 1) != 0 ? 103 : 100;
                OutputIndex += 2;
                InputIndex += 2;
                ArgumentIndex += 1;
            }
            else
            {
                return -1;
            }
        }
        else if (Byte == 125)
        {
            if (Format[InputIndex + 1] != 125)
            {
                return -1;
            }
            CFormat[OutputIndex] = 125;
            OutputIndex += 1;
            InputIndex += 2;
        }
        else
        {
            CFormat[OutputIndex] = Byte;
            OutputIndex += 1;
            if (Byte == 37)
            {
                CFormat[OutputIndex] = 37;
                OutputIndex += 1;
            }
            InputIndex += 1;
        }
    }
    CFormat[OutputIndex] = 0;
    if (ArgumentIndex != Ts.length)
    {
        return -1;
    }

    // Fixed-pack forwarding; the external C variadic call promotes float to double.
    // Args holds this invocation's values: CallOther is not evaluated again here.
    let Required: int32 = snprintf(null, 0, CFormat, Args...);
    if (Required < 0)
    {
        return -1;
    }
    let Capacity: uint64 = uint64(Required) + 1;
    let Output: ptr uint8 = malloc(Capacity);
    if (Output == null)
    {
        return -1;
    }
    defer free(Output);
    let Formatted: int32 = snprintf(Output, Capacity, CFormat, Args...);
    if (Formatted != Required)
    {
        return -1;
    }

    var Written: uint32 = 0;
    while (Written < uint32(Required))
    {
        let Count: int32 = _write(1, Output + Written, uint32(Required) - Written);
        if (Count <= 0)
        {
            return -1;
        }
        Written += uint32(Count);
    }
    return Required;
}

func CallOther(Left: int32, Right: int32) -> float
{
    return float(Left) / float(Right);
}

func main() -> int32
{
    let Written: int32 = printf::[int32, float]("hello, {} world {}", 1, CallOther(1,2));
    return Written < 0 ? 1 : 0;
}
