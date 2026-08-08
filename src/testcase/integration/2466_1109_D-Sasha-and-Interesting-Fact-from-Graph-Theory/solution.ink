// Translated from solution.cpp.

var MOD = (cpp_cast(1e9) + 7);

var N = (1e6 + 5);

var inf = (1e9 + 5);

func add(x: dynamic, y: dynamic)
{
  x += y;
  if ((x >= MOD))
  {
    return (x - MOD);
  }
  return x;
}

func sub(x: dynamic, y: dynamic)
{
  x -= y;
  if ((x < 0))
  {
    return (x + MOD);
  }
  return x;
}

func mult(x: dynamic, y: dynamic)
{
  return (((x * y)) % MOD);
}

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

func mod_pow(x: dynamic, e: dynamic)
{
  var ans = 1;
  while ((e > 0))
  {
    if ((e & 1))
    {
      ans = mult(ans, x);
    }
    x = mult(x, x);
    e >>= 1;
  }
  return ans;
}

var fact = cpp_array(N);

var inv_fact = cpp_array(N);

func pre_fact()
{
  fact[0] = cpp_assign(inv_fact[0], "=", 1);
  {
    var i = 1;
    while ((i <= N))
    {
      fact[i] = mult(fact[(i - 1)], i);
      inv_fact[i] = mod_pow(fact[i], (MOD - 2));
      i += 1;
    }
  }
}

func binom(n: dynamic, k: dynamic)
{
  if ((k == 0))
  {
    return 1;
  }
  return mult(fact[n], mult(inv_fact[k], inv_fact[(n - k)]));
}

func cayley(n: dynamic, k: dynamic)
{
  if ((((n - k) - 1) < 0))
  {
    return 1;
  }
  return mult(k, mod_pow(n, ((n - k) - 1)));
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  var m: dynamic;
  var a: dynamic;
  var b: dynamic;
  read(n, m, a, b);
  var ans = 0;
  pre_fact();
  {
    var k = 1;
    while ((k < min(n, (m + 1))))
    {
      var curr = 1;
      curr = mult(curr, binom((m - 1), (k - 1)));
      curr = mult(curr, mod_pow(m, ((n - 1) - k)));
      curr = mult(curr, cayley(n, (k + 1)));
      curr = mult(curr, mult(fact[(n - 2)], inv_fact[((n - 2) - ((k - 1)))]));
      ans = add(ans, curr);
      k += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
