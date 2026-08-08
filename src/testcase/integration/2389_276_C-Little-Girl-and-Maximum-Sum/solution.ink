// Translated from solution.cpp.

var mod = (1e9 + 7);

var INF = INT_MAX;

var NINF = INT_MIN;

var n: dynamic;

var q: dynamic;

var l: dynamic;

var r: dynamic;

var curr: dynamic;

var vec: dynamic;

var tv: dynamic;

var Count: dynamic;

func Solve()
{
  read(n, q);
  vec = cpp_assign(tv, "=", vector(n, 0));
  {
    var i = 0;
    while ((i <= (n - 1)))
    {
      read(vec[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= (q - 1)))
    {
      read(l, r);
      l -= 1;
      r;
      tv[l] += 1;
      if ((r < n))
      {
        tv[r] -= 1;
      }
      i += 1;
    }
  }
  curr = 0;
  {
    var i = 0;
    while ((i <= (n - 1)))
    {
      curr += tv[i];
      if ((curr > 0))
      {
        Count.push_back(curr);
      }
      i += 1;
    }
  }
  sort(Count.rbegin(), Count.rend());
  sort(vec.rbegin(), vec.rend());
  var ans = 0;
  var i = 0;
  for (var v in Count)
  {
    ans += ((((v * 1)) * ((vec[i] * 1))));
    i += 1;
  }
  write(ans, "\n");
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  Solve();
  return 0;
}
