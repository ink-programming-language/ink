// Translated from solution.cpp.

var INF = 1e9;

var MOD = (INF + 7);

var N = 22;

var M = ((1 << N));

var adj = cpp_array(N);

var neigh = cpp_array(M);

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < m))
    {
      var v: dynamic;
      var u: dynamic;
      read(v, u);
      v -= 1;
      u -= 1;
      adj[v] += (1 << u);
      adj[u] += (1 << v);
      i += 1;
    }
  }
  if (((2 * m) == ((n * n) - n)))
  {
    return cpp_comma(((cout << 0) << "\n"), 0);
  }
  {
    var i = 0;
    while ((i < n))
    {
      adj[i] += (1 << i);
      neigh[(1 << i)] = adj[i];
      i += 1;
    }
  }
  {
    var mask = 0;
    while ((mask < ((1 << n))))
    {
      {
        var i = 0;
        while ((i < n))
        {
          if (((!((mask & ((1 << i))))) && ((neigh[mask] & ((1 << i))))))
          {
            neigh[(mask | ((1 << i)))] |= ((neigh[mask] | adj[i]));
          }
          i += 1;
        }
      }
      mask += 1;
    }
  }
  var ans = (((1 << n)) - 1);
  {
    var mask = 0;
    while ((mask < ((1 << n))))
    {
      if (((neigh[mask] == (((1 << n)) - 1)) && (builtin_popcount(mask) < builtin_popcount(ans))))
      {
        ans = mask;
      }
      mask += 1;
    }
  }
  write(builtin_popcount(ans), "\n");
  {
    var i = 0;
    while ((i < n))
    {
      if ((ans & ((1 << i))))
      {
        write((i + 1), cpp_char(" "));
      }
      i += 1;
    }
  }
  write("\n");
  return 0;
}
