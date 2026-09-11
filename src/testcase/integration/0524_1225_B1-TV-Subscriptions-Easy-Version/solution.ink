// Translated from solution.cpp.

var N: dynamic = (1e5 + 5);

var INF: dynamic = (1e9 + 5);

var PI: dynamic = acos(-1);

var X: dynamic = [1, -1, 0, 0];

var Y: dynamic = [0, 0, 1, -1];

var mod: dynamic = (1e9 + 7);

var t: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var mp: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n, k, d);
    var a: dynamic = cpp_array((n + 5));
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        read(a[i]);
        i += 1;
      }
    }
    var cnt: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i <= d))
      {
        if ((!mp[a[i]]))
        {
          cnt += 1;
        }
        mp[a[i]] += 1;
        i += 1;
      }
    }
    var l: dynamic = 1;
    var ans: dynamic = cnt;
    {
      var i: dynamic = (d + 1);
      while ((i <= n))
      {
        mp[a[l]] -= 1;
        if ((mp[a[l]] == 0))
        {
          cnt -= 1;
        }
        l += 1;
        if ((!mp[a[i]]))
        {
          cnt += 1;
        }
        mp[a[i]] += 1;
        ans = min(ans, cnt);
        i += 1;
      }
    }
    write(ans, "\n");
    mp.clear();
  }
  return 0;
}
