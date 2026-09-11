// Translated from solution.cpp.

var FAST: dynamic = cpp_expression("#include <bits/stdc++.h> usin");

var int_cpp: dynamic = dynamic;

var F: dynamic = cpp_expression("#incl");

var S: dynamic = cpp_expression("#inclu");

var pb: dynamic = cpp_expression("#include");

func sz(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bit");
}

func len(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/");
}

var pii: dynamic = cpp_expression("#include <bit");

var ppi: dynamic = cpp_expression("#include <bit");

var vi: dynamic = cpp_expression("#include <b");

var mp: dynamic = cpp_expression("#include");

var minheap: dynamic = cpp_expression("#include <bits/stdc++.h> using namespace std");

var inf: dynamic = 1e18;

var g: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(2, 2, 200001);

func main() -> dynamic
{
  var int_cpp: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  g.resize(n);
  while (cpp_update(m, "--"))
  {
    var u: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    var w: dynamic = cpp_uninitialized();
    read(u, v, w);
    u -= 1;
    v -= 1;
    g[u].pb([v, w]);
    g[v].pb([u, w]);
  }
  var q: dynamic = cpp_uninitialized();
  q.insert([0, [0, 0, 0]]);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < 2))
        {
          {
            var k: dynamic = 0;
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
    var u: dynamic = q.begin()->second[0];
    var mx: dynamic = q.begin()->second[1];
    var mn: dynamic = q.begin()->second[2];
    q.erase(q.begin());
    for (var p: dynamic in g[u])
    {
      var v: dynamic = p.F;
      var w: dynamic = p.S;
      {
        var i: dynamic = mx;
        while ((i < 2))
        {
          {
            var j: dynamic = mn;
            while ((j < 2))
            {
              var wt: dynamic = (w * (((((1 - i) + j) + mx) - mn)));
              if ((dp[v][i][j] > (dp[u][mx][mn] + wt)))
              {
                var it: dynamic = q.find([dp[v][i][j], [v, i, j]]);
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
    var i: dynamic = 1;
    while ((i < n))
    {
      write(dp[i][1][1], cpp_char(" "));
      i += 1;
    }
  }
}
