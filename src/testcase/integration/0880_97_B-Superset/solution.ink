// Translated from solution.cpp.

var MOD = (1e9 + 7);

var INF = INT_MAX;

var LINF = LLONG_MAX;

var N = (1e4 + 20);

var a = cpp_array(N);

var s: dynamic;

var n: dynamic;

func solve(l: dynamic, r: dynamic)
{
  if (((r - l) < 2))
  {
    return;
  }
  var mid = (((l + r)) / 2);
  solve(l, mid);
  solve(mid, r);
  var x = a[mid].first;
  {
    var i = l;
    while ((i < r))
    {
      s.insert(make_pair(x, a[i].second));
      i += 1;
    }
  }
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n);
  {
    var i = 0;
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
  for (var x in s)
  {
    write(x.first, cpp_char(" "), x.second, "\n");
  }
}
