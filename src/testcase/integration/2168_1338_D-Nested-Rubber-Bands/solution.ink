// Translated from solution.cpp.

var MN = 100005;

var inf = 1000000005;

var mod = 1000000007;

var INF = 1000000000000000005;

var dp = cpp_array(MN, 2);

var naj = cpp_array(MN, 4);

var G = cpp_array(MN);

var ans: dynamic;

func dfs_pre(x: dynamic, p: dynamic)
{
  var sons = (G[x].size() - ((p != 0)));
  for (var v in G[x])
  {
    if ((v != p))
    {
      dfs_pre(v, x);
      if ((dp[0][v] >= naj[0][x].first))
      {
        naj[1][x] = naj[0][x];
        naj[0][x] = [dp[0][v], v];
      } else if ((dp[0][v] > naj[1][x].first))
      {
        naj[1][v] = [dp[0][v], v];
      }
      if ((dp[1][v] >= naj[2][x].first))
      {
        naj[3][x] = naj[2][x];
        naj[2][x] = [dp[1][v], v];
      } else if ((dp[1][v] > naj[3][x].first))
      {
        naj[3][v] = [dp[1][v], v];
      }
    }
  }
  dp[0][x] = max(0, ((sons - 1) + max(naj[0][x].first, naj[2][x].first)));
  dp[1][x] = (1 + naj[0][x].first);
}

func dfs_licz(x: dynamic, p: dynamic, res_par_bez: dynamic, res_par_z: dynamic)
{
  var sons = G[x].size();
  if ((res_par_bez >= naj[0][x].first))
  {
    naj[1][x] = naj[0][x];
    naj[0][x] = [res_par_bez, p];
  } else if ((res_par_bez > naj[1][x].first))
  {
    naj[1][x] = [res_par_bez, p];
  }
  if ((res_par_z >= naj[2][x].first))
  {
    naj[3][x] = naj[2][x];
    naj[2][x] = [res_par_z, p];
  } else if ((res_par_z > naj[3][x].first))
  {
    naj[3][x] = [res_par_z, p];
  }
  var res_bez = max(0, ((sons - 1) + max(naj[0][x].first, naj[2][x].first)));
  var res_z = (1 + naj[0][x].first);
  ans = max(ans, max(res_bez, res_z));
  for (var v in G[x])
  {
    if ((v != p))
    {
      var idx_bez = ((naj[0][x].second == v));
      var idx_z = (2 + ((naj[2][x].second == v)));
      res_bez = max(0, ((sons - 2) + max(naj[idx_bez][x].first, naj[idx_z][x].first)));
      res_z = (1 + naj[idx_bez][x].first);
      dfs_licz(v, x, res_bez, res_z);
    }
  }
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i < n))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d%d", (&u), (&v));
      G[u].push_back(v);
      G[v].push_back(u);
      i += 1;
    }
  }
  dfs_pre(1, 0);
  dfs_licz(1, 0, 0, 0);
  printf("%d", ans);
}
