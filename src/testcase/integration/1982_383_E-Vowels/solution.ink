// Translated from solution.cpp.

var MAX = 10010;

var N = 24;

var inv = (((1 << N)) - 1);

var F = cpp_array((((1 << N)) + 10));

var ara = cpp_array(MAX);

func howManyZeroPairs(n: dynamic, ara: dynamic)
{
  memset(F, 0, cpp_sizeof((F)));
  {
    var i = 0;
    while ((i < n))
    {
      F[ara[i]] += 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      {
        var mask = 0;
        while ((mask < ((1 << N))))
        {
          if ((mask & ((1 << i))))
          {
            F[mask] += F[(mask ^ ((1 << i)))];
          }
          mask += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < ((1 << N))))
    {
      ans ^= ((((n - F[(i ^ inv)])) * ((n - F[(i ^ inv)]))));
      i += 1;
    }
  }
  return ans;
}

var str = cpp_array(5);

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%s", str);
      ara[i] = (ara[i] ^ ((1 << ((str[0] - cpp_char("a"))))));
      if ((!((ara[i] & ((1 << ((str[1] - cpp_char("a")))))))))
      {
        ara[i] = (ara[i] ^ ((1 << ((str[1] - cpp_char("a"))))));
      }
      if ((!((ara[i] & ((1 << ((str[2] - cpp_char("a")))))))))
      {
        ara[i] = (ara[i] ^ ((1 << ((str[2] - cpp_char("a"))))));
      }
      i += 1;
    }
  }
  write(howManyZeroPairs(n, ara), "\n");
  return 0;
}
