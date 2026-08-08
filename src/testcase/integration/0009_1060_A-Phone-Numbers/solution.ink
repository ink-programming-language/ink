// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

var LINF = 0x3f3f3f3f3f3f3f3f;

var t: dynamic;

var n: dynamic;

var a: dynamic;

var b: dynamic;

var cnt1: dynamic;

var cnt2: dynamic;

var s: dynamic;

func main()
{
  read(n);
  read(s);
  cnt1 = (n / 11);
  {
    var i = 0;
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
