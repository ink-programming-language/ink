// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var sum: dynamic = 0;
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(f);
      sum += f;
      i += 1;
    }
  }
  var count: dynamic = 0;
  var demo: dynamic = sum;
  {
    var j: dynamic = 1;
    while ((j <= 5))
    {
      demo += 1;
      if (((demo % ((n + 1))) != 1))
      {
        count += 1;
      }
      j += 1;
    }
  }
  write(count);
  return 0;
}
