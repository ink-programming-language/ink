// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var K: dynamic = cpp_uninitialized();

var Q: dynamic = cpp_uninitialized();

var freq: dynamic = cpp_array(1000010);

var F: dynamic = cpp_array(1000010);

var inv: dynamic = cpp_array(1000010);

var Finv: dynamic = cpp_array(1000010);

var C: dynamic = cpp_array(1000010);

var coef: dynamic = cpp_array(1000010);

var minp: dynamic = cpp_array(1000010);

func query(x: dynamic) -> dynamic
{
  var ans: dynamic = ((C[(freq[x] + 1)] - C[freq[x]]) + 1000000007);
  if ((ans >= 1000000007))
  {
    ans -= 1000000007;
  }
  ans = ((ans * coef[x]) % 1000000007);
  freq[x] += 1;
  return ans;
}

func main(argument_0: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
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
      var n: dynamic = i;
      var x: dynamic = i;
      while ((n > 1))
      {
        var p: dynamic = minp[n];
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
  var ans: dynamic = 0;
  {
    (i) = 0;
    while (((i) < cpp_cast(((N + Q)))))
    {
      var x: dynamic = cpp_uninitialized();
      scanf("%d", (&x));
      {
        var d: dynamic = 1;
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
