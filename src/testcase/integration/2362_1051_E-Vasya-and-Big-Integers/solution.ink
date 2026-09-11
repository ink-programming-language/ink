// Translated from solution.cpp.

var N: dynamic = 1000005;

var M: dynamic = 1000005;

var MOD: dynamic = (1e9 + 7);

var eps: dynamic = 1e-9;

var n: dynamic = cpp_uninitialized();

var len: dynamic = cpp_array(2);

var dp: dynamic = cpp_array(N);

var sum: dynamic = cpp_array(N);

var mod: dynamic = 998244353;

var a: dynamic = cpp_array(N);

var l: dynamic = cpp_array(N);

var r: dynamic = cpp_array(N);

var z: dynamic = cpp_array(2);

var s: dynamic = cpp_array(2);

func ZAlgorithm(s: dynamic) -> dynamic
{
  var n: dynamic = s.size();
  var l: dynamic = 0;
  var r: dynamic = 0;
  {
    var i: dynamic = 0;
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

func cmp(idx: dynamic, i: dynamic, j: dynamic) -> dynamic
{
  var ln: dynamic = ((j - i) + 1);
  if ((ln != len[idx]))
  {
    return ( ((ln < len[idx])) ? -1 : +1);
  }
  var at: dynamic = z[idx][((len[idx] + 1) + i)];
  if ((at == ln))
  {
    return 0;
  }
  return  ((a[(i + at)] < s[idx][at])) ? -1 : +1;
}

func main() -> dynamic
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
    var i: dynamic = (n - 1);
    while ((i >= 0))
    {
      if ((a[i] == cpp_char("0")))
      {
        dp[i] = ( ((l[0] == cpp_char("0"))) ? dp[(i + 1)] : 0);
        sum[i] = (((sum[(i + 1)] + dp[i])) % mod);
        i -= 1;
        continue;
      }
      var L: dynamic = -1;
      var R: dynamic = -1;
      var l: dynamic = i;
      var r: dynamic = (n - 1);
      var m: dynamic = cpp_uninitialized();
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
