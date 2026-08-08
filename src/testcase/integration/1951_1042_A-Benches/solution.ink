// Translated from solution.cpp.

var INF = 100010;

var n: dynamic;

var m: dynamic;

var a = cpp_array(INF);

var ss: dynamic;

var maxn = 0;

var ans: dynamic;

func main()
{
  read(n, m);
  ss = m;
  {
    var i = 0;
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
    var i = 0;
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
