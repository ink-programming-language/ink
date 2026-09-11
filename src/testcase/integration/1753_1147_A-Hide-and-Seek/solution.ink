// Translated from solution.cpp.

func read() -> dynamic
{
  var xx: dynamic = 0;
  var ff: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch > cpp_char("9")) || (ch < cpp_char("0"))))
  {
    if ((ch == cpp_char("-")))
    {
      ff = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    xx = (((xx * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (xx * ff);
}

func READ() -> dynamic
{
  var xx: dynamic = 0;
  var ff: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch > cpp_char("9")) || (ch < cpp_char("0"))))
  {
    if ((ch == cpp_char("-")))
    {
      ff = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    xx = (((xx * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (xx * ff);
}

func one() -> dynamic
{
  var ch: dynamic = getchar();
  while (((ch == cpp_char(" ")) || (ch == cpp_char("\n"))))
  {
    ch = getchar();
  }
  return ch;
}

var maxn: dynamic = 100010;

var N: dynamic = cpp_uninitialized();

var K: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

var first: dynamic = cpp_array(maxn);

var last: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  N = read();
  K = read();
  {
    var i: dynamic = 1;
    while ((i <= K))
    {
      a[i] = read();
      if ((!first[a[i]]))
      {
        first[a[i]] = i;
      }
      last[a[i]] = i;
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      if ((!first[i]))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      if ((((first[i] > last[(i + 1)]) || (!first[i])) || (!last[(i + 1)])))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = N;
    while ((i > 1))
    {
      if ((((first[i] > last[(i - 1)]) || (!first[i])) || (!last[(i - 1)])))
      {
        ans += 1;
      }
      i -= 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
