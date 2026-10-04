import "C" func puts(Text: *u8): i32;

class Buffer
{
  field Values: [i32; 2];
};

func read(Value: *Buffer, Index: i32): i32
{
  puts("before bounds failure");
  return Value->Values[Index];
}

func main(): i32
{
  var Value = Buffer([20, 22]);
  return read(&Value, 2);
}
