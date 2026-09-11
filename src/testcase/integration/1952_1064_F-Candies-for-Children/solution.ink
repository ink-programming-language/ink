// Translated from solution.cpp.

func F() -> dynamic
{
  var F: dynamic = 1;
  var n: dynamic = 0;
  var ch: dynamic = cpp_uninitialized();
  while ((((cpp_assign(ch, "=", getchar())) != cpp_char("-")) && (((ch < cpp_char("0")) || (ch > cpp_char("9"))))))
  {
  }
   ((ch == cpp_char("-"))) ? cpp_assign(F, "=", 0) : cpp_assign(n, "=", (ch - cpp_char("0")));
  while ((((cpp_assign(ch, "=", getchar())) >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    n = (((n * 10) + ch) - cpp_char("0"));
  }
  return  (F) ? n : (-n);
}

func G() -> dynamic
{
  var F: dynamic = 1;
  var n: dynamic = 0;
  var ch: dynamic = cpp_uninitialized();
  while ((((cpp_assign(ch, "=", getchar())) != cpp_char("-")) && (((ch < cpp_char("0")) || (ch > cpp_char("9"))))))
  {
  }
   ((ch == cpp_char("-"))) ? cpp_assign(F, "=", 0) : cpp_assign(n, "=", (ch - cpp_char("0")));
  while ((((cpp_assign(ch, "=", getchar())) >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    n = (((n * 10) + ch) - cpp_char("0"));
  }
  return  (F) ? n : (-n);
}

func R(l: dynamic, r: dynamic) -> dynamic
{
  return (((((rand() << 15) | rand())) % (((r - l) + 1))) + l);
}

func main() -> dynamic
{
  var n: dynamic = io.G();
  var l: dynamic = (io.G() - 1);
  var r: dynamic = (io.G() - 1);
  var k: dynamic = io.G();
  var B: dynamic =  ((l <= r)) ? ((r - l) + 1) : (n - (((l - r) - 1)));
  var S: dynamic = (n - B);
  var ans: dynamic = -1;
  var fl: dynamic = 1;
  if (((((k - B)) / n) <= 22000000))
  {
    var u: dynamic = (((k - B)) / n);
    {
      var x: dynamic = 0;
      while ((x <= u))
      {
        var re: dynamic = ((k - B) - (x * n));
        if ((x == 0))
        {
          if (((re <= B) && (re >= 0)))
          {
            if ((fl || re))
            {
              ( ((ans < ((S + re)))) ? cpp_comma(cpp_assign(ans, "=", ((S + re))), 1) : 0);
            }
          }
        } else
        {
          var B1: dynamic = (re % x);
          var S1: dynamic = ((re / x) - B1);
          if (((B1 > B) || (S1 < 0)))
          {
            x += 1;
            continue;
          }
          if ((S1 <= S))
          {
            if ((fl || B1))
            {
              ( ((ans < ((B1 + S1)))) ? cpp_comma(cpp_assign(ans, "=", ((B1 + S1))), 1) : 0);
            }
          } else
          {
            var T: dynamic = (S1 - S);
            var ex: dynamic = (((T + x)) / ((x + 1)));
            B1 += (ex * x);
            S1 -= (ex * ((x + 1)));
            if (((S1 >= 0) && (B1 <= B)))
            {
              if ((fl || B1))
              {
                ( ((ans < ((B1 + S1)))) ? cpp_comma(cpp_assign(ans, "=", ((B1 + S1))), 1) : 0);
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
      var S1: dynamic = 0;
      while ((S1 <= S))
      {
        {
          var B1: dynamic = 0;
          while ((B1 <= B))
          {
            var y: dynamic = ((k - B1) - B);
            var a: dynamic = ((S1 + B1) + n);
            if ((((y == 0) && (a == 0)) || ((a && ((y % a) == 0)) && ((y / a) >= 0))))
            {
              if ((fl || B1))
              {
                ( ((ans < ((S1 + B1)))) ? cpp_comma(cpp_assign(ans, "=", ((S1 + B1))), 1) : 0);
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
