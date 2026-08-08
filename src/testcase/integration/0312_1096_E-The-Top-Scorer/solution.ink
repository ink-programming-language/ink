// Translated from solution.cpp.

var INF = (1e9 + 7);

var M = 998244353;

var N = (1e5 + 7);

var f = cpp_array(N);

var inv = cpp_array(N);

var fi = cpp_array(N);

func INV(i: dynamic)
{
  if ((i == 1))
  {
    return 1;
  }
  return (M - (((cpp_cast(M) / i) * INV((M % i))) % M));
}

func C(n: dynamic, k: dynamic)
{
  return ((((cpp_cast(f[n]) * fi[k]) % M) * fi[(n - k)]) % M);
}

func H(n: dynamic, k: dynamic)
{
  if ((n == 0))
  {
    return (k == 0);
  }
  return C(((n + k) - 1), k);
}

func add(a: dynamic, b: dynamic)
{
  a += b;
  if ((a >= M))
  {
    a -= M;
  }
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  f[0] = 1;
  {
    var i = 1;
    while ((i < N))
    {
      f[i] = ((cpp_cast(f[(i - 1)]) * i) % M);
      i += 1;
    }
  }
  inv[1] = 1;
  {
    var i = 2;
    while ((i < N))
    {
      inv[i] = (M - (((cpp_cast(M) / i) * inv[(M % i)]) % M));
      i += 1;
    }
  }
  fi[0] = 1;
  {
    var i = 1;
    while ((i < N))
    {
      fi[i] = ((cpp_cast(fi[(i - 1)]) * inv[i]) % M);
      i += 1;
    }
  }
  var p: dynamic;
  var s: dynamic;
  var r: dynamic;
  read(p, s, r);
  if ((p == 1))
  {
    return cpp_comma(((cout << 1) << endl), 0);
  }
  var go = __cpp_lambda_1;
  var yes = 0;
  var all = 0;
  {
    var score = r;
    while ((score <= s))
    {
      var tot_ways = H((p - 1), (s - score));
      add(all, tot_ways);
      {
        var same = 0;
        while ((same <= (p - 1)))
        {
          var rem_sum = (s - (((same + 1)) * score));
          var rem_cnt = ((p - 1) - same);
          if ((rem_sum < 0))
          {
            same += 1;
            continue;
          }
          var ways = ((cpp_cast(go(score, rem_cnt, rem_sum)) * C((p - 1), same)) % M);
          ways = ((cpp_cast(ways) * inv[(same + 1)]) % M);
          add(yes, ways);
          same += 1;
        }
      }
      score += 1;
    }
  }
  var ans = ((cpp_cast(yes) * INV(all)) % M);
  write(ans, "\n");
}

func __cpp_lambda_1(score: dynamic, cnt: dynamic, sum: dynamic)
{
  if ((cnt == 0))
  {
    return if (sum) 0 else 1;
  }
  if ((score == 0))
  {
    return if (cnt) 0 else 1;
  }
  var ans = 0;
  {
    var illegal = 0;
    while ((illegal <= cnt))
    {
      var ways = C(cnt, illegal);
      var rem_sum = (sum - (score * illegal));
      if ((rem_sum < 0))
      {
        illegal += 1;
        continue;
      }
      ways = ((cpp_cast(ways) * H(cnt, rem_sum)) % M);
      if ((illegal & 1))
      {
        ways = (M - ways);
      }
      add(ans, ways);
      illegal += 1;
    }
  }
  return ans;
}
