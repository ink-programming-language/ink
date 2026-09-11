// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var tmp: dynamic = 1;

func main() -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  var x: dynamic = 0;
  read(n, k);
  while (((k != x) && (tmp < n)))
  {
    x += 1;
    tmp += (k - x);
  }
  if ((tmp >= n))
  {
    write(x, "\n");
  } else
  {
    write(-1, "\n");
  }
}
