// Translated from solution.cpp.

var P: dynamic = 233;

var P2: dynamic = 131;

var maxn: dynamic = (1000000 + 3);

var MOD: dynamic = 1000000007;

func getint() -> dynamic
{
  var flag: dynamic = 0;
  var n: dynamic = 0;
  var ch: dynamic = getchar();
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
  return  (flag) ? ((-n)) : n;
}

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var now: dynamic = cpp_uninitialized();

var F: dynamic = cpp_array(maxn);

var F2: dynamic = cpp_array(maxn);

var s: dynamic = cpp_array(maxn);

var ans: dynamic = cpp_array(maxn);

var hash: dynamic = cpp_array(maxn);

var hash2: dynamic = cpp_array(maxn);

func Gethash(l: dynamic, r: dynamic) -> dynamic
{
  return ((((hash[r] - (((1 * hash[(l - 1)]) * F[((r - l) + 1)]) % MOD)) + MOD)) % MOD);
}

func Gethash2(l: dynamic, r: dynamic) -> dynamic
{
  return ((((hash2[r] - (((1 * hash2[(l - 1)]) * F2[((r - l) + 1)]) % MOD)) + MOD)) % MOD);
}

func main() -> dynamic
{
  F[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < maxn))
    {
      F[i] = (((1 * F[(i - 1)]) * P) % MOD);
      i += 1;
    }
  }
  F2[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < maxn))
    {
      F2[i] = (((1 * F2[(i - 1)]) * P2) % MOD);
      i += 1;
    }
  }
  n = getint();
  scanf("%s", (s + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      hash[i] = ((((((((1 * hash[(i - 1)]) * P) % MOD) + s[i]) - cpp_char("a")) + 1)) % MOD);
      hash2[i] = ((((((((1 * hash2[(i - 1)]) * P2) % MOD) + s[i]) - cpp_char("a")) + 1)) % MOD);
      i += 1;
    }
  }
  k = (((n + 1)) >> 1);
  now = 1;
  var cur: dynamic = k;
  while (cur)
  {
    var len: dynamic = ((((n - cur) + 1) - cur) + 1);
    now = min(now,  (((len & 1))) ? (len - 2) : (len - 1));
    while ((((now > -1) && (Gethash(cur, ((cur + now) - 1)) != Gethash(((((cur + len) - 1) - now) + 1), ((cur + len) - 1)))) && (Gethash2(cur, ((cur + now) - 1)) != Gethash2(((((cur + len) - 1) - now) + 1), ((cur + len) - 1)))))
    {
      now -= 2;
    }
    ans[cur] = now;
    cur -= 1;
    now += 2;
  }
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      printf("%d ", ans[i]);
      i += 1;
    }
  }
  return 0;
}
