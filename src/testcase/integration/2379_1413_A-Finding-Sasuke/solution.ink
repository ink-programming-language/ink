// Translated from solution.cpp.

var INF = (1e18 + 5);

var naxN = (1e2 + 5);

var fact = cpp_array(naxN);

var inv_fact = cpp_array(naxN);

func power(a: dynamic, n: dynamic)
{
  var res = 1;
  while (n)
  {
    if ((n % 2))
    {
      res = (((res * a)) % 1000000007);
      n -= 1;
    } else
    {
      a = (((a * a)) % 1000000007);
      n /= 2;
    }
  }
  return res;
}

func init()
{
  fact[0] = cpp_assign(inv_fact[0], "=", 1);
  {
    var i = 1;
    while ((i < naxN))
    {
      fact[i] = (((i * fact[(i - 1)])) % 1000000007);
      inv_fact[i] = (power(fact[i], (1000000007 - 2)) % 1000000007);
      i += 1;
    }
  }
}

func ncr(a: dynamic, b: dynamic)
{
  if ((((a < 0) || (b < 0)) || (a < b)))
  {
    return 0;
  }
  return (((((((fact[a] % 1000000007) * inv_fact[b]) % 1000000007) * inv_fact[(a - b)]) % 1000000007)) % 1000000007);
}

var maxN = (1e5 + 5);

func solve()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if (((i % 2) == 0))
      {
        write(v[(i + 1)], " ");
      } else
      {
        write((-v[(i - 1)]), " ");
      }
      i += 1;
    }
  }
  write("\n");
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  write(fixed, setprecision(6));
  var T = 1;
  read(T);
  while (cpp_update(T, "--"))
  {
    solve();
  }
}
