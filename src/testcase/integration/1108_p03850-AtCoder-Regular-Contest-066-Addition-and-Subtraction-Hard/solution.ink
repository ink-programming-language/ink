// Translated from solution.cpp.

var N = (2e5 + 5);

var a = cpp_array(N);

var f = cpp_array(3, N);

func main()
{
  var n: dynamic;
  scanf("%d%d", (&n), (&a[1]));
  {
    var i = 2;
    while ((i <= n))
    {
      var op = cpp_array(2);
      scanf("%s%d", op, (&a[i]));
      a[i] = (if ((op[0] == cpp_char("+"))) a[i] else (-a[i]));
      i += 1;
    }
  }
  f[1][0] = a[1];
  f[1][1] = cpp_assign(f[1][2], "=", (-1 << 60));
  {
    var i = 2;
    while ((i <= n))
    {
      f[i][0] = (max(f[(i - 1)][0], max(f[(i - 1)][1], f[(i - 1)][2])) + a[i]);
      f[i][1] = (max(f[(i - 1)][1], f[(i - 1)][2]) - a[i]);
      f[i][2] = (f[(i - 1)][2] + a[i]);
      if ((a[i] < 0))
      {
        f[i][1] = max(f[i][1], (f[(i - 1)][0] + a[i]));
        f[i][1] = max(f[i][1], (f[(i - 1)][1] + a[i]));
        f[i][1] = max(f[i][1], (f[(i - 1)][2] + a[i]));
        f[i][2] = max(f[i][2], (f[(i - 1)][1] - a[i]));
        f[i][2] = max(f[i][2], (f[(i - 1)][2] - a[i]));
      }
      i += 1;
    }
  }
  printf("%lld", max(f[n][0], max(f[n][1], f[n][2])));
}
