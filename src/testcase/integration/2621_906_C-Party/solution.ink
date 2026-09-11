// Translated from solution.cpp.

var INF: dynamic = 1e9;

var MOD: dynamic = (INF + 7);

var N: dynamic = 22;

var M: dynamic = ((1 << N));

var adj: dynamic = cpp_array(N);

var neigh: dynamic = cpp_array(M);

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var v: dynamic = cpp_uninitialized();
      var u: dynamic = cpp_uninitialized();
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
    var i: dynamic = 0;
    while ((i < n))
    {
      adj[i] += (1 << i);
      neigh[(1 << i)] = adj[i];
      i += 1;
    }
  }
  {
    var mask: dynamic = 0;
    while ((mask < ((1 << n))))
    {
      {
        var i: dynamic = 0;
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
  var ans: dynamic = (((1 << n)) - 1);
  {
    var mask: dynamic = 0;
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
    var i: dynamic = 0;
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
