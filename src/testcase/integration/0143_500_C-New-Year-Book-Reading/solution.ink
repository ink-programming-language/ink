// Translated from solution.cpp.

var N: dynamic = 1005;

func read() -> dynamic
{
  var x: dynamic = 0;
  var w: dynamic = 1;
  var ch: dynamic = 0;
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    ch = getchar();
    if ((ch == cpp_char("-")))
    {
      w = -1;
    }
  }
  while (((ch <= cpp_char("9")) && (ch >= cpp_char("0"))))
  {
    x = (((((x << 1)) + ((x << 3))) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * w);
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var tot: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(N);

var Ans: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var w: dynamic = cpp_array(N);

var ans: dynamic = cpp_array(N);

var vis: dynamic = cpp_array(N);

func main() -> dynamic
{
  n = read();
  m = read();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      w[i] = read();
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      a[i] = read();
      memset(vis, 0, cpp_sizeof((vis)));
      {
        var j: dynamic = (i - 1);
        while (j)
        {
          if ((a[i] == a[j]))
          {
            break;
          }
          if (vis[a[j]])
          {
            j -= 1;
            continue;
          }
          vis[a[j]] = 1;
          Ans += w[a[j]];
          j -= 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", Ans);
  return 0;
}
