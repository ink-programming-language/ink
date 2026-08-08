// Translated from solution.cpp.

var M = (1e9 + 7);

var pi = acos(-1.0);

func powerm(x: dynamic, y: dynamic)
{
  var res = 1;
  while (y)
  {
    if ((y & 1))
    {
      res = (((res * x)) % M);
    }
    y = (y >> 1);
    x = (((x * x)) % M);
  }
  return (res % M);
}

func power(x: dynamic, y: dynamic)
{
  var res = 1;
  while (y)
  {
    if ((y & 1))
    {
      res = ((res * x));
    }
    y = (y >> 1);
    x = ((x * x));
  }
  return res;
}

func max3(a: dynamic, b: dynamic, c: dynamic)
{
  return max(max(a, b), c);
}

func gcd(a: dynamic, b: dynamic)
{
  if ((b > a))
  {
    return gcd(b, a);
  }
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  read(n);
  var a = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ans = 0;
  {
    var val = 1;
    while ((val <= 30))
    {
      var tmp = 0;
      var h = 0;
      {
        var i = 0;
        while ((i < n))
        {
          if ((a[i] <= val))
          {
            tmp += a[i];
          }
          if ((tmp < 0))
          {
            tmp = 0;
          }
          h = max(h, tmp);
          i += 1;
        }
      }
      ans = max(ans, (h - val));
      val += 1;
    }
  }
  write(ans);
  return 0;
}
