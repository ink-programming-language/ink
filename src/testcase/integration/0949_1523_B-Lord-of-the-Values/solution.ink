// Translated from solution.cpp.

var endl: dynamic = cpp_expression("#inc");

var int_cpp: dynamic = dynamic;

var fi: dynamic = cpp_expression("#incl");

var INF: dynamic = (1e18 * 9);

var N: dynamic = (1e6 + 15);

var M: dynamic = 203;

var mod1: dynamic = (1e9 + 7);

var mod: dynamic = 998244353;

var base: dynamic = 131;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var mid: dynamic = cpp_uninitialized();

var sx: dynamic = -1;

var sy: dynamic = -1;

var mx: dynamic = cpp_uninitialized();

var mi: dynamic = INF;

var aa: dynamic = cpp_uninitialized();

var cnt: dynamic = 1;

var a: dynamic = cpp_array(N);

class node
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
}

var mp: dynamic = cpp_uninitialized();

var mp1: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var ss: dynamic = cpp_uninitialized();

var ve: dynamic = cpp_uninitialized();

var dx: dynamic = [1, 1, 1, 0, 0, -1, -1, -1];

var dy: dynamic = [-1, 0, 1, -1, 1, -1, 0, 1];

func solve() -> dynamic
{
  var pos: dynamic = 0;
  var f1: dynamic = 0;
  var s1: dynamic = 0;
  var s2: dynamic = 0;
  ans = 0;
  mx = INF;
  read(n);
  ve.clear();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var l: dynamic = 1;
  var r: dynamic = n;
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
  for (var i: dynamic in ve)
  {
    write(i.x, " ", i.y, " ", i.z, "\n");
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
  var T: dynamic = cpp_uninitialized();
  {
    read(T);
    while (cpp_update(T, "--"))
    {
      solve();
    }
  }
}
