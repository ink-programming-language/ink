// Translated from solution.cpp.

func po(x: dynamic, n: dynamic, mo: dynamic) -> dynamic
{
  var s: dynamic = 1;
  var m: dynamic = x;
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

func ou(x: dynamic) -> dynamic
{
  var ans: dynamic = 1;
  {
    var i: dynamic = 2;
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

var b: dynamic = cpp_array(1000009);

var n: dynamic = cpp_array(1000009);

var c: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  while ((scanf("%s%s%lld", b, n, (&c)) != EOF))
  {
    var rb: dynamic = 0;
    var l1: dynamic = strlen(b);
    var l2: dynamic = strlen(n);
    {
      var i: dynamic = 0;
      while ((i < l1))
      {
        rb = (((((rb * 10) + b[i]) - cpp_char("0"))) % c);
        i += 1;
      }
    }
    var rn: dynamic = 0;
    var ol: dynamic = ou(c);
    var flag: dynamic = 0;
    {
      var i: dynamic = 0;
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
      var ans1: dynamic = 0;
      {
        var i: dynamic = 0;
        while ((i < l2))
        {
          ans1 = (((((ans1 * 10) + n[i]) - cpp_char("0"))) % ol);
          i += 1;
        }
      }
      var ans2: dynamic = ((((ans1 - 1) + ol)) % ol);
      var pp: dynamic = po(rb, (ans1 + ol), c);
      var qq: dynamic = po(rb, (ans2 + ol), c);
      var ss: dynamic = ((((pp - qq) + c)) % c);
      if (ss)
      {
        printf("%lld\n", ss);
      } else
      {
        printf("%lld\n", c);
      }
    } else
    {
      var nn: dynamic = po(rb, rn, c);
      var mm: dynamic = po(rb, (rn - 1), c);
      var ss: dynamic = ((((nn - mm) + c)) % c);
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
