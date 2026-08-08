// Translated from solution.cpp.

var MAX_N = cpp_expression("#in");

var MAX_M = cpp_expression("#incl");

class edge
{
  var from_cpp: dynamic;
  var to: dynamic;
  var id: dynamic;
}

var N: dynamic;

var M: dynamic;

var a = cpp_array(MAX_M);

var b = cpp_array(MAX_M);

var flg = cpp_array(MAX_M);

var G = cpp_array(MAX_N);

var visited = cpp_array(MAX_N);

var depth = cpp_array(MAX_N);

var cnt = cpp_array(MAX_N);

var par = cpp_array(MAX_N);

var bridges: dynamic;

var edge: dynamic;

func dfs(pos: dynamic, prev: dynamic)
{
  visited[pos] = true;
  {
    var i = 0;
    while ((i < cpp_cast(G[pos].size())))
    {
      var to = G[pos][i].to;
      if ((to == prev))
      {
        i += 1;
        continue;
      }
      if ((!visited[to]))
      {
        depth[to] = (depth[pos] + 1);
        par[to] = pos;
        dfs(to, pos);
        cnt[pos] += cnt[to];
        if ((cnt[to] == 0))
        {
          bridges.push_back(G[pos][i]);
        }
      } else if ((depth[to] < depth[pos]))
      {
        cnt[pos] += 1;
        cnt[to] -= 1;
        edge.push_back(G[pos][i]);
      }
      i += 1;
    }
  }
}

func countB(id: dynamic)
{
  bridges.clear();
  edge.clear();
  memset(visited, false, cpp_sizeof((visited)));
  memset(depth, 0, cpp_sizeof((depth)));
  memset(cnt, 0, cpp_sizeof((cnt)));
  memset(par, -1, cpp_sizeof((par)));
  {
    var i = 0;
    while ((i < N))
    {
      G[i].clear();
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < M))
    {
      if ((i == id))
      {
        i += 1;
        continue;
      }
      G[a[i]].push_back([a[i], b[i], i]);
      G[b[i]].push_back([b[i], a[i], i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      if (visited[i])
      {
        i += 1;
        continue;
      }
      dfs(i, -1);
      i += 1;
    }
  }
  return bridges.size();
}

func isDag(id: dynamic)
{
  var Q: dynamic;
  var C = cpp_construct(N, 0);
  var cc = 0;
  {
    var i = 0;
    while ((i < N))
    {
      G[i].clear();
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < M))
    {
      if ((i == id))
      {
        i += 1;
        continue;
      }
      G[a[i]].push_back([a[i], b[i], i]);
      C[b[i]] += 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      if ((C[i] == 0))
      {
        Q.push(i);
      }
      i += 1;
    }
  }
  while ((!Q.empty()))
  {
    var pos = Q.front();
    Q.pop();
    cc += 1;
    {
      var i = 0;
      while ((i < cpp_cast(G[pos].size())))
      {
        var to = G[pos][i].to;
        C[to] -= 1;
        if ((C[to] == 0))
        {
          Q.push(to);
        }
        i += 1;
      }
    }
  }
  return ((cc == N));
}

func visit(v: dynamic)
{
  if (visited[v])
  {
    return;
  }
  visited[v] = true;
  {
    var i = 0;
    while ((i < cpp_cast(G[v].size())))
    {
      visit(G[v][i].to);
      i += 1;
    }
  }
}

func calcDec(id: dynamic)
{
  {
    var i = 0;
    while ((i < N))
    {
      G[i].clear();
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < M))
    {
      if ((i == id))
      {
        i += 1;
        continue;
      }
      G[a[i]].push_back([a[i], b[i], i]);
      G[b[i]].push_back([b[i], a[i], i]);
      i += 1;
    }
  }
  var res = 0;
  memset(visited, false, cpp_sizeof((visited)));
  {
    var i = 0;
    while ((i < N))
    {
      if ((!visited[i]))
      {
        res += 1;
        visit(i);
      }
      i += 1;
    }
  }
  return res;
}

var mm: dynamic;

func check(id: dynamic)
{
  if (mm.count(id))
  {
    return false;
  }
  mm[id] = true;
  if ((((countB(id) + calcDec(id)) >= 3) && isDag(id)))
  {
    return true;
  } else
  {
    return false;
  }
}

var loope: dynamic;

func rec(pos: dynamic, si: dynamic)
{
  if (visited[pos])
  {
    return ((pos == si));
  }
  visited[pos] = true;
  {
    var i = 0;
    while ((i < cpp_cast(G[pos].size())))
    {
      var e = G[pos][i];
      if (rec(e.to, si))
      {
        loope.push_back(e);
        return true;
      }
      i += 1;
    }
  }
  return false;
}

func solve2()
{
  {
    var i = 0;
    while ((i < N))
    {
      G[i].clear();
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < M))
    {
      G[a[i]].push_back([a[i], b[i], i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      memset(visited, false, cpp_sizeof((visited)));
      loope.clear();
      if (rec(i, i))
      {
        break;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < cpp_cast(loope.size())))
    {
      if (check(loope[i].id))
      {
        return true;
      }
      i += 1;
    }
  }
  return false;
}

func solve()
{
  var B = countB(-1);
  if ((B == M))
  {
    return false;
  }
  if ((!isDag(-1)))
  {
    return solve2();
  }
  var maxm = 0;
  B = countB(-1);
  {
    var i = 0;
    while ((i < cpp_cast(edge.size())))
    {
      var e = edge[i];
      var a = e.from_cpp;
      var cc = 0;
      while (1)
      {
        if ((a == e.to))
        {
          break;
        }
        if ((cnt[a] == 1))
        {
          cc += 1;
        }
        a = par[a];
      }
      maxm = max(maxm, cc);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < M))
    {
      if (check(i))
      {
        return true;
      }
      i += 1;
    }
  }
  return (((B + maxm) >= 2));
}

func main()
{
  read(N, M);
  {
    var i = 0;
    while ((i < M))
    {
      read(a[i], b[i]);
      a[i] -= 1;
      b[i] -= 1;
      i += 1;
    }
  }
  write((if (solve()) "YES" else "NO"), "\n");
  return 0;
}
