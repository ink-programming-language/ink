// Translated from solution.cpp.

func solve()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var a: dynamic;
  var b: dynamic;
  for (var e in v)
  {
    read(e);
  }
  for (var e in v)
  {
    var x: dynamic;
    read(x);
    if ((x == 1))
    {
      a.push_back(e);
    } else
    {
      b.push_back(e);
    }
  }
  sort(a.rbegin(), a.rend());
  sort(b.rbegin(), b.rend());
  var curSumA = 0;
  var r = cpp_cast(b.size());
  var curSumB = accumulate(b.begin(), b.end(), 0);
  var ans = INT_MAX;
  {
    var l = 0;
    while ((l <= a.size()))
    {
      while (((r > 0) && (((curSumA + curSumB) - b[(r - 1)]) >= m)))
      {
        r -= 1;
        curSumB -= b[r];
      }
      if (((curSumB + curSumA) >= m))
      {
        ans = min(ans, ((2 * r) + l));
      }
      if ((l != a.size()))
      {
        curSumA += a[l];
      }
      l += 1;
    }
  }
  write((if ((ans == INT_MAX)) -1 else ans), "\n");
}

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
