// Translated from solution.cpp.

var INF: dynamic = (1e9 + 7);

var M: dynamic = 998244353;

var N: dynamic = (1e5 + 7);

var f: dynamic = cpp_array(N);

var inv: dynamic = cpp_array(N);

var fi: dynamic = cpp_array(N);

func INV(i: dynamic) -> dynamic
{
  if ((i == 1))
  {
    return 1;
  }
  return (M - (((cpp_cast(M) / i) * INV((M % i))) % M));
}

func C(n: dynamic, k: dynamic) -> dynamic
{
  return ((((cpp_cast(f[n]) * fi[k]) % M) * fi[(n - k)]) % M);
}

func H(n: dynamic, k: dynamic) -> dynamic
{
  if ((n == 0))
  {
    return (k == 0);
  }
  return C(((n + k) - 1), k);
}

func add(a: dynamic, b: dynamic) -> dynamic
{
  a += b;
  if ((a >= M))
  {
    a -= M;
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  f[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      f[i] = ((cpp_cast(f[(i - 1)]) * i) % M);
      i += 1;
    }
  }
  inv[1] = 1;
  {
    var i: dynamic = 2;
    while ((i < N))
    {
      inv[i] = (M - (((cpp_cast(M) / i) * inv[(M % i)]) % M));
      i += 1;
    }
  }
  fi[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      fi[i] = ((cpp_cast(fi[(i - 1)]) * inv[i]) % M);
      i += 1;
    }
  }
  var p: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  read(p, s, r);
  if ((p == 1))
  {
    return cpp_comma(((cout << 1) << endl), 0);
  }
  var go: dynamic = __cpp_lambda_1;
  var yes: dynamic = 0;
  var all: dynamic = 0;
  {
    var score: dynamic = r;
    while ((score <= s))
    {
      var tot_ways: dynamic = H((p - 1), (s - score));
      add(all, tot_ways);
      {
        var same: dynamic = 0;
        while ((same <= (p - 1)))
        {
          var rem_sum: dynamic = (s - (((same + 1)) * score));
          var rem_cnt: dynamic = ((p - 1) - same);
          if ((rem_sum < 0))
          {
            same += 1;
            continue;
          }
          var ways: dynamic = ((cpp_cast(go(score, rem_cnt, rem_sum)) * C((p - 1), same)) % M);
          ways = ((cpp_cast(ways) * inv[(same + 1)]) % M);
          add(yes, ways);
          same += 1;
        }
      }
      score += 1;
    }
  }
  var ans: dynamic = ((cpp_cast(yes) * INV(all)) % M);
  write(ans, "\n");
}

func __cpp_lambda_1(score: dynamic, cnt: dynamic, sum: dynamic) -> dynamic
{
  if ((cnt == 0))
  {
    return  (sum) ? 0 : 1;
  }
  if ((score == 0))
  {
    return  (cnt) ? 0 : 1;
  }
  var ans: dynamic = 0;
  {
    var illegal: dynamic = 0;
    while ((illegal <= cnt))
    {
      var ways: dynamic = C(cnt, illegal);
      var rem_sum: dynamic = (sum - (score * illegal));
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
