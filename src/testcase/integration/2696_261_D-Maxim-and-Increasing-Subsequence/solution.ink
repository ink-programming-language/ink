// Translated from solution.cpp.

func read()
{
  var x = 0;
  var f = 1;
  var ch = getchar();
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

var T: dynamic;

var n: dynamic;

var b: dynamic;

var t: dynamic;

var a = cpp_array(1000005);

var f = cpp_array(1000005);

func main()
{
  T = read();
  n = read();
  b = read();
  t = read();
  t = min(t, min(n, b));
  while (cpp_update(T, "--"))
  {
    {
      var i = 1;
      while ((i <= n))
      {
        a[i] = read();
        i += 1;
      }
    }
    memset(f, 0, cpp_sizeof((f)));
    {
      var i = 1;
      while ((i <= t))
      {
        {
          var j = 1;
          while ((j <= n))
          {
            f[a[j]] = (f[(a[j] - 1)] + 1);
            {
              var k = (a[j] + 1);
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
