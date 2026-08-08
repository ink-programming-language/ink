// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

var N = (3e5 + 100);

var debug = false;

var vc = cpp_array(N);

var gg1 = cpp_array(N);

var gg2 = cpp_array(N);

var gg3 = cpp_array(N);

var iit: dynamic;

var in_cpp = cpp_array(N);

var vis = cpp_array(N);

var ord = cpp_array(N);

var dp = cpp_array(2, N);

var sum = cpp_array(N);

func cmp(a: dynamic, b: dynamic)
{
  return (in_cpp[a.first] > in_cpp[b.first]);
}

func cmp1(a: dynamic, b: dynamic)
{
  return (in_cpp[a] > in_cpp[b]);
}

func dfs(o: dynamic, u: dynamic, op: dynamic)
{
  vis[o] = op;
  dp[o][1] = 0;
  while ((!vc[o].empty()))
  {
    var it = vc[o].back();
    if ((in_cpp[it.first] != op))
    {
      break;
    }
    gg2[o].insert([it.second, it.first]);
    sum[o] += it.second;
    vc[o].pop_back();
  }
  while ((gg2[o].size() > (in_cpp[o] - op)))
  {
    if ((gg2[o].rbegin()->first < 0))
    {
      break;
    }
    sum[o] -= gg2[o].rbegin()->first;
    gg2[o].erase(cpp_update(gg2[o].end(), "--"));
  }
  for (var it in vc[o])
  {
    if ((vis[it.first] == op))
    {
      continue;
    }
    dfs(it.first, o, op);
    dp[o][1] += dp[it.first][0];
    var now = (min((dp[it.first][1] - dp[it.first][0]), 0) + it.second);
    gg1[o].insert([now, it.first]);
    gg2[o].insert([now, it.first]);
    sum[o] += now;
  }
  while ((gg2[o].size() > (in_cpp[o] - op)))
  {
    if ((gg2[o].rbegin()->first < 0))
    {
      break;
    }
    sum[o] -= gg2[o].rbegin()->first;
    gg3[o].insert((*(gg2[o].rbegin())));
    gg2[o].erase(cpp_update(gg2[o].end(), "--"));
  }
  dp[o][0] = (dp[o][1] + sum[o]);
  dp[o][1] = (dp[o][0] - gg2[o].rbegin()->first);
  for (var it in gg3[o])
  {
    gg2[o].insert(it);
    sum[o] += it.first;
  }
  for (var it in gg1[o])
  {
    gg2[o].erase(it);
    sum[o] -= it.first;
  }
  gg1[o].clear();
  gg3[o].clear();
}

var ret: dynamic;

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      ord[i] = i;
      i += 1;
    }
  }
  {
    var i = 1;
    var u: dynamic;
    var v: dynamic;
    var x: dynamic;
    while ((i < n))
    {
      scanf("%d %d %d", (&u), (&v), (&x));
      vc[u].emplace_back(v, x);
      vc[v].emplace_back(u, x);
      in_cpp[u] += 1;
      in_cpp[v] += 1;
      ans += x;
      i += 1;
    }
  }
  sort((ord + 1), ((ord + n) + 1), cmp1);
  {
    var i = 1;
    while ((i <= n))
    {
      sort(vc[i].begin(), vc[i].end(), cmp);
      i += 1;
    }
  }
  ret.push_back(ans);
  var cnt = 0;
  {
    var i = 1;
    while ((i < n))
    {
      ans = 0;
      {
        var j = 1;
        while ((j <= n))
        {
          if ((in_cpp[ord[j]] <= i))
          {
            break;
          }
          if ((vis[ord[j]] != i))
          {
            dfs(ord[j], 0, i);
            ans += dp[ord[j]][0];
          }
          j += 1;
        }
      }
      ret.push_back(ans);
      i += 1;
    }
  }
  for (var it in ret)
  {
    printf("%I64d ", it);
  }
  return 0;
}
