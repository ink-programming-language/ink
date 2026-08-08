// Translated from solution.cpp.

var PI = acos(-1.0);

var maxn = (1e5 + 5);

var Inf = (1e9 + 7);

var n: dynamic;

var a = cpp_array(maxn);

var isprime = cpp_array(maxn);

var prime = cpp_array(maxn);

var pnum = 0;

var ans = cpp_array(maxn);

func elect_Prime()
{
  memset(isprime, 1, cpp_sizeof((isprime)));
  {
    var i = 2;
    while ((i < (1e5 + 2)))
    {
      if (isprime[i])
      {
        prime[cpp_update(pnum, "++")] = i;
      }
      {
        var j = 0;
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

func main()
{
  while ((scanf("%d", (&n)) != EOF))
  {
    var x: dynamic;
    memset(ans, 0, cpp_sizeof((ans)));
    {
      var i = 1;
      while ((i <= n))
      {
        scanf("%d", (&x));
        {
          var j = 1;
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
    var num = 1;
    {
      var i = 2;
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
