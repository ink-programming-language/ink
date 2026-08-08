// Translated from solution.cpp.

func po(x: dynamic, n: dynamic, mo: dynamic)
{
  var s = 1;
  var m = x;
  while (n)
  {
    if ((n & cpp_cast(1)))
    {
      s = (((s * m)) % mo);
    }
    m = (((m * m)) % mo);
    n >>= 1;
  }
  return (((mo + s)) % mo);
}

func ou(x: dynamic)
{
  var ans = 1;
  {
    var i = 2;
    while ((i <= sqrt(x)))
    {
      if (((x % i) == 0))
      {
        ans *= ((i - 1));
        x /= i;
        while (((x % i) == 0))
        {
          ans *= i;
          x /= i;
        }
      }
      i += 1;
    }
  }
  if ((x > 1))
  {
    ans *= ((x - 1));
  }
  return ans;
}

var b = cpp_array(1000009);

var n = cpp_array(1000009);

var c: dynamic;

func main()
{
  while ((scanf("%s%s%lld", b, n, (&c)) != EOF))
  {
    var rb = 0;
    var l1 = strlen(b);
    var l2 = strlen(n);
    {
      var i = 0;
      while ((i < l1))
      {
        rb = (((((rb * 10) + b[i]) - cpp_char("0"))) % c);
        i += 1;
      }
    }
    var rn = 0;
    var ol = ou(c);
    var flag = 0;
    {
      var i = 0;
      while ((i < l2))
      {
        rn = ((((rn * 10) + n[i]) - cpp_char("0")));
        if ((rn > c))
        {
          flag = 1;
          break;
        }
        i += 1;
      }
    }
    if (flag)
    {
      var ans1 = 0;
      {
        var i = 0;
        while ((i < l2))
        {
          ans1 = (((((ans1 * 10) + n[i]) - cpp_char("0"))) % ol);
          i += 1;
        }
      }
      var ans2 = ((((ans1 - 1) + ol)) % ol);
      var pp = po(rb, (ans1 + ol), c);
      var qq = po(rb, (ans2 + ol), c);
      var ss = ((((pp - qq) + c)) % c);
      if (ss)
      {
        printf("%lld\n", ss);
      } else
      {
        printf("%lld\n", c);
      }
    } else
    {
      var nn = po(rb, rn, c);
      var mm = po(rb, (rn - 1), c);
      var ss = ((((nn - mm) + c)) % c);
      if (ss)
      {
        printf("%lld\n", ss);
      } else
      {
        printf("%lld\n", c);
      }
    }
  }
  return 0;
}
