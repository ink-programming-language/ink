// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

var pb = cpp_expression("#include <bi");

var ans = cpp_array(101, 101);

var g = cpp_array(10001);

var used = cpp_array(10001);

var dx = [1, 0, -1, 0];

var dy = [0, 1, 0, -1];

func main()
{
  var n: dynamic;
  var ai: dynamic;
  var bi: dynamic;
  read(n);
  var m = (((2 * n) * n) - (2 * n));
  var cur = 0;
  var tx = 0;
  var ty = 0;
  var id = 0;
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
    var t1 = (ans[(i - 1)][j] - 1);
    var t2 = (ans[i][(j - 1)] - 1);
    var mp: dynamic;
    cpp_update(rep(k, g[t1].size()), "++")[g[t1][k]];
    cpp_update(rep(k, g[t2].size()), "++")[g[t2][k]];
    for (var it in mp)
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

func rep(argument_0: dynamic, argument_1: dynamic)
{
    read(ai, bi);
    ai -= 1;
    bi -= 1;
    g[ai].pb(bi);
    g[bi].pb(ai);
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if ((g[i].size() == 2))
    {
      cur = i;
      break;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    ((rep(j, n) << ans[i][j]) << " ");
    write("\n");
  }
