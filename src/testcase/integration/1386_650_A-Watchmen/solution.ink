// Translated from solution.cpp.

func rdInt(n: dynamic) -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  n = (x * f);
}

var mpx: dynamic = cpp_uninitialized();

var mpy: dynamic = cpp_uninitialized();

var st: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while ((~scanf("%d", (&n))))
  {
    mpx.clear();
    mpy.clear();
    st.clear();
    var an: dynamic = 0;
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        rdInt(x);
        rdInt(y);
        an += mpx[x];
        an += mpy[y];
        an -= st[make_pair(x, y)];
        mpx[x] += 1;
        mpy[y] += 1;
        st[make_pair(x, y)] += 1;
        i += 1;
      }
    }
    printf("%I64d\n", an);
  }
  return 0;
}
