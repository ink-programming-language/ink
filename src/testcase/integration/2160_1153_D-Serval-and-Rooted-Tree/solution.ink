// Translated from solution.cpp.

var mod = 100000007700000049;

var MAXN = (3e5 + 5);

var op = cpp_array(MAXN);

var son = cpp_array(MAXN);

var val = cpp_array(MAXN);

func dfs(pos: dynamic)
{
  var ans: dynamic;
  var i: dynamic;
  if ((val[pos] != -1))
  {
    return val[pos];
  }
  if ((op[pos] == 0))
  {
    ans = 0;
    {
      i = 0;
      while ((i < son[pos].size()))
      {
        ans += dfs(son[pos][i]);
        i += 1;
      }
    }
    return cpp_assign(val[pos], "=", ans);
  } else
  {
    ans = dfs(son[pos][0]);
    {
      i = 1;
      while ((i < son[pos].size()))
      {
        ans = min(ans, dfs(son[pos][i]));
        i += 1;
      }
    }
    return cpp_assign(val[pos], "=", ans);
  }
}

func main()
{
  ios.sync_with_stdio();
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var t1: dynamic;
  var t2: dynamic;
  var t3: dynamic;
  var n: dynamic;
  var m: dynamic;
  read(n);
  {
    i = 1;
    while ((i <= n))
    {
      read(op[i]);
      i += 1;
    }
  }
  {
    i = 2;
    while ((i <= n))
    {
      read(t1);
      son[t1].push_back(i);
      i += 1;
    }
  }
  t1 = 1;
  memset(val, -1, cpp_sizeof((val)));
  {
    i = 2;
    while ((i <= n))
    {
      if ((!son[i].size()))
      {
        val[i] = 1;
        t1 += 1;
      }
      i += 1;
    }
  }
  write((t1 - dfs(1)));
}
