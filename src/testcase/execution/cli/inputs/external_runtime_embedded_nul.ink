// Runtime C-string conversion rejects embedded NUL instead of silently truncating it.
extern "C" func puts(Text: *u8): i32;

func main(): i32
{
  puts("prefix\0suffix");
  return 0;
}
