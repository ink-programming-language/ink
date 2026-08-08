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

var q = cpp_array((100000 + 5));

var act = cpp_array((100000 + 5));

var pw = cpp_array((100000 + 5));

var inv = cpp_array((100000 + 5));

var top = 0;

var n: dynamic;

var m: dynamic;

var a = cpp_array((100000 + 5));

var Ans = cpp_array((100000 + 5));

var s = cpp_array((100000 + 5));

var p = cpp_array((100000 + 5));

var v = cpp_array((100000 + 5));

func Calc(l: dynamic, r: dynamic)
{
  return (((1 * (((s[r] - s[(l - 1)]) + 1000000007))) * inv[l]) % 1000000007);
}

func add(x: dynamic, y: dynamic)
{
  x = min((x + y), cpp_cast(4e18));
}

func pow(x: dynamic, t: dynamic)
{
  {
    var i = 1;
    while (((i <= t) && (x < cpp_cast(4e18))))
    {
      x = min(cpp_cast(4e18), (x * 2));
      i += 1;
    }
  }
}

func main()
{
  n = read();
  m = read();
  pw[0] = cpp_assign(inv[0], "=", 1);
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] = read();
      pw[i] = ((2 * pw[(i - 1)]) % 1000000007);
      inv[i] = ((((1 * inv[(i - 1)]) * ((1000000007 + 1))) / 2) % 1000000007);
      s[i] = (((s[(i - 1)] + (((a[i] + (2 * 1000000007))) * pw[i]))) % 1000000007);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      var l = read();
      var r = read();
      v[r].push_back(make_pair(l, i));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var x = a[i];
      var l = i;
      var r = i;
      {
        while ((top && (x >= 0)))
        {
          l = q[top].first;
          if ((x > 0))
          {
            pow(x, ((q[top].second - q[top].first) + 1));
          }
          add(x, act[top]);
          top -= 1;
        }
      }
      q[cpp_update(top, "++")] = make_pair(l, r);
      p[top] = (((p[(top - 1)] + Calc(l, r))) % 1000000007);
      act[top] = x;
      {
        var j = 0;
        while ((j < v[i].size()))
        {
          var pos = ((upper_bound((q + 1), ((q + top) + 1), make_pair(v[i][j].first, (n + 1))) - q) - 1);
          var ans = ((((((1 * p[top]) - p[pos]) + Calc((v[i][j].first + 1), q[pos].second)) + 1000000007)) % 1000000007);
          Ans[v[i][j].second] = (((((2 * ans) + a[v[i][j].first]) + (2 * 1000000007))) % 1000000007);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      printf("%d\n", Ans[i]);
      i += 1;
    }
  }
  return 0;
}
