// Translated from solution.cpp.

var a: dynamic = cpp_array(100010);

var b: dynamic = cpp_array(100010);

var n: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var mp: dynamic = cpp_uninitialized();

func sqr(x: dynamic) -> dynamic
{
  return (x * x);
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = cpp_uninitialized();
      scanf("%lld", (&x));
      {
        var j: dynamic = 2;
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
      var y: dynamic = 1;
      {
        var j: dynamic = 2;
        while ((((j * j) * j) <= x))
        {
          if (((x % j) == 0))
          {
            y *=  ((((x % ((j * j))) == 0))) ? j : (j * j);
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
    var i: dynamic = 1;
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
