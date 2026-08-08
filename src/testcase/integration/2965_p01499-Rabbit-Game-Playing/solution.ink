// Translated from solution.cpp.

var PI = 3.14159265358979323846;

var EPS = 1e-12;

var INF = (numeric_limits.max() / 2);

var NEG_INF = (numeric_limits.min() / 2);

var MOD = 1000000007;

func binsearch_lower_bound(vec: dynamic, val: dynamic)
{
  var low = 0;
  var high = vec.size();
  while ((low < high))
  {
    var mid = (((low + high)) / 2);
    var midval = vec[mid];
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

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var n: dynamic;
  var t: dynamic;
  read(n, t);
  var v: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var d: dynamic;
      read(d);
      v.push_back(d);
      i += 1;
    }
  }
  sort(v.begin(), v.end());
  var res = 1;
  {
    var i = 1;
    while ((i < n))
    {
      if ((v[(i - 1)] >= (v[i] - t)))
      {
        var idx = binsearch_lower_bound(v, (v[i] - t));
        res = (((res * (((i - idx) + 1)))) % MOD);
      }
      i += 1;
    }
  }
  write(res, "\n");
}
