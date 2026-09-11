// Translated from solution.cpp.

var N: dynamic = 200005;

var s: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var lcp: dynamic = cpp_uninitialized();

var next: dynamic = cpp_array(N);

var occur: dynamic = cpp_array(N);

func kmp(s: dynamic) -> dynamic
{
  {
    var i: dynamic = 1;
    var j: dynamic = cpp_assign(next[0], "=", -1);
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

var size: dynamic = cpp_array(N);

var idx: dynamic = 1;

var lst: dynamic = 1;

var nxt: dynamic = cpp_array(26, N);

var fail: dynamic = cpp_array(N);

var max: dynamic = cpp_array(N);

func append(ch: dynamic) -> dynamic
{
  var p: dynamic = lst;
  var np: dynamic = cpp_assign(lst, "=", cpp_update(idx, "++"));
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
    var q: dynamic = nxt[p][ch];
    if (((max[p] + 1) == max[q]))
    {
      fail[np] = q;
    } else
    {
      var nq: dynamic = cpp_update(idx, "++");
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

var head: dynamic = cpp_array(N);

var next: dynamic = cpp_array(N);

func link(x: dynamic, y: dynamic) -> dynamic
{
  next[y] = head[x];
  head[x] = y;
}

func dfs(x: dynamic) -> dynamic
{
  {
    var i: dynamic = head[x];
    while (i)
    {
      size[x] += dfs(i);
      i = next[i];
    }
  }
  return size[x];
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(s, t);
  var n: dynamic = s.size();
  var m: dynamic = t.size();
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
  for (var ch: dynamic in s)
  {
    append((ch - cpp_char("a")));
  }
  {
    var i: dynamic = 2;
    while ((i <= idx))
    {
      fail_tree.link(fail[i], i);
      i += 1;
    }
  }
  fail_tree.dfs(1);
  kmp(t);
  var now: dynamic = 1;
  var ans: dynamic = (cpp_cast(n) * m);
  {
    var i: dynamic = 0;
    while ((i < t.size()))
    {
      occur[i] = size[cpp_assign(now, "=", nxt[now][(t[i] - cpp_char("a"))])];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < lcp))
    {
      occur[i] -= 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
