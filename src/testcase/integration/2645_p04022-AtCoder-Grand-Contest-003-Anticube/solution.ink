// Translated from solution.cpp.

var a = cpp_array(100010);

var b = cpp_array(100010);

var n: dynamic;

var ans: dynamic;

var mp: dynamic;

func sqr(x: dynamic)
{
  return (x * x);
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      scanf("%lld", (&x));
      {
        var j = 2;
        while ((((j * j) * j) <= x))
        {
          {
            while (((x % (((j * j) * j))) == 0))
            {
              x /= ((j * j) * j);
            }
          }
          j += 1;
        }
      }
      mp[x] += 1;
      a[i] = x;
      var y = 1;
      {
        var j = 2;
        while ((((j * j) * j) <= x))
        {
          if (((x % j) == 0))
          {
            y *= if ((((x % ((j * j))) == 0))) j else (j * j);
            {
              while (((x % j) == 0))
              {
                x /= j;
              }
            }
          }
          j += 1;
        }
      }
      if ((sqr(cpp_cast(sqrt(x))) == x))
      {
        y *= cpp_cast(sqrt(x));
      } else
      {
        y *= (x * x);
      }
      b[i] = y;
      i += 1;
    }
  }
  if (mp[1])
  {
    ans += 1;
    mp[1] = 0;
  }
  {
    var i = 1;
    while ((i <= n))
    {
      ans += max(mp[a[i]], mp[b[i]]);
      mp[a[i]] = cpp_assign(mp[b[i]], "=", 0);
      i += 1;
    }
  }
  printf("%d", ans);
  return 0;
}
