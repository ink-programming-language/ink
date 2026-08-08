// Translated from solution.cpp.

var N = 1002;

func apn(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func get_c()
{
  var buf = cpp_array(20000);
  var h: dynamic;
  var t: dynamic;
  if ((h == t))
  {
    t = ((cpp_assign(h, "=", buf)) + fread(buf, 1, 20000, stdin));
  }
  return if ((h == t)) EOF else (*cpp_update(h, "++"));
}

func nxi()
{
  var x = 0;
  var c: dynamic;
  while ((((cpp_assign(c, "=", get_c())) > cpp_char("9")) || (c < cpp_char("0"))))
  {
  }
  while (cpp_comma(cpp_assign(x, "=", (((x * 10) + c) - 48)), (((cpp_assign(c, "=", get_c())) >= cpp_char("0")) && (c <= cpp_char("9")))))
  {
  }
  return x;
}

func main()
{
  var fa = cpp_array(N);
  var hx = cpp_array(N);
  var dp = cpp_array(5002, N);
  var n = nxi();
  {
    var i = 2;
    while ((i <= n))
    {
      fa[i] = nxi();
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      hx[i] = nxi();
      i += 1;
    }
  }
  {
    var x = n;
    while (x)
    {
      var y = fa[x];
      {
        var i = hx[y];
        while ((i >= 0))
        {
          var p = dp[y][i];
          dp[y][i] = 1e5;
          if ((hx[x] <= i))
          {
            dp[y][i] = ((if (hx[x]) dp[y][(i - hx[x])] else p) + dp[x][hx[x]]);
          }
          if ((dp[x][hx[x]] <= i))
          {
            apn(dp[y][i], ((if (dp[x][hx[x]]) dp[y][(i - dp[x][hx[x]])] else p) + hx[x]));
          }
          i -= 1;
        }
      }
      x -= 1;
    }
  }
  puts(if ((dp[1][hx[1]] >= 1e5)) "IMPOSSIBLE" else "POSSIBLE");
  return 0;
}
