// Translated from solution.cpp.

var t: dynamic;

var p = cpp_array(200005);

var s = cpp_construct(5000005, 0);

var d = cpp_construct(5000005, 0);

var mp: dynamic;

var q: dynamic;

var mxn = 200005;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var v: dynamic;

func gcd(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func bpow(a: dynamic, b: dynamic)
{
  var res = 1;
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

func fact(n: dynamic)
{
  if ((n == 0))
  {
    return 1;
  }
  return (n * fact((n - 1)));
}

func isprime(n: dynamic)
{
  {
    var i = 2;
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

func prime()
{
  {
    var i = 2;
    while ((i <= 5000000))
    {
      if ((s[i] == 0))
      {
        {
          var j = 2;
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

func pal(i: dynamic)
{
  var j = i;
  var c = 0;
  var d = 0;
  while ((j > 0))
  {
    c += 1;
    j /= 10;
  }
  {
    var j = 1;
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic;
  var s: dynamic;
  read(n, s);
  var i = 0;
  var j = 0;
  var sum = 0;
  var c = 0;
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
