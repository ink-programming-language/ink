// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

var pb: dynamic = cpp_expression("#include <bi");

var ans: dynamic = cpp_array(101, 101);

var g: dynamic = cpp_array(10001);

var used: dynamic = cpp_array(10001);

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, 1, 0, -1];

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var ai: dynamic = cpp_uninitialized();
  var bi: dynamic = cpp_uninitialized();
  read(n);
  var m: dynamic = (((2 * n) * n) - (2 * n));
  var cur: dynamic = 0;
  var tx: dynamic = 0;
  var ty: dynamic = 0;
  var id: dynamic = 0;
  while (1)
  {
    ans[ty][tx] = (cur + 1);
    used[cur] = true;
    ty = (ty + dy[id]);
    tx = (tx + dx[id]);
    rep(i, g[cur].size());
    {
      if (((g[g[cur][i]].size() == 2) && (!used[g[cur][i]])))
      {
        cur = g[cur][i];
        id += 1;
        break;
      } else if (((g[g[cur][i]].size() == 3) && (!used[g[cur][i]])))
      {
        cur = g[cur][i];
        break;
      }
    }
    if (((tx == 0) && (ty == 0)))
    {
      break;
    }
  }
  FOR(i, 1, (n - 1));
  FOR(j, 1, (n - 1));
  {
    var t1: dynamic = (ans[(i - 1)][j] - 1);
    var t2: dynamic = (ans[i][(j - 1)] - 1);
    var mp: dynamic = cpp_uninitialized();
    cpp_update(rep(k, g[t1].size()), "++")[g[t1][k]];
    cpp_update(rep(k, g[t2].size()), "++")[g[t2][k]];
    for (var it: dynamic in mp)
    {
      if (((it.second == 2) && (!used[it.first])))
      {
        ans[i][j] = (it.first + 1);
        used[it.first] = true;
        break;
      }
    }
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    read(ai, bi);
    ai -= 1;
    bi -= 1;
    g[ai].pb(bi);
    g[bi].pb(ai);
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    if ((g[i].size() == 2))
    {
      cur = i;
      break;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    ((rep(j, n) << ans[i][j]) << " ");
    write("\n");
  }
