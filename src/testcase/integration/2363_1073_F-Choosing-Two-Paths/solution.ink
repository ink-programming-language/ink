// Translated from solution.cpp.

var sz = (2e5 + 10);

var sv = cpp_array(sz);

var dp = cpp_array(sz);

var ve = cpp_array(sz);

var be = [0, 0];

var an1: dynamic;

var an2: dynamic;

func dfs(v: dynamic, pr: dynamic)
{
  dp[v] = [0, 1];
  ve[v] = [v, -1];
  var sp: dynamic;
  {
    var a = 0;
    while ((a < sv[v].size()))
    {
      var ne = sv[v][a];
      if ((ne != pr))
      {
        dfs(ne, v);
        var q = dp[ne].first;
        var su = dp[ne].second;
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
            var cq = (dp[v].first + dp[ne].first);
            var csu = (dp[v].second + dp[ne].second);
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
      var q = dp[v].first;
      var su = ((dp[v].second + sp[0].first) + sp[1].first);
      if ((make_pair(q, su) > be))
      {
        be = [q, su];
        an1 = ve[v];
        an2 = [sp[0].second, sp[1].second];
      }
    } else
    {
      var q = 1;
      var su = ((sp[0].first + sp[1].first) + 2);
      dp[v] = [q, su];
      ve[v] = [sp[0].second, sp[1].second];
    }
  }
}

func main()
{
  var n: dynamic;
  read(n);
  {
    var a = 0;
    while ((a < (n - 1)))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d%d", (&u), (&v));
      u -= 1;
      v -= 1;
      sv[u].push_back(v);
      sv[v].push_back(u);
      a += 1;
    }
  }
  {
    var a = 0;
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
