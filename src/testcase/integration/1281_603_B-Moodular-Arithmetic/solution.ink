// Translated from solution.cpp.

var INF: dynamic = (((1 << 30)) - 1);

var LINF: dynamic = (((1 << 62)) - 1);

var MOD: dynamic = (cpp_cast(1e9) + 7);

var NMAX: dynamic = cpp_cast(1e6);

var P: dynamic = cpp_uninitialized();

var K: dynamic = cpp_uninitialized();

var root: dynamic = cpp_array((NMAX + 5));

var M: dynamic = cpp_uninitialized();

func expLog(B: dynamic, E: dynamic) -> dynamic
{
  var Q: dynamic = B;
  var sol: dynamic = 1;
  {
    var i: dynamic = E;
    while (i)
    {
      if ((i & 1))
      {
        sol = ((((sol * 1) * Q)) % MOD);
      }
      Q = ((((Q * 1) * Q)) % MOD);
      i /= 2;
    }
  }
  return sol;
}

func find(x: dynamic) -> dynamic
{
  if ((x != root[x]))
  {
    root[x] = find(root[x]);
  }
  return root[x];
}

func unite(x: dynamic, y: dynamic) -> dynamic
{
  x = find(x);
  y = find(y);
  root[y] = x;
}

func main() -> dynamic
{
  cin.sync_with_stdio(false);
  scanf("%d%d", (&P), (&K));
  if ((K == 0))
  {
    printf("%d\n", expLog(P, (P - 1)));
    return 0;
  } else if ((K == 1))
  {
    printf("%d\n", expLog(P, P));
    return 0;
  }
  {
    var i: dynamic = 1;
    while ((i <= (P - 1)))
    {
      root[i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    var j: dynamic = cpp_uninitialized();
    while ((i <= (P - 1)))
    {
      j = ((((K * 1) * i)) % P);
      unite(i, j);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= (P - 1)))
    {
      M.insert(find(i));
      i += 1;
    }
  }
  var sol: dynamic = expLog(P, cpp_cast(M.size()));
  printf("%d\n", sol);
  return 0;
}
