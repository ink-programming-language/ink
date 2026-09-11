// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

var INF: dynamic = INT_MAX;

var NINF: dynamic = INT_MIN;

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var curr: dynamic = cpp_uninitialized();

var vec: dynamic = cpp_uninitialized();

var tv: dynamic = cpp_uninitialized();

var Count: dynamic = cpp_uninitialized();

func Solve() -> dynamic
{
  read(n, q);
  vec = cpp_assign(tv, "=", vector(n, 0));
  {
    var i: dynamic = 0;
    while ((i <= (n - 1)))
    {
      read(vec[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
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
  var ans: dynamic = 0;
  var i: dynamic = 0;
  for (var v: dynamic in Count)
  {
    ans += ((((v * 1)) * ((vec[i] * 1))));
    i += 1;
  }
  write(ans, "\n");
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  Solve();
  return 0;
}
