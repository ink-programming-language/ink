// Translated from solution.cpp.

var MOD: dynamic = (1e9 + 7);

var INF: dynamic = INT_MAX;

var LINF: dynamic = LLONG_MAX;

var N: dynamic = (1e4 + 20);

var a: dynamic = cpp_array(N);

var s: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

func solve(l: dynamic, r: dynamic) -> dynamic
{
  if (((r - l) < 2))
  {
    return;
  }
  var mid: dynamic = (((l + r)) / 2);
  solve(l, mid);
  solve(mid, r);
  var x: dynamic = a[mid].first;
  {
    var i: dynamic = l;
    while ((i < r))
    {
      s.insert(make_pair(x, a[i].second));
      i += 1;
    }
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i].first, a[i].second);
      s.insert(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  solve(0, n);
  write(s.size(), "\n");
  for (var x: dynamic in s)
  {
    write(x.first, cpp_char(" "), x.second, "\n");
  }
}
