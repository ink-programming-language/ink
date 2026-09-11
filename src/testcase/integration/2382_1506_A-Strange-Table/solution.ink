// Translated from solution.cpp.

var ll: dynamic = dynamic;

var pb: dynamic = cpp_expression("/*There i");

var pf: dynamic = cpp_expression("/*There is");

var mp: dynamic = cpp_expression("/*There i");

var tt: dynamic = cpp_expression("/*The");

var nn: dynamic = cpp_expression("/*There is");

func ff(i: dynamic, a: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(ll i=a;i<n;i++)");
}

func f(i: dynamic, n: dynamic, a: dynamic) -> dynamic
{
  cpp_macro("for(ll i=n;i>=a;i--)");
}

func fr(x: dynamic, a: dynamic) -> dynamic
{
  return cpp_expression("/*There is no");
}

func F(a: dynamic) -> dynamic
{
  return cpp_expression("/*There is no salvati");
}

var cY: dynamic = cpp_expression("/*There is no");

var cN: dynamic = cpp_expression("/*There is n");

var cy: dynamic = cpp_expression("/*There is no");

var cn: dynamic = cpp_expression("/*There is n");

var sc: dynamic = cpp_expression("/*Ther");

var fs: dynamic = cpp_expression("/*The");

func c(a: dynamic) -> dynamic
{
  return cpp_expression("/*There is no");
}

func all(a: dynamic) -> dynamic
{
  return cpp_expression("/*There is no sal");
}

var pi: dynamic = cpp_expression("/*There is no");

var M: dynamic = (1e9 + 7);

func mod(x: dynamic) -> dynamic
{
  return (((((x % M) + M)) % M));
}

func fact(n: dynamic) -> dynamic
{
  return  (((n <= 1))) ? 1 : (n * fact((n - 1)));
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(n, m, x);
  var r: dynamic = ((((x + n) - 1)) / n);
  var c: dynamic = ((n + x) - ((r * n)));
  c(((((c - 1)) * m) + r));
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
}

func nCr(n: dynamic, r: dynamic) -> dynamic
{
  r =  ((((n - r) <= r))) ? (n - r) : r;
  var ans: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < r))
    {
      ans *= ((n - i));
      ans /= ((i + 1));
      i += 1;
    }
  }
  return ans;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((a == 0))
  {
    return b;
  }
  return  (((a == 1))) ? a : gcd((b % a), a);
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return (((a / gcd(a, b))) * b);
}

func prime(n: dynamic) -> dynamic
{
  if ((n < 2))
  {
    return false;
  }
  {
    var i: dynamic = 2;
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

func BinExp(base: dynamic, power: dynamic) -> dynamic
{
  if ((!power))
  {
    return 1;
  }
  var res: dynamic = 1;
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

func ModInverse(base: dynamic) -> dynamic
{
  var power: dynamic = (M - 2);
  if ((!power))
  {
    return 1;
  }
  var res: dynamic = 1;
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
