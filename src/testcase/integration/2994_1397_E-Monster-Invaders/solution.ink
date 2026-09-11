// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var c: dynamic = getchar();
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

func upd(a: dynamic, b: dynamic) -> dynamic
{
  a =  (((a < b))) ? a : b;
}

var N: dynamic = (1e6 + 5);

var f: dynamic = cpp_array(2, N);

var n: dynamic = cpp_uninitialized();

var r1: dynamic = cpp_uninitialized();

var r2: dynamic = cpp_uninitialized();

var r3: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

func main() -> dynamic
{
  n = read();
  r1 = read();
  r2 = read();
  r3 = read();
  d = read();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[cpp_update(i, "++")] = read();
    }
  }
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      f[i][0] = cpp_assign(f[i][1], "=", 1e18);
      i += 1;
    }
  }
  f[1][0] = (((1 * r1) * a[1]) + r3);
  f[1][1] = min((0 + r2), (((1 * r1) * a[1]) + r1));
  {
    var i: dynamic = 1;
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
