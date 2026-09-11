// Translated from solution.cpp.

var maxn: dynamic = cpp_cast(4e5);

class state
{
  var next: dynamic = cpp_array(37);
  var len: dynamic = cpp_uninitialized();
  var suff: dynamic = cpp_uninitialized();
  func state() -> dynamic
  {
      memset(next, -1, cpp_sizeof((next)));
      suff = -1;
      len = 0;
    }
}

var st: dynamic = cpp_array((2 * maxn));

var sz: dynamic = 1;

var last: dynamic = 0;

var g: dynamic = cpp_array(maxn);

var second: dynamic = cpp_array(maxn);

var L: dynamic = cpp_array(maxn);

var R: dynamic = cpp_array(maxn);

var n: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(11, (maxn * 3));

var cnt: dynamic = cpp_array((maxn * 2));

var was: dynamic = cpp_array((maxn * 2));

var ans: dynamic = cpp_uninitialized();

func addAutomat(ch: dynamic) -> dynamic
{
  var p: dynamic = last;
  var nv: dynamic = cpp_update(sz, "++");
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
    var q: dynamic = st[p].next[ch];
    if ((st[q].len == (st[p].len + 1)))
    {
      st[nv].suff = q;
    } else
    {
      var clone: dynamic = cpp_update(sz, "++");
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

func Count(v: dynamic) -> dynamic
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
      var to: dynamic = (*it);
      cnt[v] += Count(to);
      it += 1;
    }
  }
  return cnt[v];
}

func Dfs(v: dynamic) -> dynamic
{
  if (was[v])
  {
    return;
  }
  was[v] = true;
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < 26))
    {
      if ((st[v].next[i] != -1))
      {
        var u: dynamic = st[v].next[i];
        Dfs(u);
        {
          var j: dynamic = 0;
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
    var ok: dynamic = true;
    {
      var i: dynamic = 1;
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

func main() -> dynamic
{
  scanf("%s", second);
  {
    var i: dynamic = 0;
    while (second[i])
    {
      addAutomat((second[i] - cpp_char("a")));
      i += 1;
    }
  }
  addAutomat(26);
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("\n%s %d %d", (&second), (&L[i]), (&R[i]));
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < sz))
    {
      {
        var ch: dynamic = 0;
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
