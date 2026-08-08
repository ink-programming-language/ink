// Translated from solution.cpp.

var endl = cpp_expression("#inc");

var int_cpp = dynamic;

var fi = cpp_expression("#incl");

var INF = (1e18 * 9);

var N = (1e6 + 15);

var M = 203;

var mod1 = (1e9 + 7);

var mod = 998244353;

var base = 131;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var ans: dynamic;

var x: dynamic;

var y: dynamic;

var mid: dynamic;

var sx = -1;

var sy = -1;

var mx: dynamic;

var mi = INF;

var aa: dynamic;

var cnt = 1;

var a = cpp_array(N);

class node
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
}

var mp: dynamic;

var mp1: dynamic;

var s: dynamic;

var ss: dynamic;

var ve: dynamic;

var dx = [1, 1, 1, 0, 0, -1, -1, -1];

var dy = [-1, 0, 1, -1, 1, -1, 0, 1];

func solve()
{
  var pos = 0;
  var f1 = 0;
  var s1 = 0;
  var s2 = 0;
  ans = 0;
  mx = INF;
  read(n);
  ve.clear();
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var l = 1;
  var r = n;
  while ((l <= r))
  {
    ve.push_back([1, l, r]);
    a[l] += a[r];
    ve.push_back([2, l, r]);
    a[r] -= a[l];
    ve.push_back([1, l, r]);
    a[l] += a[r];
    ve.push_back([2, l, r]);
    a[r] -= a[l];
    ve.push_back([1, l, r]);
    a[l] += a[r];
    ve.push_back([2, l, r]);
    a[r] -= a[l];
    l += 1;
    r -= 1;
  }
  write(ve.size(), "\n");
  for (var i in ve)
  {
    write(i.x, " ", i.y, " ", i.z, "\n");
  }
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
  var T: dynamic;
  {
    read(T);
    while (cpp_update(T, "--"))
    {
      solve();
    }
  }
}
