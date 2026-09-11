// Translated from solution.cpp.

var N: dynamic = (1e5 + 5);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

func check(sz: dynamic) -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      s.insert(a[i]);
      i += 1;
    }
  }
  var cnt: dynamic = 0;
  while (s.size())
  {
    cnt += 1;
    var elem: dynamic = (*(cpp_update(s.end(), "--")));
    if ((elem > sz))
    {
      return 0;
    }
    s.erase(cpp_update(s.end(), "--"));
    var it: dynamic = s.upper_bound((sz - elem));
    if ((it != s.begin()))
    {
      s.erase(cpp_update(it, "--"));
    }
  }
  return (cnt <= k);
}

func binsearch(lo: dynamic, hi: dynamic) -> dynamic
{
  while ((lo < hi))
  {
    var mid: dynamic = (((lo + hi)) / 2);
    if (check(mid))
    {
      hi = mid;
    } else
    {
      lo = (mid + 1);
    }
  }
  return lo;
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, k);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ans: dynamic = binsearch(1, 2e6);
  write(ans);
  return 0;
}
