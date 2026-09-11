// Translated from solution.cpp.

var f: dynamic = cpp_array(1010);

var g: dynamic = cpp_array(1010);

var s: dynamic = cpp_array(1010);

var p: dynamic = cpp_array(1010);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
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
    var i: dynamic = 2;
    while ((i <= n))
    {
      var x: dynamic = ((((s[(i - 1)] - s[(p[i] - 1)]) + 1000000007)) % 1000000007);
      g[i] = ((((x + 1) + ((i - p[i])))) % 1000000007);
      f[i] = ((((g[i] + f[(i - 1)]) + 1)) % 1000000007);
      s[i] = (((s[(i - 1)] + g[i])) % 1000000007);
      i += 1;
    }
  }
  var ans: dynamic = (((f[n] + 1)) % 1000000007);
  write(ans, "\n");
}
