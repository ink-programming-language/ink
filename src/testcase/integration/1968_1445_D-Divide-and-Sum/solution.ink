// Translated from solution.cpp.

var p = 998244353;

func Denominator(a: dynamic)
{
  var i = p;
  var v = 0;
  var d = 1;
  if ((a == 0))
  {
    return 1;
  }
  while ((a > 0))
  {
    var t = (i / a);
    var x = a;
    a = (i % x);
    i = x;
    x = d;
    d = (v - (t * x));
    v = x;
  }
  v %= p;
  if ((v < 0))
  {
    v = (((v + p)) % p);
  }
  return v;
}

func main()
{
  var n: dynamic;
  var res = 1;
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      res *= ((n + i));
      res = (res % p);
      res *= Denominator(i);
      res = (res % p);
      i += 1;
    }
  }
  var sum = 0;
  var a = cpp_construct((2 * n));
  {
    var i = 0;
    while ((i < (2 * n)))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a.begin(), a.end());
  {
    var i = 0;
    while ((i < n))
    {
      sum += (a[(((2 * n) - 1) - i)] - a[i]);
      i += 1;
    }
  }
  write(((res * ((sum % p))) % p), "\n");
  return 0;
}
