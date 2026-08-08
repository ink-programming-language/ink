// Translated from solution.cpp.

var MOD = (1e9 + 7);

var INF = 1e9;

var LINF = 1e18;

var int_cpp = dynamic;

func fin(a: dynamic)
{
  write(a, "\n");
  exit(0);
}

func pw(n: dynamic, k: dynamic)
{
  if ((k < 0))
  {
    return pw(n, ((k + MOD) - 1));
  }
  var res = 1;
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

func main()
{
  var s: dynamic;
  var t: dynamic;
  read(s, t);
  var N = s.size();
  var v = cpp_construct(223456);
  var a = cpp_construct(223456);
  var w = cpp_construct(11);
  w[0] = 0;
  {
    var i = 1;
    while ((i <= 10))
    {
      w[i] = (w[(i - 1)] + i);
      i += 1;
    }
  }
  reverse(t.begin(), t.end());
  a[0] = 1;
  {
    var i = 0;
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
  var now = 1;
  {
    var i = 0;
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
  var gyaku = pw(10, (MOD - 2));
  now = pw(10, (N - 1));
  var ans = 0;
  var res = 0;
  var flg = 0;
  {
    var i = 0;
    while ((i < N))
    {
      var j = (s[i] - cpp_char("0"));
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
