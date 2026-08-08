// Translated from solution.cpp.

var N = 1005;

func read()
{
  var x = 0;
  var w = 1;
  var ch = 0;
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

var n: dynamic;

var m: dynamic;

var tot: dynamic;

var s = cpp_array(N);

var Ans: dynamic;

var a = cpp_array(N);

var w = cpp_array(N);

var ans = cpp_array(N);

var vis = cpp_array(N);

func main()
{
  n = read();
  m = read();
  {
    var i = 1;
    while ((i <= n))
    {
      w[i] = read();
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      a[i] = read();
      memset(vis, 0, cpp_sizeof((vis)));
      {
        var j = (i - 1);
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
