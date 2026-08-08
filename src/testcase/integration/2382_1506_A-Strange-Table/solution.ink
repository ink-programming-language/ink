// Translated from solution.cpp.

var ll = dynamic;

var pb = cpp_expression("/*There i");

var pf = cpp_expression("/*There is");

var mp = cpp_expression("/*There i");

var tt = cpp_expression("/*The");

var nn = cpp_expression("/*There is");

func ff(i: dynamic, a: dynamic, n: dynamic)
{
  cpp_macro("for(ll i=a;i<n;i++)");
}

func f(i: dynamic, n: dynamic, a: dynamic)
{
  cpp_macro("for(ll i=n;i>=a;i--)");
}

func fr(x: dynamic, a: dynamic)
{
  return cpp_expression("/*There is no");
}

func F(a: dynamic)
{
  return cpp_expression("/*There is no salvati");
}

var cY = cpp_expression("/*There is no");

var cN = cpp_expression("/*There is n");

var cy = cpp_expression("/*There is no");

var cn = cpp_expression("/*There is n");

var sc = cpp_expression("/*Ther");

var fs = cpp_expression("/*The");

func c(a: dynamic)
{
  return cpp_expression("/*There is no");
}

func all(a: dynamic)
{
  return cpp_expression("/*There is no sal");
}

var pi = cpp_expression("/*There is no");

var M = (1e9 + 7);

func mod(x: dynamic)
{
  return (((((x % M) + M)) % M));
}

func fact(n: dynamic)
{
  return if (((n <= 1))) 1 else (n * fact((n - 1)));
}

func solve()
{
  var n: dynamic;
  var m: dynamic;
  var x: dynamic;
  read(n, m, x);
  var r = ((((x + n) - 1)) / n);
  var c = ((n + x) - ((r * n)));
  c(((((c - 1)) * m) + r));
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
}

func nCr(n: dynamic, r: dynamic)
{
  r = if ((((n - r) <= r))) (n - r) else r;
  var ans = 1;
  {
    var i = 0;
    while ((i < r))
    {
      ans *= ((n - i));
      ans /= ((i + 1));
      i += 1;
    }
  }
  return ans;
}

func gcd(a: dynamic, b: dynamic)
{
  if ((a == 0))
  {
    return b;
  }
  return if (((a == 1))) a else gcd((b % a), a);
}

func lcm(a: dynamic, b: dynamic)
{
  return (((a / gcd(a, b))) * b);
}

func prime(n: dynamic)
{
  if ((n < 2))
  {
    return false;
  }
  {
    var i = 2;
    while ((i <= sqrt(n)))
    {
      if (((n % i) == 0))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func BinExp(base: dynamic, power: dynamic)
{
  if ((!power))
  {
    return 1;
  }
  var res = 1;
  while ((power > 1))
  {
    if ((power % 2))
    {
      power -= 1;
      res *= base;
    } else
    {
      base *= base;
      power /= 2;
    }
  }
  return (base * res);
}

func ModInverse(base: dynamic)
{
  var power = (M - 2);
  if ((!power))
  {
    return 1;
  }
  var res = 1;
  while ((power > 1))
  {
    if ((power % 2))
    {
      power -= 1;
      res = mod((res * base));
    } else
    {
      base = mod((base * base));
      power /= 2;
    }
  }
  return mod((base * res));
}
