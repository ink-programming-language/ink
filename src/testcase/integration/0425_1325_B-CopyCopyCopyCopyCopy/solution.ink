// Translated from solution.cpp.

func abs(a: dynamic)
{
  return if ((a < 0)) (-a) else a;
}

func min(b: dynamic, a: dynamic)
{
  return if ((a < b)) a else b;
}

func max(a: dynamic, b: dynamic)
{
  return if ((a < b)) b else a;
}

func read()
{
  var x = 0;
  var f = 0;
  var ch = getchar();
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
  return if (f) (-x) else x;
}

func write(x: dynamic)
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

var Maxn = (1e5 + 11);

var t: dynamic;

var n: dynamic;

var a = cpp_array(Maxn);

func main()
{
  ios.sync_with_stdio(false);
  read(t);
  while (cpp_update(t, "--"))
  {
    var cnt = 1;
    read(n);
    {
      var i = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    sort(a, (a + n));
    {
      var i = 1;
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
