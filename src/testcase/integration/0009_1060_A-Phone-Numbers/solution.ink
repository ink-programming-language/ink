// Translated from solution.cpp.

var INF: dynamic = 0x3f3f3f3f;

var LINF: dynamic = 0x3f3f3f3f3f3f3f3f;

var t: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var cnt1: dynamic = cpp_uninitialized();

var cnt2: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  read(s);
  cnt1 = (n / 11);
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      if ((s[i] == cpp_char("8")))
      {
        cnt2 += 1;
      }
      i += 1;
    }
  }
  write(min(cnt1, cnt2), "\n");
  return 0;
}
