// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var g1: dynamic = 0;
  var g2: dynamic = 0;
  var a: dynamic = cpp_array((n / 2));
  {
    var i: dynamic = 0;
    while ((i < (n / 2)))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + (n / 2)));
  {
    var i: dynamic = 1;
    while ((i <= (n / 2)))
    {
      g1 += abs((a[(i - 1)] - (((i * 2) - 1))));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= (n / 2)))
    {
      g2 += abs((a[(i - 1)] - ((i * 2))));
      i += 1;
    }
  }
  write(min(g1, g2), "\n");
  return 0;
}
