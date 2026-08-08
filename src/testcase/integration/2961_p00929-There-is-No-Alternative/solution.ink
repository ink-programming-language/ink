// Translated from solution.cpp.

var EPS = cpp_expression("#inclu");

var INF = cpp_expression("#incl");

var PI = cpp_expression("#include <");

class UnionFind
{
  var par: dynamic;
  var rank: dynamic;
  var Size: dynamic;
  func UnionFind(n: dynamic = 1)
  {
      init(n);
    }
  func init(n: dynamic = 1)
  {
      par.resize((n + 1));
      rank.resize((n + 1));
      Size.resize((n + 1));
      {
        var i = 0;
        while ((i <= n))
        {
          par[i] = i;
          rank[i] = 0;
          Size[i] = 1;
          i += 1;
        }
      }
    }
  func root(x: dynamic)
  {
      if ((par[x] == x))
      {
        return x;
      } else
      {
        var r = root(par[x]);
        return cpp_assign(par[x], "=", r);
      }
    }
  func issame(x: dynamic, y: dynamic)
  {
      return (root(x) == root(y));
    }
  func merge(x: dynamic, y: dynamic)
  {
      x = root(x);
      y = root(y);
      if ((x == y))
      {
        return false;
      }
      if ((rank[x] < rank[y]))
      {
        swap(x, y);
      }
      if ((rank[x] == rank[y]))
      {
        rank[x] += 1;
      }
      par[y] = x;
      Size[x] += Size[y];
      return true;
    }
  func size(x: dynamic)
  {
      return Size[root(x)];
    }
}

var N: dynamic;

var M: dynamic;

var mp: dynamic;

var S = cpp_array(50000);

var D = cpp_array(50000);

var C = cpp_array(50000);

var paths = cpp_array(500);

var que: dynamic;

var rest: dynamic;

var ans = cpp_array(50000);

func dfs(now: dynamic, from_cpp: dynamic, destination: dynamic, cost: dynamic)
{
  if ((now == destination))
  {
    return true;
  }
  for (var tmp in paths[now])
  {
    var to = tmp.first;
    var nowcost = tmp.second.first;
    var e = tmp.second.second;
    if ((to == from_cpp))
    {
      continue;
    }
    if (dfs(to, now, destination, cost))
    {
      if ((nowcost == cost))
      {
        ans[e] = false;
      }
      return true;
    }
  }
  return false;
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  read(N, M);
  {
    var i = 0;
    while ((i < M))
    {
      read(S[i], D[i], C[i]);
      S[i] -= 1;
      D[i] -= 1;
      mp[[S[i], D[i]]] = i;
      que.push_back([C[i], [S[i], D[i]]]);
      i += 1;
    }
  }
  sort(que.begin(), que.end());
  for (var tmp in que)
  {
    var s = tmp.second.first;
    var t = tmp.second.second;
    if (uni.merge(tmp.second.first, tmp.second.second))
    {
      paths[s].push_back([t, [tmp.first, mp[[s, t]]]]);
      paths[t].push_back([s, [tmp.first, mp[[s, t]]]]);
      ans[mp[[s, t]]] = true;
    } else
    {
      rest.push_back(tmp);
    }
  }
  for (var tmp in rest)
  {
    dfs(tmp.second.first, -1, tmp.second.second, tmp.first);
  }
  var ansnum = 0;
  var anscost = 0;
  {
    var i = 0;
    while ((i < M))
    {
      if (ans[i])
      {
        ansnum += 1;
        anscost += C[i];
      }
      i += 1;
    }
  }
  write(ansnum, " ", anscost, "\n");
  return 0;
}
