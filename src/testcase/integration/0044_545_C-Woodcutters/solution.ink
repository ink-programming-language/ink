// Translated from solution.cpp.

var MOD: dynamic = (1e9 + 7);

var INF: dynamic = INT_MAX;

var LINF: dynamic = LLONG_MAX;

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic = cpp_uninitialized();
  var maxx: dynamic = (-INF);
  var x: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var res: dynamic = 0;
  read(n);
  var tree: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(tree[i].first, tree[i].second);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var cx: dynamic = tree[i].first;
      var ch: dynamic = tree[i].second;
      if (((cx - ch) > maxx))
      {
        res += 1;
        maxx = cx;
      } else if (((i == (n - 1)) || ((cx + ch) < tree[(i + 1)].first)))
      {
        res += 1;
        maxx = (cx + ch);
      } else
      {
        maxx = cx;
      }
      i += 1;
    }
  }
  write(res);
  return 0;
}
