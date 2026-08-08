// Translated from solution.cpp.

var a = cpp_array(200010);

func calc(l: dynamic, r: dynamic, x: dynamic, y: dynamic)
{
  if (((x == l) && (y == r)))
  {
    return 0;
  }
  var t = if (((((x + y)) % 2))) 0 else 1;
  if (((x - l) == (y - r)))
  {
    return (t * ((x - l)));
  } else
  {
    if (((((l + r)) % 2) == 0))
    {
      return (((((x - l) - y) + r)) / 2);
    } else
    {
      return ((((((x - l) - y) + r)) / 2) + ((+((((x - l) - y) + r))) % 2));
    }
  }
}

func solve()
{
  var n: dynamic;
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i].first);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i].second);
      i += 1;
    }
  }
  sort((a + 1), ((a + 1) + n));
  var l = 1;
  var r = 1;
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      ans += calc(l, r, a[i].first, a[i].second);
      l = a[i].first;
      r = a[i].second;
      i += 1;
    }
  }
  write(ans, cpp_char("\n"));
}

func main()
{
  ios.sync_with_stdio(false);
  var t: dynamic;
  read(t);
  {
    var tt = 1;
    while ((tt <= t))
    {
      solve();
      tt += 1;
    }
  }
}
