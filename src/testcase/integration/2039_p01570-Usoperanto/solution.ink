// Translated from solution.cpp.

var pll: dynamic = cpp_expression("#include<bit");

var pii: dynamic = cpp_expression("#include<bits/");

var X: dynamic = cpp_expression("#incl");

var Y: dynamic = cpp_expression("#inclu");

var MAXN: dynamic = cpp_expression("#includ");

var lson: dynamic = cpp_expression("#include<bits/std");

var rson: dynamic = cpp_expression("#include<bits/stdc++.h>");

var eps: dynamic = 1e-10;

var n: dynamic = cpp_uninitialized();

var root: dynamic = cpp_uninitialized();

var edge: dynamic = cpp_array(MAXN);

var value: dynamic = cpp_array(MAXN);

var cost: dynamic = cpp_array(MAXN);

func cmp(i: dynamic, j: dynamic) -> dynamic
{
  return (value[i] < value[j]);
}

func init() -> dynamic
{
  root.clear();
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      edge[i].clear();
      cost[i] = -1;
      i += 1;
    }
  }
}

func solve(rt: dynamic) -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  q.push(rt);
  s.push(rt);
  while ((!q.empty()))
  {
    var tmp: dynamic = q.front();
    q.pop();
    var len: dynamic = edge[tmp].size();
    {
      var i: dynamic = 0;
      while ((i < len))
      {
        q.push(edge[tmp][i]);
        s.push(edge[tmp][i]);
        i += 1;
      }
    }
  }
  while ((!s.empty()))
  {
    var tmp: dynamic = s.top();
    s.pop();
    sort(edge[tmp].begin(), edge[tmp].end(), cmp);
    var len: dynamic = edge[tmp].size();
    cost[tmp] = 0;
    var res: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < len))
      {
        cost[tmp] += (res + cost[edge[tmp][i]]);
        res += value[edge[tmp][i]];
        i += 1;
      }
    }
    value[tmp] += res;
  }
}

func main() -> dynamic
{
  while ((~scanf("%d", (&n))))
  {
    init();
    var x: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        scanf("%lld%d", (&value[i]), (&x));
        if ((x == -1))
        {
          root.push_back(i);
        } else
        {
          edge[x].push_back(i);
        }
        i += 1;
      }
    }
    var len: dynamic = root.size();
    var ans: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < len))
      {
        solve(root[i]);
        ans += cost[root[i]];
        i += 1;
      }
    }
    printf("%lld\n", ans);
  }
  return 0;
}
