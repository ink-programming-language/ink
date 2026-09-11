// Translated from solution.cpp.

func abs(a: dynamic) -> dynamic
{
  return  ((a < 0)) ? (-a) : a;
}

func min(b: dynamic, a: dynamic) -> dynamic
{
  return  ((a < b)) ? a : b;
}

func max(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a < b)) ? b : a;
}

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 0;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    f = ((ch == cpp_char("-")));
    ch = getchar();
  }
  while (((ch <= cpp_char("9")) && (ch >= cpp_char("0"))))
  {
    x = ((((x << 1)) + ((x << 3))) + ((ch - cpp_char("0"))));
    ch = getchar();
  }
  return  (f) ? (-x) : x;
}

func write(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    x = abs(x);
    putchar(cpp_char("-"));
  }
  if ((x < 10))
  {
    putchar((x + 48));
    return;
  }
  write((x / 10));
  putchar(((x % 10) + 48));
}

var Maxn: dynamic = (1e5 + 11);

var t: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(Maxn);

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  read(t);
  while (cpp_update(t, "--"))
  {
    var cnt: dynamic = 1;
    read(n);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    sort(a, (a + n));
    {
      var i: dynamic = 1;
      while ((i < n))
      {
        if ((a[i] != a[(i - 1)]))
        {
          cnt += 1;
        }
        i += 1;
      }
    }
    write(cnt, "\n");
  }
}
