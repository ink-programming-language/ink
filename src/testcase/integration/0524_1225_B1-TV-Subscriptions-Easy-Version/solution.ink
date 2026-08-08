// Translated from solution.cpp.

var N = (1e5 + 5);

var INF = (1e9 + 5);

var PI = acos(-1);

var X = [1, -1, 0, 0];

var Y = [0, 0, 1, -1];

var mod = (1e9 + 7);

var t: dynamic;

var n: dynamic;

var k: dynamic;

var d: dynamic;

var mp: dynamic;

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n, k, d);
    var a = cpp_array((n + 5));
    {
      var i = 1;
      while ((i <= n))
      {
        read(a[i]);
        i += 1;
      }
    }
    var cnt = 0;
    {
      var i = 1;
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
    var l = 1;
    var ans = cnt;
    {
      var i = (d + 1);
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
