// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var size: dynamic = cpp_array((200000 + 5));

var dep: dynamic = cpp_array((200000 + 5));

var pre: dynamic = cpp_array((200000 + 5));

var le: dynamic = cpp_array((200000 + 5));

var ri: dynamic = cpp_array((200000 + 5));

var son: dynamic = cpp_array(4, (200000 + 5));

var e: dynamic = cpp_array((200000 + 5));

var a: dynamic = cpp_array(4);

var dp: dynamic = cpp_array((200000 + 5));

func dfs(u: dynamic, fa: dynamic) -> dynamic
{
  size[u] = 1;
  {
    var i: dynamic = 0;
    while ((i < e[u]))
    {
      var v: dynamic = son[u][i];
      if ((v == fa))
      {
        i += 1;
        continue;
      }
      dfs(v, u);
      size[u] += size[v];
      pre[u] = pre[v];
      dep[u] = (dep[v] + 1);
      if ((!le[u]))
      {
        le[u] = v;
      } else
      {
        ri[u] = v;
      }
      i += 1;
    }
  }
  if (ri[u])
  {
    pre[u] = u;
    dep[u] = 0;
  }
}

func getAns(u: dynamic) -> dynamic
{
  if ((!u))
  {
    return 1;
  }
  if ((size[u] & 1))
  {
    return 0;
  }
  if ((!pre[u]))
  {
    return (size[u] >> 1);
  }
  if ((dp[u] != -1))
  {
    return dp[u];
  }
  var p: dynamic = pre[u];
  var l: dynamic = le[p];
  var r: dynamic = ri[p];
  var sum: dynamic = 0;
  dp[u] = 0;
  {
    var i: dynamic = 0;
    while ((i < 2))
    {
      if ((!ri[r]))
      {
        if (((!((dep[u] & 1))) && (size[r] > 1)))
        {
          sum += cal(l, le[r]);
        }
        if ((((!pre[r]) && (dep[r] <= dep[u])) && (!(((dep[r] + dep[u]) & 1)))))
        {
          sum += (getAns(l) * ( (((dep[u] - dep[r]) >= 2)) ? 2 : 1));
        }
        sum %= 1000000007;
      } else
      {
        if ((((!pre[le[r]]) && (dep[le[r]] < dep[u])) && (((dep[u] + dep[le[r]]) & 1))))
        {
          sum += cal(l, ri[r]);
        }
        if ((((!pre[ri[r]]) && (dep[ri[r]] < dep[u])) && (((dep[u] + dep[ri[r]])) & 1)))
        {
          sum += cal(l, le[r]);
        }
        sum %= 1000000007;
      }
      i += 1;
      swap(l, r);
    }
  }
  return cpp_assign(dp[u], "=", (sum % 1000000007));
}

func cal(u: dynamic, v: dynamic) -> dynamic
{
  if (((!u) || (!v)))
  {
    return getAns((u + v));
  }
  if ((ri[u] || ri[v]))
  {
    return 0;
  }
  if ((le[u] && le[v]))
  {
    return cal(le[u], le[v]);
  }
  return getAns((le[u] + le[v]));
}

func main() -> dynamic
{
  scanf("%d", (&n));
  n <<= 1;
  var rt: dynamic = 0;
  var flag: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d%d", (&u), (&v));
      if (((e[u] == 3) || (e[v] == 3)))
      {
        flag = 1;
        break;
      }
      son[u][cpp_update(e[u], "++")] = v;
      son[v][cpp_update(e[v], "++")] = u;
      if ((e[u] == 3))
      {
        rt = u;
      }
      if ((e[v] == 3))
      {
        rt = v;
      }
      i += 1;
    }
  }
  if (flag)
  {
    puts("0");
    return 0;
  }
  if ((!rt))
  {
    printf("%lld\n",  ((n == 2)) ? 2 : (((((1 * n) * (((n / 2) - 1))) + 4)) % 1000000007));
    return 0;
  }
  var ans: dynamic = 0;
  dfs(rt, 0);
  a[0] = 0;
  a[1] = 1;
  a[2] = 2;
  memset(dp, -1, cpp_sizeof((dp)));
  while (true)
  {
    var l: dynamic = son[rt][a[0]];
    var m: dynamic = son[rt][a[1]];
    var r: dynamic = son[rt][a[2]];
    var u: dynamic = le[m];
    var v: dynamic = ri[m];
    ans += ((cal(l, u) * cal(r, v)) % 1000000007);
    ans %= 1000000007;
    if ((u || v))
    {
      ans += ((cal(l, v) * cal(r, u)) % 1000000007);
      ans %= 1000000007;
    }
    if (!((next_permutation(a, (a + 3)))))
    {
      break;
    }
  }
  printf("%lld\n", ((ans * 2) % 1000000007));
  return 0;
}
