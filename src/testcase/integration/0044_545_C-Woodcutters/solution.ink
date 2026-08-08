// Translated from solution.cpp.

var MOD = (1e9 + 7);

var INF = INT_MAX;

var LINF = LLONG_MAX;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic;
  var maxx = (-INF);
  var x: dynamic;
  var h: dynamic;
  var res = 0;
  read(n);
  var tree = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(tree[i].first, tree[i].second);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      var cx = tree[i].first;
      var ch = tree[i].second;
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
