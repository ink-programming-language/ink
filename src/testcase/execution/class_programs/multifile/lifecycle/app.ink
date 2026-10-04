from provider import Counter;

func main(): i32
{
  var Result = 0;
  {
    const Value = Counter(&Result);
  }
  return Result;
}
