// Translated from solution.cpp.

var MAXN: dynamic = 1005;

var cost: dynamic = cpp_array(MAXN);

var used: dynamic = cpp_array(MAXN);

var g: dynamic = cpp_array(MAXN);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(cost[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      x -= 1;
      y -= 1;
      g[x].push_back(y);
      g[y].push_back(x);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var maxv: dynamic = -1;
      {
        var j: dynamic = 0;
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
        var j: dynamic = 0;
        while ((j < g[maxv].size()))
        {
          ans += cost[g[maxv][j]];
          j += 1;
        }
      }
      used[maxv] = true;
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          {
            var k: dynamic = 0;
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
