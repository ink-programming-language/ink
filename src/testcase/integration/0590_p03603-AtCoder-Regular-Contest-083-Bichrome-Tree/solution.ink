// Translated from solution.cpp.

var N: dynamic = 1002;

func apn(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func get_c() -> dynamic
{
  var buf: dynamic = cpp_array(20000);
  var h: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  if ((h == t))
  {
    t = ((cpp_assign(h, "=", buf)) + fread(buf, 1, 20000, stdin));
  }
  return  ((h == t)) ? EOF : (*cpp_update(h, "++"));
}

func nxi() -> dynamic
{
  var x: dynamic = 0;
  var c: dynamic = cpp_uninitialized();
  while ((((cpp_assign(c, "=", get_c())) > cpp_char("9")) || (c < cpp_char("0"))))
  {
  }
  while (cpp_comma(cpp_assign(x, "=", (((x * 10) + c) - 48)), (((cpp_assign(c, "=", get_c())) >= cpp_char("0")) && (c <= cpp_char("9")))))
  {
  }
  return x;
}

func main() -> dynamic
{
  var fa: dynamic = cpp_array(N);
  var hx: dynamic = cpp_array(N);
  var dp: dynamic = cpp_array(5002, N);
  var n: dynamic = nxi();
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      fa[i] = nxi();
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      hx[i] = nxi();
      i += 1;
    }
  }
  {
    var x: dynamic = n;
    while (x)
    {
      var y: dynamic = fa[x];
      {
        var i: dynamic = hx[y];
        while ((i >= 0))
        {
          var p: dynamic = dp[y][i];
          dp[y][i] = 1e5;
          if ((hx[x] <= i))
          {
            dp[y][i] = (( (hx[x]) ? dp[y][(i - hx[x])] : p) + dp[x][hx[x]]);
          }
          if ((dp[x][hx[x]] <= i))
          {
            apn(dp[y][i], (( (dp[x][hx[x]]) ? dp[y][(i - dp[x][hx[x]])] : p) + hx[x]));
          }
          i -= 1;
        }
      }
      x -= 1;
    }
  }
  puts( ((dp[1][hx[1]] >= 1e5)) ? "IMPOSSIBLE" : "POSSIBLE");
  return 0;
}
