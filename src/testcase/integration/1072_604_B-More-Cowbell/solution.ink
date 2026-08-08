// Translated from solution.cpp.

var N = (1e5 + 5);

var n: dynamic;

var k: dynamic;

var a = cpp_array(N);

func check(sz: dynamic)
{
  var s: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      s.insert(a[i]);
      i += 1;
    }
  }
  var cnt = 0;
  while (s.size())
  {
    cnt += 1;
    var elem = (*(cpp_update(s.end(), "--")));
    if ((elem > sz))
    {
      return 0;
    }
    s.erase(cpp_update(s.end(), "--"));
    var it = s.upper_bound((sz - elem));
    if ((it != s.begin()))
    {
      s.erase(cpp_update(it, "--"));
    }
  }
  return (cnt <= k);
}

func binsearch(lo: dynamic, hi: dynamic)
{
  while ((lo < hi))
  {
    var mid = (((lo + hi)) / 2);
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

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, k);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ans = binsearch(1, 2e6);
  write(ans);
  return 0;
}
