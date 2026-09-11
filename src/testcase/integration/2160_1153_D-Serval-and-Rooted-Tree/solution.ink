// Translated from solution.cpp.

var mod: dynamic = 100000007700000049;

var MAXN: dynamic = (3e5 + 5);

var op: dynamic = cpp_array(MAXN);

var son: dynamic = cpp_array(MAXN);

var val: dynamic = cpp_array(MAXN);

func dfs(pos: dynamic) -> dynamic
{
  var ans: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
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

func main() -> dynamic
{
  ios.sync_with_stdio();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var t1: dynamic = cpp_uninitialized();
  var t2: dynamic = cpp_uninitialized();
  var t3: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
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
