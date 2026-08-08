// Translated from solution.cpp.

var f = cpp_array(1010);

var g = cpp_array(1010);

var s = cpp_array(1010);

var p = cpp_array(1010);

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&p[i]));
      i += 1;
    }
  }
  f[1] = 1;
  g[1] = 1;
  s[1] = 1;
  {
    var i = 2;
    while ((i <= n))
    {
      var x = ((((s[(i - 1)] - s[(p[i] - 1)]) + 1000000007)) % 1000000007);
      g[i] = ((((x + 1) + ((i - p[i])))) % 1000000007);
      f[i] = ((((g[i] + f[(i - 1)]) + 1)) % 1000000007);
      s[i] = (((s[(i - 1)] + g[i])) % 1000000007);
      i += 1;
    }
  }
  var ans = (((f[n] + 1)) % 1000000007);
  write(ans, "\n");
}
