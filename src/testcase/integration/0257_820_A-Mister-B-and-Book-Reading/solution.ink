// Translated from solution.cpp.

func main() -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  var v0: dynamic = cpp_uninitialized();
  var v1: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  read(c, v0, v1, a, l);
  var ans: dynamic = 1;
  var t: dynamic = 0;
  var s: dynamic = v0;
  t += s;
  while ((t < c))
  {
    t -= l;
    if (((s + a) > v1))
    {
      s = v1;
    } else
    {
      s += a;
    }
    t += s;
    ans += 1;
  }
  write(ans, "\n");
}
