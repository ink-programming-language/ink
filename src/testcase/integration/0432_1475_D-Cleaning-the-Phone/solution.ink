// Translated from solution.cpp.

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  for (var e: dynamic in v)
  {
    read(e);
  }
  for (var e: dynamic in v)
  {
    var x: dynamic = cpp_uninitialized();
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
  var curSumA: dynamic = 0;
  var r: dynamic = cpp_cast(b.size());
  var curSumB: dynamic = accumulate(b.begin(), b.end(), 0);
  var ans: dynamic = INT_MAX;
  {
    var l: dynamic = 0;
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
  write(( ((ans == INT_MAX)) ? -1 : ans), "\n");
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
