// Translated from solution.cpp.

func read()
{
  var x = 0;
  var c = getchar();
  {
    while ((!(((c > 47) && (c < 58)))))
    {
      c = getchar();
    }
  }
  {
    while ((((c > 47) && (c < 58))))
    {
      x = (((x * 10) + c) - 48);
      c = getchar();
    }
  }
  return x;
}

func upd(a: dynamic, b: dynamic)
{
  a = if (((a < b))) a else b;
}

var N = (1e6 + 5);

var f = cpp_array(2, N);

var n: dynamic;

var r1: dynamic;

var r2: dynamic;

var r3: dynamic;

var d: dynamic;

var a = cpp_array(N);

func main()
{
  n = read();
  r1 = read();
  r2 = read();
  r3 = read();
  d = read();
  {
    var i = 1;
    while ((i <= n))
    {
      a[cpp_update(i, "++")] = read();
    }
  }
  {
    var i = 2;
    while ((i <= n))
    {
      f[i][0] = cpp_assign(f[i][1], "=", 1e18);
      i += 1;
    }
  }
  f[1][0] = (((1 * r1) * a[1]) + r3);
  f[1][1] = min((0 + r2), (((1 * r1) * a[1]) + r1));
  {
    var i = 1;
    while ((i < n))
    {
      upd(f[(i + 1)][0], (((f[i][0] + d) + ((1 * r1) * a[(i + 1)])) + r3));
      upd(f[(i + 1)][1], ((f[i][0] + d) + min((0 + r2), (((1 * r1) * a[(i + 1)]) + r1))));
      upd(f[(i + 1)][0], (((((f[i][1] + d) + ((1 * r1) * a[(i + 1)])) + r3) + (2 * d)) + r1));
      upd(f[(i + 1)][0], (((((((f[i][1] + d) + ((1 * r1) * a[(i + 1)])) + r1) + d) + r1) + d) + r1));
      upd(f[(i + 1)][0], ((((((f[i][1] + d) + r2) + d) + r1) + d) + r1));
      upd(f[(i + 1)][1], (((((f[i][1] + d) + r2) + d) + r1) + d));
      upd(f[(i + 1)][1], ((((((f[i][1] + d) + ((1 * r1) * a[(i + 1)])) + r1) + d) + r1) + d));
      if ((i == (n - 1)))
      {
        upd(f[(i + 1)][0], (((((f[i][1] + d) + ((1 * r1) * a[(i + 1)])) + r3) + d) + r1));
      }
      i += 1;
    }
  }
  write(f[n][0], "\n");
}
