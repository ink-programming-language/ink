// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}

var EPS: dynamic = 0.0000000000001;

var n: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(100005);

var b: dynamic = cpp_array(100005);

var can: dynamic = cpp_array(100005);

func check(m: dynamic) -> dynamic
{
  var need: dynamic = 0;
  {
    var i: dynamic = 1;
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

func solve() -> dynamic
{
  read(n, p);
  var sum: dynamic = 0;
  {
    var i: dynamic = 1;
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
  var l: dynamic = (-EPS);
  var r: dynamic = (1e14 + EPS);
  {
    var i: dynamic = 1;
    while ((i <= 500))
    {
      var m: dynamic = (((l + r)) / 2);
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
