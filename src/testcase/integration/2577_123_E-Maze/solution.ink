// Translated from solution.cpp.

var INF = 1e9;

var MOD = (1e9 + 7);

var N = (1e5 + 5);

var S = cpp_array(N);

var T = cpp_array(N);

var SZ = cpp_array(N);

var P = cpp_array(N);

var G = cpp_array(N);

func DFS(v: dynamic, p: dynamic = 0)
{
  SZ[v] = 1;
  P[v] = p;
  for (var u in G[v])
  {
    if ((u != p))
    {
      DFS(u, v);
      SZ[v] += SZ[u];
      S[v] += S[u];
    }
  }
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var v: dynamic;
      var u: dynamic;
      read(v, u);
      G[v].push_back(u);
      G[u].push_back(v);
      i += 1;
    }
  }
  var s = 0;
  var t = 0;
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      read(S[i], T[i]);
      s += S[i];
      t += T[i];
      i += 1;
    }
  }
  DFS(1);
  {
    var v = 1;
    while ((v <= n))
    {
      var tmp = 0;
      for (var u in G[v])
      {
        if ((u == P[v]))
        {
          tmp += ((1 * ((n - SZ[v]))) * ((s - S[v])));
        } else
        {
          tmp += ((1 * S[u]) * SZ[u]);
        }
      }
      ans += (tmp * T[v]);
      v += 1;
    }
  }
  write(setprecision(15), (((1.0 * ans)) / (((1.0 * s) * t))), cpp_char("\n"));
  return 0;
}
