// Translated from solution.cpp.

var MAXN = (500 + 17);

var inf = (1e9 + 17);

var n: dynamic;

var m: dynamic;

var dp = cpp_array(MAXN, MAXN);

var par = cpp_array(MAXN, MAXN);

var adj = cpp_array(MAXN);

var ans1: dynamic;

var ans2: dynamic;

func bfs()
{
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          dp[i][j] = inf;
          j += 1;
        }
      }
      i += 1;
    }
  }
  dp[1][n] = 0;
  var q: dynamic;
  q.push([1, n]);
  while (q.size())
  {
    var fr = q.front();
    q.pop();
    for (var i in adj[fr.first])
    {
      for (var j in adj[fr.second])
      {
        if (((i != j) && ((dp[fr.first][fr.second] + 1) < dp[i][j])))
        {
          dp[i][j] = (dp[fr.first][fr.second] + 1);
          par[i][j] = [fr.first, fr.second];
          q.push([i, j]);
        }
      }
    }
  }
}

func pp(i: dynamic = n, j: dynamic = 1)
{
  ans1.push_back(i);
  ans2.push_back(j);
  if (((i == 1) && (j == n)))
  {
    return;
  }
  pp(par[i][j].first, par[i][j].second);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n, m);
  {
    var i = 1;
    while ((i <= m))
    {
      var v: dynamic;
      var u: dynamic;
      read(v, u);
      adj[v].push_back(u);
      adj[u].push_back(v);
      i += 1;
    }
  }
  bfs();
  if ((dp[n][1] == inf))
  {
    return cpp_comma(((cout << -1) << endl), 0);
  }
  write(dp[n][1], "\n");
  pp();
  reverse(ans1.begin(), ans1.end());
  reverse(ans2.begin(), ans2.end());
  for (var i in ans1)
  {
    write(i, cpp_char(" "));
  }
  write("\n");
  for (var i in ans2)
  {
    write(i, cpp_char(" "));
  }
  write("\n");
  return 0;
}
