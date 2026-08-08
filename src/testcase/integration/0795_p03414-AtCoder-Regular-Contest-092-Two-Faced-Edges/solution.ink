// Translated from solution.cpp.

var dp = cpp_array(1010, 1010, 2);

var pv = cpp_array(1010, 1010);

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < m))
    {
      read(a[i], b[i]);
      a[i] -= 1;
      b[i] -= 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      G[a[i]].emplace_back(b[i]);
      i += 1;
    }
  }
  memset(dp, 0, cpp_sizeof((dp)));
  memset(pv, -1, cpp_sizeof((pv)));
  var q: dynamic;
  {
    var i = 0;
    while ((i < m))
    {
      pv[a[i]][b[i]] = i;
      dp[0][a[i]][b[i]] = 1;
      q.emplace(0, a[i], b[i]);
      i += 1;
    }
  }
  while ((!q.empty()))
  {
    var t: dynamic;
    var x: dynamic;
    var y: dynamic;
    tie(t, x, y) = q.front();
    q.pop();
    for (var z in G[y])
    {
      if ((x == z))
      {
        continue;
      }
      var nt = (t || (((~pv[x][z]) && (pv[x][y] != pv[x][z]))));
      if (dp[nt][x][z])
      {
        continue;
      }
      if ((pv[x][z] < 0))
      {
        pv[x][z] = pv[x][y];
      }
      dp[nt][x][z] = 1;
      q.emplace(nt, x, z);
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var s = dp[1][a[i]][b[i]];
      var t = (dp[0][b[i]][a[i]] | dp[1][b[i]][a[i]]);
      write((if ((s ^ t)) "diff" else "same"), "\n");
      i += 1;
    }
  }
  return 0;
}
