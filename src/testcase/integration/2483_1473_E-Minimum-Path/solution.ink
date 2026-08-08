// Translated from solution.cpp.

var FAST = cpp_expression("#include <bits/stdc++.h> usin");

var int_cpp = dynamic;

var F = cpp_expression("#incl");

var S = cpp_expression("#inclu");

var pb = cpp_expression("#include");

func sz(x: dynamic)
{
  return cpp_expression("#include <bit");
}

func len(x: dynamic)
{
  return cpp_expression("#include <bits/");
}

var pii = cpp_expression("#include <bit");

var ppi = cpp_expression("#include <bit");

var vi = cpp_expression("#include <b");

var mp = cpp_expression("#include");

var minheap = cpp_expression("#include <bits/stdc++.h> using namespace std");

var inf = 1e18;

var g: dynamic;

var dp = cpp_array(2, 2, 200001);

func main()
{
  var int_cpp: dynamic;
  var m: dynamic;
  read(n, m);
  g.resize(n);
  while (cpp_update(m, "--"))
  {
    var u: dynamic;
    var v: dynamic;
    var w: dynamic;
    read(u, v, w);
    u -= 1;
    v -= 1;
    g[u].pb([v, w]);
    g[v].pb([u, w]);
  }
  var q: dynamic;
  q.insert([0, [0, 0, 0]]);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < 2))
        {
          {
            var k = 0;
            while ((k < 2))
            {
              dp[i][j][k] = inf;
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  dp[0][0][0] = 0;
  while ((!q.empty()))
  {
    var u = q.begin()->second[0];
    var mx = q.begin()->second[1];
    var mn = q.begin()->second[2];
    q.erase(q.begin());
    for (var p in g[u])
    {
      var v = p.F;
      var w = p.S;
      {
        var i = mx;
        while ((i < 2))
        {
          {
            var j = mn;
            while ((j < 2))
            {
              var wt = (w * (((((1 - i) + j) + mx) - mn)));
              if ((dp[v][i][j] > (dp[u][mx][mn] + wt)))
              {
                var it = q.find([dp[v][i][j], [v, i, j]]);
                if ((it != q.end()))
                {
                  q.erase(it);
                }
                dp[v][i][j] = (dp[u][mx][mn] + wt);
                q.insert([dp[v][i][j], [v, i, j]]);
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      write(dp[i][1][1], cpp_char(" "));
      i += 1;
    }
  }
}
