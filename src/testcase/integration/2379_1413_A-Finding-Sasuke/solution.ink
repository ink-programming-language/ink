// Translated from solution.cpp.

var INF: dynamic = (1e18 + 5);

var naxN: dynamic = (1e2 + 5);

var fact: dynamic = cpp_array(naxN);

var inv_fact: dynamic = cpp_array(naxN);

func power(a: dynamic, n: dynamic) -> dynamic
{
  var res: dynamic = 1;
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

func init() -> dynamic
{
  fact[0] = cpp_assign(inv_fact[0], "=", 1);
  {
    var i: dynamic = 1;
    while ((i < naxN))
    {
      fact[i] = (((i * fact[(i - 1)])) % 1000000007);
      inv_fact[i] = (power(fact[i], (1000000007 - 2)) % 1000000007);
      i += 1;
    }
  }
}

func ncr(a: dynamic, b: dynamic) -> dynamic
{
  if ((((a < 0) || (b < 0)) || (a < b)))
  {
    return 0;
  }
  return (((((((fact[a] % 1000000007) * inv_fact[b]) % 1000000007) * inv_fact[(a - b)]) % 1000000007)) % 1000000007);
}

var maxN: dynamic = (1e5 + 5);

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  write(fixed, setprecision(6));
  var T: dynamic = 1;
  read(T);
  while (cpp_update(T, "--"))
  {
    solve();
  }
}
