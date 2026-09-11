// Translated from solution.cpp.

var t: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(200005);

var s: dynamic = cpp_construct(5000005, 0);

var d: dynamic = cpp_construct(5000005, 0);

var mp: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var mxn: dynamic = 200005;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func bpow(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while ((b > 0))
  {
    if ((b & 1))
    {
      res = (((res * a)) % 1000000007);
    }
    a = (((a * a)) % 1000000007);
    b >>= 1;
  }
  return (res % 1000000007);
}

func fact(n: dynamic) -> dynamic
{
  if ((n == 0))
  {
    return 1;
  }
  return (n * fact((n - 1)));
}

func isprime(n: dynamic) -> dynamic
{
  {
    var i: dynamic = 2;
    while ((i <= sqrt(n)))
    {
      if (((n % i) == 0))
      {
        return 0;
      }
      i += 1;
    }
  }
  return 1;
}

func prime() -> dynamic
{
  {
    var i: dynamic = 2;
    while ((i <= 5000000))
    {
      if ((s[i] == 0))
      {
        {
          var j: dynamic = 2;
          while (((i * j) <= 5000000))
          {
            s[(i * j)] = 1;
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
}

func pal(i: dynamic) -> dynamic
{
  var j: dynamic = i;
  var c: dynamic = 0;
  var d: dynamic = 0;
  while ((j > 0))
  {
    c += 1;
    j /= 10;
  }
  {
    var j: dynamic = 1;
    while ((j <= (c / 2)))
    {
      d *= 10;
      d += (i % 10);
      i /= 10;
      j += 1;
    }
  }
  if ((c & 1))
  {
    i /= 10;
  }
  return ((i == d));
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, s);
  var i: dynamic = 0;
  var j: dynamic = 0;
  var sum: dynamic = 0;
  var c: dynamic = 0;
  {
    i = s;
    while ((i <= n))
    {
      j = i;
      sum = 0;
      while ((j > 0))
      {
        sum += (j % 10);
        j /= 10;
      }
      if ((((i - sum)) >= s))
      {
        break;
      }
      i += 1;
    }
  }
  write(((((n - i) + 1)) * (((((n - i) + 1)) > 0))), "\n");
}
