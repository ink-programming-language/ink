// Translated from solution.cpp.

var INF: dynamic = 0x3f3f3f3f;

var N: dynamic = (3e5 + 100);

var debug: dynamic = false;

var vc: dynamic = cpp_array(N);

var gg1: dynamic = cpp_array(N);

var gg2: dynamic = cpp_array(N);

var gg3: dynamic = cpp_array(N);

var iit: dynamic = cpp_uninitialized();

var in_cpp: dynamic = cpp_array(N);

var vis: dynamic = cpp_array(N);

var ord: dynamic = cpp_array(N);

var dp: dynamic = cpp_array(2, N);

var sum: dynamic = cpp_array(N);

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (in_cpp[a.first] > in_cpp[b.first]);
}

func cmp1(a: dynamic, b: dynamic) -> dynamic
{
  return (in_cpp[a] > in_cpp[b]);
}

func dfs(o: dynamic, u: dynamic, op: dynamic) -> dynamic
{
  vis[o] = op;
  dp[o][1] = 0;
  while ((!vc[o].empty()))
  {
    var it: dynamic = vc[o].back();
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
  for (var it: dynamic in vc[o])
  {
    if ((vis[it.first] == op))
    {
      continue;
    }
    dfs(it.first, o, op);
    dp[o][1] += dp[it.first][0];
    var now: dynamic = (min((dp[it.first][1] - dp[it.first][0]), 0) + it.second);
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
  for (var it: dynamic in gg3[o])
  {
    gg2[o].insert(it);
    sum[o] += it.first;
  }
  for (var it: dynamic in gg1[o])
  {
    gg2[o].erase(it);
    sum[o] -= it.first;
  }
  gg1[o].clear();
  gg3[o].clear();
}

var ret: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ord[i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    var u: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      sort(vc[i].begin(), vc[i].end(), cmp);
      i += 1;
    }
  }
  ret.push_back(ans);
  var cnt: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      ans = 0;
      {
        var j: dynamic = 1;
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
  for (var it: dynamic in ret)
  {
    printf("%I64d ", it);
  }
  return 0;
}
