// Translated from solution.cpp.

func read() -> dynamic
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
  return (x * f);
}

var T: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(1000005);

var f: dynamic = cpp_array(1000005);

func main() -> dynamic
{
  T = read();
  n = read();
  b = read();
  t = read();
  t = min(t, min(n, b));
  while (cpp_update(T, "--"))
  {
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        a[i] = read();
        i += 1;
      }
    }
    memset(f, 0, cpp_sizeof((f)));
    {
      var i: dynamic = 1;
      while ((i <= t))
      {
        {
          var j: dynamic = 1;
          while ((j <= n))
          {
            f[a[j]] = (f[(a[j] - 1)] + 1);
            {
              var k: dynamic = (a[j] + 1);
              while (((k <= b) && (f[a[j]] > f[k])))
              {
                f[k] = f[a[j]];
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%d\n", f[b]);
  }
  return 0;
}
