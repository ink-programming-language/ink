// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic;
  t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}

var EPS = 0.0000000000001;

var n: dynamic;

var p: dynamic;

var a = cpp_array(100005);

var b = cpp_array(100005);

var can = cpp_array(100005);

func check(m: dynamic)
{
  var need = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      need += max(cpp_cast(0), (((m - can[i])) * cpp_cast(a[i])));
      i += 1;
    }
  }
  if ((need > (m * cpp_cast(p))))
  {
    return false;
  }
  return true;
}

func solve()
{
  read(n, p);
  var sum = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i], b[i]);
      can[i] = (cpp_cast(b[i]) / cpp_cast(a[i]));
      sum += a[i];
      i += 1;
    }
  }
  if ((p >= sum))
  {
    write(-1, cpp_char("\n"));
    return;
  }
  var l = (-EPS);
  var r = (1e14 + EPS);
  {
    var i = 1;
    while ((i <= 500))
    {
      var m = (((l + r)) / 2);
      if (check(m))
      {
        l = m;
      } else
      {
        r = m;
      }
      i += 1;
    }
  }
  write(fixed, setprecision(9), l, cpp_char("\n"));
}
