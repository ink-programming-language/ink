// Translated from solution.cpp.

var PI: dynamic = acos(-1.0);

var maxn: dynamic = (1e5 + 5);

var Inf: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

var isprime: dynamic = cpp_array(maxn);

var prime: dynamic = cpp_array(maxn);

var pnum: dynamic = 0;

var ans: dynamic = cpp_array(maxn);

func elect_Prime() -> dynamic
{
  memset(isprime, 1, cpp_sizeof((isprime)));
  {
    var i: dynamic = 2;
    while ((i < (1e5 + 2)))
    {
      if (isprime[i])
      {
        prime[cpp_update(pnum, "++")] = i;
      }
      {
        var j: dynamic = 0;
        while (((j < pnum) && ((i * prime[j]) < (1e5 + 2))))
        {
          isprime[(i * prime[j])] = 0;
          if (((i % prime[j]) == 0))
          {
            break;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  while ((scanf("%d", (&n)) != EOF))
  {
    var x: dynamic = cpp_uninitialized();
    memset(ans, 0, cpp_sizeof((ans)));
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        scanf("%d", (&x));
        {
          var j: dynamic = 1;
          while (((j * j) <= x))
          {
            if (((x % j) == 0))
            {
              ans[j] += 1;
              if (((j * j) != x))
              {
                ans[(x / j)] += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    var num: dynamic = 1;
    {
      var i: dynamic = 2;
      while ((i < maxn))
      {
        num = max(num, ans[i]);
        i += 1;
      }
    }
    printf("%d\n", num);
  }
  return 0;
}
