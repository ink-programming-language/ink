// Translated from solution.cpp.

var PI: dynamic = 3.14159265358979323846;

var EPS: dynamic = 1e-12;

var INF: dynamic = (numeric_limits.max() / 2);

var NEG_INF: dynamic = (numeric_limits.min() / 2);

var MOD: dynamic = 1000000007;

func binsearch_lower_bound(vec: dynamic, val: dynamic) -> dynamic
{
  var low: dynamic = 0;
  var high: dynamic = vec.size();
  while ((low < high))
  {
    var mid: dynamic = (((low + high)) / 2);
    var midval: dynamic = vec[mid];
    if ((midval < val))
    {
      low = (mid + 1);
    } else
    {
      high = mid;
    }
  }
  return low;
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(n, t);
  var v: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var d: dynamic = cpp_uninitialized();
      read(d);
      v.push_back(d);
      i += 1;
    }
  }
  sort(v.begin(), v.end());
  var res: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      if ((v[(i - 1)] >= (v[i] - t)))
      {
        var idx: dynamic = binsearch_lower_bound(v, (v[i] - t));
        res = (((res * (((i - idx) + 1)))) % MOD);
      }
      i += 1;
    }
  }
  write(res, "\n");
}
