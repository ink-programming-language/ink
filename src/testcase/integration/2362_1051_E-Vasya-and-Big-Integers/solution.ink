// Translated from solution.cpp.

var N = 1000005;

var M = 1000005;

var MOD = (1e9 + 7);

var eps = 1e-9;

var n: dynamic;

var len = cpp_array(2);

var dp = cpp_array(N);

var sum = cpp_array(N);

var mod = 998244353;

var a = cpp_array(N);

var l = cpp_array(N);

var r = cpp_array(N);

var z = cpp_array(2);

var s = cpp_array(2);

func ZAlgorithm(s: dynamic)
{
  var n = s.size();
  var l = 0;
  var r = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((i > r))
      {
        l = i;
        r = i;
        while (((r < n) && (s[(r - i)] == s[r])))
        {
          r += 1;
        }
        r -= 1;
        z[i] = ((r - l) + 1);
      } else
      {
        if ((((r - i) + 1) > z[(i - l)]))
        {
          z[i] = z[(i - l)];
        } else
        {
          l = i;
          while (((r < n) && (s[(r - i)] == s[r])))
          {
            r += 1;
          }
          r -= 1;
          z[i] = ((r - l) + 1);
        }
      }
      i += 1;
    }
  }
  return z;
}

func cmp(idx: dynamic, i: dynamic, j: dynamic)
{
  var ln = ((j - i) + 1);
  if ((ln != len[idx]))
  {
    return (if ((ln < len[idx])) -1 else +1);
  }
  var at = z[idx][((len[idx] + 1) + i)];
  if ((at == ln))
  {
    return 0;
  }
  return if ((a[(i + at)] < s[idx][at])) -1 else +1;
}

func main()
{
  scanf("%s%s%s", a, l, r);
  n = strlen(a);
  s[0] = ((string_cpp(l) + "#") + string_cpp(a));
  s[1] = ((string_cpp(r) + "#") + string_cpp(a));
  len[0] = strlen(l);
  len[1] = strlen(r);
  z[0] = ZAlgorithm(s[0]);
  z[1] = ZAlgorithm(s[1]);
  dp[n] = 1;
  sum[n] = 1;
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      if ((a[i] == cpp_char("0")))
      {
        dp[i] = (if ((l[0] == cpp_char("0"))) dp[(i + 1)] else 0);
        sum[i] = (((sum[(i + 1)] + dp[i])) % mod);
        i -= 1;
        continue;
      }
      var L = -1;
      var R = -1;
      var l = i;
      var r = (n - 1);
      var m: dynamic;
      while ((l <= r))
      {
        m = (((l + r)) / 2);
        if ((cmp(0, i, m) >= 0))
        {
          L = m;
          r = (m - 1);
        } else
        {
          l = (m + 1);
        }
      }
      l = i;
      r = (n - 1);
      while ((l <= r))
      {
        m = (((l + r)) / 2);
        if ((cmp(1, i, m) <= 0))
        {
          R = m;
          l = (m + 1);
        } else
        {
          r = (m - 1);
        }
      }
      R = min(R, (n - 1));
      L = min(L, (n - 1));
      if ((L <= R))
      {
        dp[i] = ((((sum[(L + 1)] - sum[(R + 2)]) + mod)) % mod);
      }
      sum[i] = (((sum[(i + 1)] + dp[i])) % mod);
      i -= 1;
    }
  }
  printf("%d\n", dp[0]);
  return 0;
}
