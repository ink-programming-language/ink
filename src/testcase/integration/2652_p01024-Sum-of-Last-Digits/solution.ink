// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, m, k);
  n %= 10;
  m %= 4;
  var a: dynamic = n;
  var d: dynamic = cpp_array(4);
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      d[i] = a;
      a = ((a * n) % 10);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      ans += ((k / 4) * d[((m * i) % 4)]);
      if ((i < (k % 4)))
      {
        ans += d[((m * i) % 4)];
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
