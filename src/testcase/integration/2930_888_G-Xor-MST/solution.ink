// Translated from solution.cpp.

var N = (1e5 + 7);

var MOD = (1e9 + 7);

var eps = 1e-8;

var Pi = acos(-1.0);

var E = exp(1.0);

func read()
{
  var x = 0;
  var f = 1;
  var ch = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((((x << 3)) + ((x << 1))) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * f);
}

func gcd(a: dynamic, b: dynamic)
{
  return if (((b == 0))) a else gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic)
{
  return ((a / gcd(a, b)) * b);
}

func qmod(a: dynamic, b: dynamic, c: dynamic)
{
  var ret = 1;
  while (b)
  {
    if ((b & 1))
    {
      ret = ((ret * a) % c);
    }
    b >>= 1;
    a = ((a * a) % c);
  }
  return ret;
}

var trie = cpp_array(3, (N * 40));

var sum = cpp_array((N * 40));

var val = cpp_array((N * 40));

var to = cpp_array((N * 40));

var cnt: dynamic;

var weishu = (30 - 1);

var ans: dynamic;

var tot: dynamic;

var ta: dynamic;

var tt: dynamic;

func insert(x: dynamic)
{
  var now = 0;
  {
    var i = weishu;
    var bt: dynamic;
    while ((i >= 0))
    {
      bt = (1 - ((0 == ((x & ((1 << i)))))));
      if ((!trie[now][bt]))
      {
        trie[now][bt] = cpp_update(cnt, "++");
      }
      now = trie[now][bt];
      sum[now] += 1;
      i -= 1;
    }
  }
  val[now] = x;
}

func query(x: dynamic, pre: dynamic)
{
  var now = 0;
  {
    var i = weishu;
    var bt: dynamic;
    while ((i >= 0))
    {
      bt = (1 - ((0 == ((x & ((1 << i)))))));
      if ((!trie[now][bt]))
      {
        bt = (1 - bt);
      }
      if ((now == pre))
      {
        bt = (1 - bt);
      }
      now = trie[now][bt];
      i -= 1;
    }
  }
  return now;
}

func dfs2(x: dynamic, pre: dynamic)
{
  if (trie[x][0])
  {
    dfs2(trie[x][0], pre);
  }
  if (trie[x][1])
  {
    dfs2(trie[x][1], pre);
  }
  if (((!trie[x][0]) && (!trie[x][1])))
  {
    var now = query(val[x], pre);
    if ((ta == ((val[now] ^ val[x]))))
    {
      tt = (((tt + (((1 * sum[now]) * sum[x]) % MOD))) % MOD);
    }
    if ((ta > ((val[now] ^ val[x]))))
    {
      ta = ((val[now] ^ val[x]));
      tt = ((((1 * sum[now]) * sum[x])) % MOD);
    }
  }
}

func dfs(x: dynamic)
{
  if (trie[x][0])
  {
    dfs(trie[x][0]);
  }
  if (trie[x][1])
  {
    dfs(trie[x][1]);
  }
  if ((trie[x][0] && trie[x][1]))
  {
    ta = ((~((1 << 31))));
    tt = 0;
    if ((to[trie[x][1]] < to[trie[x][0]]))
    {
      dfs2(trie[x][1], x);
    } else
    {
      dfs2(trie[x][0], x);
    }
    ans += ta;
    tot = ((tot * tt) % MOD);
  } else if ((((!trie[x][0]) && (!trie[x][1])) && (sum[x] > 1)))
  {
    tot = ((tot * qmod((1 * sum[x]), ((1 * sum[x]) - 2), (1 * MOD))) % MOD);
  }
}

func dfs1(x: dynamic)
{
  if (((!trie[x][0]) && (!trie[x][1])))
  {
    return cpp_assign(to[x], "=", 1);
  }
  if (trie[x][0])
  {
    to[x] += dfs1(trie[x][0]);
  }
  if (trie[x][1])
  {
    to[x] += dfs1(trie[x][1]);
  }
  return to[x];
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  cnt = 0;
  ans = 0;
  tot = 1;
  {
    var i = 1;
    var x: dynamic;
    while ((i <= n))
    {
      x = read();
      insert(x);
      i += 1;
    }
  }
  dfs1(0);
  dfs(0);
  printf("%lld\n", ans);
  return 0;
}
