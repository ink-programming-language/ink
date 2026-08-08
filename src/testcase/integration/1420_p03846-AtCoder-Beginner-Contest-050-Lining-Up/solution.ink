// Translated from solution.cpp.

var MOD = cpp_expression("#include<c");

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = 0; i < n; i++)");
}

var n: dynamic;

var a = cpp_array(100000);

var c = cpp_array(100000);

func mod_pow(x: dynamic, n: dynamic)
{
  if ((n == 0))
  {
    return 1;
  }
  return ((x * mod_pow(x, (n - 1))) % MOD);
}

func solve()
{
  if ((c[0] > 1))
  {
    return 0;
  }
  rep(i, n);
  if ((c[i] > 2))
  {
    return 0;
  }
  return mod_pow(2, (n / 2));
}

func main()
{
  scanf("%d", (&n));
  printf("%lld\n", solve());
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if ((!((n ^ ((a[i] & 1))))))
    {
      return 0;
    }
    if ((a[i] >= n))
    {
      return 0;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    scanf("%d", (&a[i]));
    c[a[i]] += 1;
  }
