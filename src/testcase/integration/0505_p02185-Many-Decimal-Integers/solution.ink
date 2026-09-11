// Translated from solution.cpp.

var MOD: dynamic = (1e9 + 7);

var INF: dynamic = 1e9;

var LINF: dynamic = 1e18;

var int_cpp: dynamic = dynamic;

func fin(a: dynamic) -> dynamic
{
  write(a, "\n");
  exit(0);
}

func pw(n: dynamic, k: dynamic) -> dynamic
{
  if ((k < 0))
  {
    return pw(n, ((k + MOD) - 1));
  }
  var res: dynamic = 1;
  while (k)
  {
    if ((k & 1))
    {
      res *= n;
    }
    res %= MOD;
    n *= n;
    n %= MOD;
    k >>= 1;
  }
  return res;
}

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(s, t);
  var N: dynamic = s.size();
  var v: dynamic = cpp_construct(223456);
  var a: dynamic = cpp_construct(223456);
  var w: dynamic = cpp_construct(11);
  w[0] = 0;
  {
    var i: dynamic = 1;
    while ((i <= 10))
    {
      w[i] = (w[(i - 1)] + i);
      i += 1;
    }
  }
  reverse(t.begin(), t.end());
  a[0] = 1;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if ((t[i] == cpp_char("?")))
      {
        a[(i + 1)] = ((a[i] * 10) % MOD);
      } else
      {
        a[(i + 1)] = a[i];
      }
      a[(i + 1)] %= MOD;
      i += 1;
    }
  }
  v[0] = 0;
  var now: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if ((t[i] == cpp_char("?")))
      {
        v[(i + 1)] = (((v[i] * 10) % MOD) + (((w[9] * now) % MOD) * a[i]));
      } else
      {
        v[(i + 1)] = ((v[i] * 1) + ((((((t[i] - cpp_char("0"))) * now) % MOD) * a[i]) % MOD));
      }
      now *= 10;
      now %= MOD;
      v[(i + 1)] %= MOD;
      i += 1;
    }
  }
  reverse(t.begin(), t.end());
  var gyaku: dynamic = pw(10, (MOD - 2));
  now = pw(10, (N - 1));
  var ans: dynamic = 0;
  var res: dynamic = 0;
  var flg: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var j: dynamic = (s[i] - cpp_char("0"));
      if ((t[i] == cpp_char("?")))
      {
        if ((j > 0))
        {
          ans += ((((res * j) % MOD) * a[((N - i) - 1)]) % MOD);
          ans += ((((w[(j - 1)] * now) % MOD) * a[((N - i) - 1)]) % MOD);
          ans += ((v[((N - i) - 1)] * j) % MOD);
          ans %= MOD;
        }
        res += ((j * now) % MOD);
        res %= MOD;
        now *= gyaku;
        now %= MOD;
        i += 1;
        continue;
      }
      if ((s[i] < t[i]))
      {
        flg = 1;
        break;
      }
      if ((s[i] > t[i]))
      {
        ans += (res * a[((N - i) - 1)]);
        ans += ((((((t[i] - cpp_char("0"))) * now) % MOD) * a[((N - i) - 1)]) % MOD);
        ans += v[((N - i) - 1)];
        ans %= MOD;
        flg = 1;
        break;
      }
      assert((s[i] == t[i]));
      res += ((j * now) % MOD);
      res %= MOD;
      now *= gyaku;
      now %= MOD;
      i += 1;
    }
  }
  if ((!flg))
  {
    ans += res;
  }
  ans %= MOD;
  fin(ans);
}
