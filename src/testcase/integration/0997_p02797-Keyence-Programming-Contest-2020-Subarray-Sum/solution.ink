// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, k, s);
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      write(s, cpp_char(" "));
      i += 1;
    }
  }
  if ((s != 1))
  {
    s -= 1;
  } else
  {
    s += 1;
  }
  {
    var i: dynamic = k;
    while ((i < n))
    {
      write(s, cpp_char(" "));
      i += 1;
    }
  }
}
