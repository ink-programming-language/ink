// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var o: dynamic = 0;
  var e: dynamic = 0;
  var o1: dynamic = 0;
  var e1: dynamic = 0;
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(n, m);
  {
    i = 0;
    while ((i < n))
    {
      read(a);
      if (((a % 2) == 1))
      {
        o += 1;
      } else
      {
        e += 1;
      }
      i += 1;
    }
  }
  {
    j = 0;
    while ((j < m))
    {
      read(b);
      if (((b % 2) == 1))
      {
        o1 += 1;
      } else
      {
        e1 += 1;
      }
      j += 1;
    }
  }
  c = min(o, e1);
  d = min(o1, e);
  write((c + d));
  return 0;
}
