// Translated from solution.cpp.

func read()
{
  var ch = getchar();
  var ret = 0;
  var f = 1;
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  {
    while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
    {
      ret = (((ret * 10) + ch) - cpp_char("0"));
      ch = getchar();
    }
  }
  return (ret * f);
}

var INF = (1 << 30);

func main()
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  read(n, m, k);
  if ((((n * m) * 2) % k))
  {
    puts("NO");
    return 0;
  }
  k = (((n * m) * 2) / k);
  var t = sqrt(k);
  {
    var i = max((k / max(n, m)), 1);
    while ((i <= t))
    {
      if (((k % i) == 0))
      {
        var tmp = (k / i);
        if (((i <= n) && (tmp <= m)))
        {
          printf("YES\n%d %d\n%I64d %d\n%d %I64d\n", 0, 0, i, 0, 0, tmp);
          return 0;
        }
        if (((i <= m) && (tmp <= n)))
        {
          printf("YES\n%d %d\n%I64d %d\n%d %I64d\n", 0, 0, tmp, 0, 0, i);
          return 0;
        }
      }
      i += 1;
    }
  }
  puts("NO");
  return 0;
}
