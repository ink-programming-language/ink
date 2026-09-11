// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  while ((n != 0))
  {
    var s: dynamic = 1;
    var sum: dynamic = 0;
    var e: dynamic = 1;
    var o: dynamic = 0;
    while ((e <= n))
    {
      if ((sum < n))
      {
        sum += e;
        e += 1;
      } else if ((sum == n))
      {
        o += 1;
        sum += e;
        e += 1;
      } else
      {
        sum -= s;
        s += 1;
      }
    }
    write(o, "\n");
    read(n);
  }
  return 0;
}
