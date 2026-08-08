// Translated from solution.cpp.

var MAXN = 1005;

var cost = cpp_array(MAXN);

var used = cpp_array(MAXN);

var g = cpp_array(MAXN);

func main()
{
  ios_base.sync_with_stdio(false);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      read(cost[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      x -= 1;
      y -= 1;
      g[x].push_back(y);
      g[y].push_back(x);
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var maxv = -1;
      {
        var j = 0;
        while ((j < n))
        {
          if (((!used[j]) && (((maxv == -1) || (cost[j] > cost[maxv])))))
          {
            maxv = j;
          }
          j += 1;
        }
      }
      {
        var j = 0;
        while ((j < g[maxv].size()))
        {
          ans += cost[g[maxv][j]];
          j += 1;
        }
      }
      used[maxv] = true;
      {
        var j = 0;
        while ((j < n))
        {
          {
            var k = 0;
            while ((k < cpp_cast(g[j].size())))
            {
              if ((g[j][k] == maxv))
              {
                swap(g[j][k], g[j].back());
                g[j].pop_back();
                k -= 1;
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, cpp_char("\n"));
  return 0;
}
