// Translated from solution.cpp.

var N = 200005;

var s: dynamic;

var t: dynamic;

var lcp: dynamic;

var next = cpp_array(N);

var occur = cpp_array(N);

func kmp(s: dynamic)
{
  {
    var i = 1;
    var j = cpp_assign(next[0], "=", -1);
    while ((i <= s.size()))
    {
      {
        while (((j >= 0) && (s[j] != s[(i - 1)])))
        {
          j = next[j];
        }
      }
      next[cpp_update(i, "++")] = cpp_update(j, "++");
    }
  }
}

var size = cpp_array(N);

var idx = 1;

var lst = 1;

var nxt = cpp_array(26, N);

var fail = cpp_array(N);

var max = cpp_array(N);

func append(ch: dynamic)
{
  var p = lst;
  var np = cpp_assign(lst, "=", cpp_update(idx, "++"));
  max[np] = (max[p] + 1);
  size[np] = 1;
  {
    while ((p && (!nxt[p][ch])))
    {
      nxt[p][ch] = np;
      p = fail[p];
    }
  }
  if ((!p))
  {
    fail[np] = 1;
  } else
  {
    var q = nxt[p][ch];
    if (((max[p] + 1) == max[q]))
    {
      fail[np] = q;
    } else
    {
      var nq = cpp_update(idx, "++");
      max[nq] = (max[p] + 1);
      memcpy(nxt[nq], nxt[q], (26 << 2));
      fail[nq] = fail[q];
      fail[q] = cpp_assign(fail[np], "=", nq);
      {
        while ((nxt[p][ch] == q))
        {
          nxt[p][ch] = nq;
          p = fail[p];
        }
      }
    }
  }
}

var head = cpp_array(N);

var next = cpp_array(N);

func link(x: dynamic, y: dynamic)
{
  next[y] = head[x];
  head[x] = y;
}

func dfs(x: dynamic)
{
  {
    var i = head[x];
    while (i)
    {
      size[x] += dfs(i);
      i = next[i];
    }
  }
  return size[x];
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(s, t);
  var n = s.size();
  var m = t.size();
  {
    lcp = 0;
    while (((lcp < s.size()) && (lcp < t.size())))
    {
      if ((s[lcp] != t[lcp]))
      {
        break;
      }
      lcp += 1;
    }
  }
  for (var ch in s)
  {
    append((ch - cpp_char("a")));
  }
  {
    var i = 2;
    while ((i <= idx))
    {
      fail_tree.link(fail[i], i);
      i += 1;
    }
  }
  fail_tree.dfs(1);
  kmp(t);
  var now = 1;
  var ans = (cpp_cast(n) * m);
  {
    var i = 0;
    while ((i < t.size()))
    {
      occur[i] = size[cpp_assign(now, "=", nxt[now][(t[i] - cpp_char("a"))])];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < lcp))
    {
      occur[i] -= 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= t.size()))
    {
      if (next[i])
      {
        ans -= occur[((i - next[i]) - 1)];
      }
      i += 1;
    }
  }
  write(ans, cpp_char("\n"));
  return 0;
}
