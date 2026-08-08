// Translated from solution.cpp.

var P = 233;

var P2 = 131;

var maxn = (1000000 + 3);

var MOD = 1000000007;

func getint()
{
  var flag = 0;
  var n = 0;
  var ch = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      flag = 1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    n = (((ch - cpp_char("0")) + ((n << 3))) + ((n << 1)));
    ch = getchar();
  }
  return if (flag) ((-n)) else n;
}

var n: dynamic;

var k: dynamic;

var now: dynamic;

var F = cpp_array(maxn);

var F2 = cpp_array(maxn);

var s = cpp_array(maxn);

var ans = cpp_array(maxn);

var hash = cpp_array(maxn);

var hash2 = cpp_array(maxn);

func Gethash(l: dynamic, r: dynamic)
{
  return ((((hash[r] - (((1 * hash[(l - 1)]) * F[((r - l) + 1)]) % MOD)) + MOD)) % MOD);
}

func Gethash2(l: dynamic, r: dynamic)
{
  return ((((hash2[r] - (((1 * hash2[(l - 1)]) * F2[((r - l) + 1)]) % MOD)) + MOD)) % MOD);
}

func main()
{
  F[0] = 1;
  {
    var i = 1;
    while ((i < maxn))
    {
      F[i] = (((1 * F[(i - 1)]) * P) % MOD);
      i += 1;
    }
  }
  F2[0] = 1;
  {
    var i = 1;
    while ((i < maxn))
    {
      F2[i] = (((1 * F2[(i - 1)]) * P2) % MOD);
      i += 1;
    }
  }
  n = getint();
  scanf("%s", (s + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      hash[i] = ((((((((1 * hash[(i - 1)]) * P) % MOD) + s[i]) - cpp_char("a")) + 1)) % MOD);
      hash2[i] = ((((((((1 * hash2[(i - 1)]) * P2) % MOD) + s[i]) - cpp_char("a")) + 1)) % MOD);
      i += 1;
    }
  }
  k = (((n + 1)) >> 1);
  now = 1;
  var cur = k;
  while (cur)
  {
    var len = ((((n - cur) + 1) - cur) + 1);
    now = min(now, if (((len & 1))) (len - 2) else (len - 1));
    while ((((now > -1) && (Gethash(cur, ((cur + now) - 1)) != Gethash(((((cur + len) - 1) - now) + 1), ((cur + len) - 1)))) && (Gethash2(cur, ((cur + now) - 1)) != Gethash2(((((cur + len) - 1) - now) + 1), ((cur + len) - 1)))))
    {
      now -= 2;
    }
    ans[cur] = now;
    cur -= 1;
    now += 2;
  }
  {
    var i = 1;
    while ((i <= k))
    {
      printf("%d ", ans[i]);
      i += 1;
    }
  }
  return 0;
}
