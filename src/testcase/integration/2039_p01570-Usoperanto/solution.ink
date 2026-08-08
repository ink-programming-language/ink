// Translated from solution.cpp.

var pll = cpp_expression("#include<bit");

var pii = cpp_expression("#include<bits/");

var X = cpp_expression("#incl");

var Y = cpp_expression("#inclu");

var MAXN = cpp_expression("#includ");

var lson = cpp_expression("#include<bits/std");

var rson = cpp_expression("#include<bits/stdc++.h>");

var eps = 1e-10;

var n: dynamic;

var root: dynamic;

var edge = cpp_array(MAXN);

var value = cpp_array(MAXN);

var cost = cpp_array(MAXN);

func cmp(i: dynamic, j: dynamic)
{
  return (value[i] < value[j]);
}

func init()
{
  root.clear();
  {
    var i = 0;
    while ((i <= n))
    {
      edge[i].clear();
      cost[i] = -1;
      i += 1;
    }
  }
}

func solve(rt: dynamic)
{
  var q: dynamic;
  var s: dynamic;
  q.push(rt);
  s.push(rt);
  while ((!q.empty()))
  {
    var tmp = q.front();
    q.pop();
    var len = edge[tmp].size();
    {
      var i = 0;
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
    var tmp = s.top();
    s.pop();
    sort(edge[tmp].begin(), edge[tmp].end(), cmp);
    var len = edge[tmp].size();
    cost[tmp] = 0;
    var res = 0;
    {
      var i = 0;
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

func main()
{
  while ((~scanf("%d", (&n))))
  {
    init();
    var x: dynamic;
    {
      var i = 0;
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
    var len = root.size();
    var ans = 0;
    {
      var i = 0;
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
