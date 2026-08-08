// Translated from solution.cpp.

func F()
{
  var F = 1;
  var n = 0;
  var ch: dynamic;
  while ((((cpp_assign(ch, "=", getchar())) != cpp_char("-")) && (((ch < cpp_char("0")) || (ch > cpp_char("9"))))))
  {
  }
  if ((ch == cpp_char("-"))) cpp_assign(F, "=", 0) else cpp_assign(n, "=", (ch - cpp_char("0")));
  while ((((cpp_assign(ch, "=", getchar())) >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    n = (((n * 10) + ch) - cpp_char("0"));
  }
  return if (F) n else (-n);
}

func G()
{
  var F = 1;
  var n = 0;
  var ch: dynamic;
  while ((((cpp_assign(ch, "=", getchar())) != cpp_char("-")) && (((ch < cpp_char("0")) || (ch > cpp_char("9"))))))
  {
  }
  if ((ch == cpp_char("-"))) cpp_assign(F, "=", 0) else cpp_assign(n, "=", (ch - cpp_char("0")));
  while ((((cpp_assign(ch, "=", getchar())) >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    n = (((n * 10) + ch) - cpp_char("0"));
  }
  return if (F) n else (-n);
}

func R(l: dynamic, r: dynamic)
{
  return (((((rand() << 15) | rand())) % (((r - l) + 1))) + l);
}

func main()
{
  var n = io.G();
  var l = (io.G() - 1);
  var r = (io.G() - 1);
  var k = io.G();
  var B = if ((l <= r)) ((r - l) + 1) else (n - (((l - r) - 1)));
  var S = (n - B);
  var ans = -1;
  var fl = 1;
  if (((((k - B)) / n) <= 22000000))
  {
    var u = (((k - B)) / n);
    {
      var x = 0;
      while ((x <= u))
      {
        var re = ((k - B) - (x * n));
        if ((x == 0))
        {
          if (((re <= B) && (re >= 0)))
          {
            if ((fl || re))
            {
              (if ((ans < ((S + re)))) cpp_comma(cpp_assign(ans, "=", ((S + re))), 1) else 0);
            }
          }
        } else
        {
          var B1 = (re % x);
          var S1 = ((re / x) - B1);
          if (((B1 > B) || (S1 < 0)))
          {
            x += 1;
            continue;
          }
          if ((S1 <= S))
          {
            if ((fl || B1))
            {
              (if ((ans < ((B1 + S1)))) cpp_comma(cpp_assign(ans, "=", ((B1 + S1))), 1) else 0);
            }
          } else
          {
            var T = (S1 - S);
            var ex = (((T + x)) / ((x + 1)));
            B1 += (ex * x);
            S1 -= (ex * ((x + 1)));
            if (((S1 >= 0) && (B1 <= B)))
            {
              if ((fl || B1))
              {
                (if ((ans < ((B1 + S1)))) cpp_comma(cpp_assign(ans, "=", ((B1 + S1))), 1) else 0);
              }
            }
          }
        }
        x += 1;
      }
    }
  } else
  {
    {
      var S1 = 0;
      while ((S1 <= S))
      {
        {
          var B1 = 0;
          while ((B1 <= B))
          {
            var y = ((k - B1) - B);
            var a = ((S1 + B1) + n);
            if ((((y == 0) && (a == 0)) || ((a && ((y % a) == 0)) && ((y / a) >= 0))))
            {
              if ((fl || B1))
              {
                (if ((ans < ((S1 + B1)))) cpp_comma(cpp_assign(ans, "=", ((S1 + B1))), 1) else 0);
              }
            }
            B1 += 1;
          }
        }
        S1 += 1;
      }
    }
  }
  if (fl)
  {
    k = (k + 1);
    fl = 0;
    cpp_goto("goto start;");
  }
  printf("%lld\n", ans);
  return 0;
}
