// Translated from solution.cpp.

var INF: dynamic = 100010;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(INF);

var ss: dynamic = cpp_uninitialized();

var maxn: dynamic = 0;

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n, m);
  ss = m;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      if ((a[i] >= maxn))
      {
        maxn = a[i];
      }
      i += 1;
    }
  }
  sort(a, (a + n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((a[i] != maxn))
      {
        ans += (maxn - a[i]);
      }
      i += 1;
    }
  }
  if ((ans >= m))
  {
    write(maxn);
  } else
  {
    ans = (m - ans);
    ans = ((((ans + n) - 1)) / n);
    write((maxn + ans));
  }
  write(" ", (a[(n - 1)] + m));
  return 0;
}
