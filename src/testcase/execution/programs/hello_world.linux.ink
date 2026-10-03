import "C" func write(Fd: i32, Buffer: *u8, Count: u64): i64;

func main(): i32
{
  write(1, "hello, world\n", 13);
  return 0;
}
