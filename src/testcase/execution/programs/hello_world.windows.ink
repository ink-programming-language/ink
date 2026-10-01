extern "C" func _write(Fd: i32, Buffer: *u8, Count: u32): i32;

func main(): i32
{
  _write(1, "hello, world\n", 13);
  return 0;
}
