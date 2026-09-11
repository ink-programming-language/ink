// Translated from solution.cpp.

var dp: dynamic = cpp_array(1010, 1010, 2);

var pv: dynamic = cpp_array(1010, 1010);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(a[i], b[i]);
      a[i] -= 1;
      b[i] -= 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      G[a[i]].emplace_back(b[i]);
      i += 1;
    }
  }
  memset(dp, 0, cpp_sizeof((dp)));
  memset(pv, -1, cpp_sizeof((pv)));
  var q: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
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
    var t: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    tie(t, x, y) = q.front();
    q.pop();
    for (var z: dynamic in G[y])
    {
      if ((x == z))
      {
        continue;
      }
      var nt: dynamic = (t || (((~pv[x][z]) && (pv[x][y] != pv[x][z]))));
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
    var i: dynamic = 0;
    while ((i < m))
    {
      var s: dynamic = dp[1][a[i]][b[i]];
      var t: dynamic = (dp[0][b[i]][a[i]] | dp[1][b[i]][a[i]]);
      write(( ((s ^ t)) ? "diff" : "same"), "\n");
      i += 1;
    }
  }
  return 0;
}
