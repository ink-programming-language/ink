// Linux x64 ABI; redirect arrays_read.input containing exactly ABCDEF to stdin.
import "C" func read(Fd: i32, Buffer: *u8, Count: u64): i64;
import "C" func puts(Text: *u8): i32;

func check(Condition: bool): bool
{
  if (Condition)
  {
    puts("PASS");
    return true;
  }
  puts("FAIL");
  return false;
}

func main(): i32
{
  // Repeated initialization establishes sentinels around the writable input span.
  puts("ArrayRead.initialized_repeat");
  var Buffer: [u8; 8] = [85; 8];
  if (!check(Buffer[0] == 85 && Buffer[7] == 85))
  {
    return 1;
  }

  // Read the first three input bytes through an interior managed element address.
  puts("ArrayRead.interior_offset");
  var First = read(0, &Buffer[1], 3);
  if (!check(First == 3 && Buffer[0] == 85 && Buffer[1] == 65 && Buffer[2] == 66 && Buffer[3] == 67 && Buffer[4] == 85))
  {
    return 1;
  }
  var Before = Buffer;

  // A zero-byte read changes no element and must not consume the remaining DEF.
  puts("ArrayRead.zero_count");
  var Empty = read(0, &Buffer[0], 0);
  if (!check(Empty == 0 && Buffer[0] == 85 && Buffer[1] == 65 && Buffer[3] == 67 && Buffer[4] == 85))
  {
    return 1;
  }

  // Request five bytes when only DEF remains; preserve both unread tail sentinels.
  puts("ArrayRead.short_read");
  var Short = read(0, &Buffer[3], 5);
  if (!check(Short == 3 && Buffer[0] == 85 && Buffer[1] == 65 && Buffer[2] == 66 && Buffer[3] == 68 && Buffer[4] == 69 && Buffer[5] == 70 && Buffer[6] == 85 && Buffer[7] == 85))
  {
    return 1;
  }

  // A whole-array copy taken before the C write retains its original element values.
  puts("ArrayRead.value_copy");
  if (!check(Before[0] == 85 && Before[1] == 65 && Before[2] == 66 && Before[3] == 67 && Before[4] == 85 && Before[5] == 85 && Before[7] == 85))
  {
    return 1;
  }

  // Ink element assignment remains visible after the same storage was written by C.
  puts("ArrayRead.element_write");
  Buffer[4] = 90;
  if (!check(Buffer[3] == 68 && Buffer[4] == 90 && Buffer[5] == 70 && Before[4] == 85))
  {
    return 1;
  }

  // Changing a second array copy never aliases either the live buffer or an earlier copy.
  puts("ArrayRead.copy_independence");
  var Copy = Buffer;
  Copy[1] = 88;
  if (!check(Copy[1] == 88 && Buffer[1] == 65 && Before[1] == 65 && Copy[4] == 90))
  {
    return 1;
  }

  // EOF returns zero without overwriting the sentinels, input bytes or later Ink edits.
  puts("ArrayRead.eof");
  var End = read(0, &Buffer[0], 8);
  if (!check(End == 0 && Buffer[0] == 85 && Buffer[1] == 65 && Buffer[2] == 66 && Buffer[3] == 68 && Buffer[4] == 90 && Buffer[5] == 70 && Buffer[6] == 85 && Buffer[7] == 85))
  {
    return 1;
  }
  return 0;
}
