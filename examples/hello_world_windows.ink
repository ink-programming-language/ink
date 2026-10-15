// Windows x64: GetStdHandle and WriteFile are exported by Kernel32.dll.
// Current main supports parsing this example; native execution is not implemented.
import "C" func GetStdHandle(StandardHandle: u32): *void;
import "C" func WriteFile(File: *void, Buffer: *u8, BytesToWrite: u32, BytesWritten: *u32, Overlapped: *void): i32;

func main(): i32
{
  // STD_OUTPUT_HANDLE is the u32 representation of -11.
  const StdOutputHandle: u32 = 0xFFFFFFF5;
  var Output = GetStdHandle(StdOutputHandle);
  var Written: u32 = 0;

  // The message contains 12 ASCII bytes followed by CRLF, for 14 bytes total.
  if (WriteFile(Output, "hello, world\r\n", 14, &Written, null) == 0)
  {
    return 1;
  }

  if (Written != 14)
  {
    return 1;
  }

  return 0;
}
