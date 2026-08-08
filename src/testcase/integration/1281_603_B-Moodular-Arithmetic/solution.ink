// Translated from solution.cpp.

var INF = (((1 << 30)) - 1);

var LINF = (((1 << 62)) - 1);

var MOD = (cpp_cast(1e9) + 7);

var NMAX = cpp_cast(1e6);

var P: dynamic;

var K: dynamic;

var root = cpp_array((NMAX + 5));

var M: dynamic;

func expLog(B: dynamic, E: dynamic)
{
  var Q = B;
  var sol = 1;
  {
    var i = E;
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

func find(x: dynamic)
{
  if ((x != root[x]))
  {
    root[x] = find(root[x]);
  }
  return root[x];
}

func unite(x: dynamic, y: dynamic)
{
  x = find(x);
  y = find(y);
  root[y] = x;
}

func main()
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
    var i = 1;
    while ((i <= (P - 1)))
    {
      root[i] = i;
      i += 1;
    }
  }
  {
    var i = 1;
    var j: dynamic;
    while ((i <= (P - 1)))
    {
      j = ((((K * 1) * i)) % P);
      unite(i, j);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= (P - 1)))
    {
      M.insert(find(i));
      i += 1;
    }
  }
  var sol = expLog(P, cpp_cast(M.size()));
  printf("%d\n", sol);
  return 0;
}
