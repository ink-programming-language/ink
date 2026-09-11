// Translated from solution.cpp.

var INF: dynamic = 1e9;

var MOD: dynamic = (1e9 + 7);

var N: dynamic = (1e5 + 5);

var S: dynamic = cpp_array(N);

var T: dynamic = cpp_array(N);

var SZ: dynamic = cpp_array(N);

var P: dynamic = cpp_array(N);

var G: dynamic = cpp_array(N);

func DFS(v: dynamic, p: dynamic = 0) -> dynamic
{
  SZ[v] = 1;
  P[v] = p;
  for (var u: dynamic in G[v])
  {
    if ((u != p))
    {
      DFS(u, v);
      SZ[v] += SZ[u];
      S[v] += S[u];
    }
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var v: dynamic = cpp_uninitialized();
      var u: dynamic = cpp_uninitialized();
      read(v, u);
      G[v].push_back(u);
      G[u].push_back(v);
      i += 1;
    }
  }
  var s: dynamic = 0;
  var t: dynamic = 0;
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
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
    var v: dynamic = 1;
    while ((v <= n))
    {
      var tmp: dynamic = 0;
      for (var u: dynamic in G[v])
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
