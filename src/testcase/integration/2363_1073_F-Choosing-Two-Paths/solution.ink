// Translated from solution.cpp.

var sz: dynamic = (2e5 + 10);

var sv: dynamic = cpp_array(sz);

var dp: dynamic = cpp_array(sz);

var ve: dynamic = cpp_array(sz);

var be: dynamic = [0, 0];

var an1: dynamic = cpp_uninitialized();

var an2: dynamic = cpp_uninitialized();

func dfs(v: dynamic, pr: dynamic) -> dynamic
{
  dp[v] = [0, 1];
  ve[v] = [v, -1];
  var sp: dynamic = cpp_uninitialized();
  {
    var a: dynamic = 0;
    while ((a < sv[v].size()))
    {
      var ne: dynamic = sv[v][a];
      if ((ne != pr))
      {
        dfs(ne, v);
        var q: dynamic = dp[ne].first;
        var su: dynamic = dp[ne].second;
        if ((q == 0))
        {
          sp.push_back([su, ve[ne].first]);
          if ((make_pair(0, (su + 1)) > dp[v]))
          {
            dp[v] = [0, (su + 1)];
            ve[v] = ve[ne];
          }
        } else
        {
          if (dp[v].first)
          {
            var cq: dynamic = (dp[v].first + dp[ne].first);
            var csu: dynamic = (dp[v].second + dp[ne].second);
            if ((make_pair(cq, csu) > be))
            {
              be = [cq, csu];
              an1 = ve[v];
              an2 = ve[ne];
            }
          }
          if ((make_pair((q + 1), (su + 2)) > dp[v]))
          {
            dp[v] = [(q + 1), (su + 2)];
            ve[v] = ve[ne];
          }
        }
      }
      a += 1;
    }
  }
  sort(sp.begin(), sp.end(), greater());
  if ((sp.size() > 1))
  {
    if (dp[v].first)
    {
      var q: dynamic = dp[v].first;
      var su: dynamic = ((dp[v].second + sp[0].first) + sp[1].first);
      if ((make_pair(q, su) > be))
      {
        be = [q, su];
        an1 = ve[v];
        an2 = [sp[0].second, sp[1].second];
      }
    } else
    {
      var q: dynamic = 1;
      var su: dynamic = ((sp[0].first + sp[1].first) + 2);
      dp[v] = [q, su];
      ve[v] = [sp[0].second, sp[1].second];
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var a: dynamic = 0;
    while ((a < (n - 1)))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d%d", (&u), (&v));
      u -= 1;
      v -= 1;
      sv[u].push_back(v);
      sv[v].push_back(u);
      a += 1;
    }
  }
  {
    var a: dynamic = 0;
    while ((a < n))
    {
      if ((sv[a].size() > 2))
      {
        dfs(a, a);
        break;
      }
      a += 1;
    }
  }
  write((an1.first + 1), " ", (an2.first + 1), "\n");
  write((an1.second + 1), " ", (an2.second + 1));
}
