// Compile-time calls must lower dependencies even though the entry module is analyzed first.
from adapter import answer;
from provider import foldedAnswer;

func main(): i32
{
  var Built: i32 = comptime answer();
  if (Built != 42)
  {
    return 1;
  }
  return comptime foldedAnswer();
}
