// Translated from solution.cpp.

var maxn = cpp_cast(4e5);

class state
{
  var next: dynamic = cpp_array(37);
  var len: dynamic;
  var suff: dynamic;
  func state()
  {
      memset(next, -1, cpp_sizeof((next)));
      suff = -1;
      len = 0;
    }
}

var st = cpp_array((2 * maxn));

var sz = 1;

var last = 0;

var g = cpp_array(maxn);

var second = cpp_array(maxn);

var L = cpp_array(maxn);

var R = cpp_array(maxn);

var n: dynamic;

var dp = cpp_array(11, (maxn * 3));

var cnt = cpp_array((maxn * 2));

var was = cpp_array((maxn * 2));

var ans: dynamic;

func addAutomat(ch: dynamic)
{
  var p = last;
  var nv = cpp_update(sz, "++");
  st[nv].len = (st[last].len + 1);
  memset(st[nv].next, -1, cpp_sizeof((st[nv].next)));
  {
    while (((p != -1) && (st[p].next[ch] == -1)))
    {
      st[p].next[ch] = nv;
      p = st[p].suff;
    }
  }
  if ((p == -1))
  {
    st[nv].suff = 0;
  } else
  {
    var q = st[p].next[ch];
    if ((st[q].len == (st[p].len + 1)))
    {
      st[nv].suff = q;
    } else
    {
      var clone = cpp_update(sz, "++");
      memcpy(st[clone].next, st[q].next, cpp_sizeof((st[clone].next)));
      st[clone].suff = st[q].suff;
      st[clone].len = (st[p].len + 1);
      {
        while (((p != -1) && (st[p].next[ch] == q)))
        {
          st[p].next[ch] = clone;
          p = st[p].suff;
        }
      }
      st[nv].suff = cpp_assign(st[q].suff, "=", clone);
    }
  }
  last = nv;
}

func Count(v: dynamic)
{
  if ((v == 0))
  {
    return 1;
  }
  if ((cnt[v] != -1))
  {
    return cnt[v];
  }
  cnt[v] = 0;
  {
    typeof(g[v].begin()) = g[v].begin();
    while ((it != g[v].end()))
    {
      var to = (*it);
      cnt[v] += Count(to);
      it += 1;
    }
  }
  return cnt[v];
}

func Dfs(v: dynamic)
{
  if (was[v])
  {
    return;
  }
  was[v] = true;
  {
    var i = 0;
    while ((i <= n))
    {
      if ((st[v].next[(26 + i)] != -1))
      {
        dp[v][i] = 1;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 26))
    {
      if ((st[v].next[i] != -1))
      {
        var u = st[v].next[i];
        Dfs(u);
        {
          var j = 0;
          while ((j <= n))
          {
            dp[v][j] += dp[u][j];
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  if ((v && dp[v][0]))
  {
    var ok = true;
    {
      var i = 1;
      while ((ok && (i <= n)))
      {
        if ((!(((L[i] <= dp[v][i]) && (dp[v][i] <= R[i])))))
        {
          ok = false;
        }
        i += 1;
      }
    }
    if (ok)
    {
      ans += Count(v);
    }
  }
}

func main()
{
  scanf("%s", second);
  {
    var i = 0;
    while (second[i])
    {
      addAutomat((second[i] - cpp_char("a")));
      i += 1;
    }
  }
  addAutomat(26);
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("\n%s %d %d", (&second), (&L[i]), (&R[i]));
      {
        var j = 0;
        while (second[j])
        {
          addAutomat((second[j] - cpp_char("a")));
          j += 1;
        }
      }
      addAutomat((i + 26));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < sz))
    {
      {
        var ch = 0;
        while ((ch < 26))
        {
          if ((st[i].next[ch] != -1))
          {
            g[st[i].next[ch]].push_back(i);
          }
          ch += 1;
        }
      }
      i += 1;
    }
  }
  memset(cnt, -1, cpp_sizeof((cnt)));
  Dfs(0);
  write(ans, "\n");
  return 0;
}
