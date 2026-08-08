// Translated from solution.cpp.

var N: dynamic;

var K: dynamic;

var Q: dynamic;

var freq = cpp_array(1000010);

var F = cpp_array(1000010);

var inv = cpp_array(1000010);

var Finv = cpp_array(1000010);

var C = cpp_array(1000010);

var coef = cpp_array(1000010);

var minp = cpp_array(1000010);

func query(x: dynamic)
{
  var ans = ((C[(freq[x] + 1)] - C[freq[x]]) + 1000000007);
  if ((ans >= 1000000007))
  {
    ans -= 1000000007;
  }
  ans = ((ans * coef[x]) % 1000000007);
  freq[x] += 1;
  return ans;
}

func main(argument_0: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  inv[1] = 1;
  {
    i = 2;
    while ((i < 1000010))
    {
      inv[i] = ((((1000000007 - (1000000007 / i))) * inv[(1000000007 % i)]) % 1000000007);
      i += 1;
    }
  }
  F[0] = cpp_assign(Finv[0], "=", 1);
  {
    i = 1;
    while ((i < 1000010))
    {
      F[i] = ((F[(i - 1)] * i) % 1000000007);
      i += 1;
    }
  }
  {
    i = 1;
    while ((i < 1000010))
    {
      Finv[i] = ((Finv[(i - 1)] * inv[i]) % 1000000007);
      i += 1;
    }
  }
  read(N, K, Q);
  {
    (i) = 0;
    while (((i) < cpp_cast((1000010))))
    {
      if ((i >= K))
      {
        C[i] = ((((F[i] * Finv[K]) % 1000000007) * Finv[(i - K)]) % 1000000007);
      }
      (i) += 1;
    }
  }
  {
    i = 2;
    while ((i < 1000010))
    {
      minp[i] = i;
      i += 1;
    }
  }
  {
    i = 2;
    while ((i < 1000010))
    {
      if ((minp[i] == i))
      {
        {
          j = (2 * i);
          while ((j < 1000010))
          {
            minp[j] = min(minp[j], i);
            j += i;
          }
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i < 1000010))
    {
      var n = i;
      var x = i;
      while ((n > 1))
      {
        var p = minp[n];
        while (((n % p) == 0))
        {
          n /= p;
        }
        x -= (x / p);
      }
      coef[i] = x;
      i += 1;
    }
  }
  var ans = 0;
  {
    (i) = 0;
    while (((i) < cpp_cast(((N + Q)))))
    {
      var x: dynamic;
      scanf("%d", (&x));
      {
        var d = 1;
        while (((d * d) <= x))
        {
          if (((x % d) == 0))
          {
            ans += query(d);
            if (((d * d) != x))
            {
              ans += query((x / d));
            }
          }
          d += 1;
        }
      }
      ans %= 1000000007;
      if ((i >= N))
      {
        printf("%I64d\n", ans);
      }
      (i) += 1;
    }
  }
  return 0;
}
