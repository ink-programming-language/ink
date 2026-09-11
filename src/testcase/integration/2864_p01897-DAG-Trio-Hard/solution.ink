// Translated from solution.cpp.

var MAX_N: dynamic = cpp_expression("#in");

var MAX_M: dynamic = cpp_expression("#incl");

class edge
{
  var from_cpp: dynamic = cpp_uninitialized();
  var to: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(MAX_M);

var b: dynamic = cpp_array(MAX_M);

var flg: dynamic = cpp_array(MAX_M);

var G: dynamic = cpp_array(MAX_N);

var visited: dynamic = cpp_array(MAX_N);

var depth: dynamic = cpp_array(MAX_N);

var cnt: dynamic = cpp_array(MAX_N);

var par: dynamic = cpp_array(MAX_N);

var bridges: dynamic = cpp_uninitialized();

var edge: dynamic = cpp_uninitialized();

func dfs(pos: dynamic, prev: dynamic) -> dynamic
{
  visited[pos] = true;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(G[pos].size())))
    {
      var to: dynamic = G[pos][i].to;
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

func countB(id: dynamic) -> dynamic
{
  bridges.clear();
  edge.clear();
  memset(visited, false, cpp_sizeof((visited)));
  memset(depth, 0, cpp_sizeof((depth)));
  memset(cnt, 0, cpp_sizeof((cnt)));
  memset(par, -1, cpp_sizeof((par)));
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      G[i].clear();
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
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

func isDag(id: dynamic) -> dynamic
{
  var Q: dynamic = cpp_uninitialized();
  var C: dynamic = cpp_construct(N, 0);
  var cc: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      G[i].clear();
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
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
    var pos: dynamic = Q.front();
    Q.pop();
    cc += 1;
    {
      var i: dynamic = 0;
      while ((i < cpp_cast(G[pos].size())))
      {
        var to: dynamic = G[pos][i].to;
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

func visit(v: dynamic) -> dynamic
{
  if (visited[v])
  {
    return;
  }
  visited[v] = true;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(G[v].size())))
    {
      visit(G[v][i].to);
      i += 1;
    }
  }
}

func calcDec(id: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      G[i].clear();
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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
  var res: dynamic = 0;
  memset(visited, false, cpp_sizeof((visited)));
  {
    var i: dynamic = 0;
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

var mm: dynamic = cpp_uninitialized();

func check(id: dynamic) -> dynamic
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

var loope: dynamic = cpp_uninitialized();

func rec(pos: dynamic, si: dynamic) -> dynamic
{
  if (visited[pos])
  {
    return ((pos == si));
  }
  visited[pos] = true;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(G[pos].size())))
    {
      var e: dynamic = G[pos][i];
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

func solve2() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      G[i].clear();
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      G[a[i]].push_back([a[i], b[i], i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
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

func solve() -> dynamic
{
  var B: dynamic = countB(-1);
  if ((B == M))
  {
    return false;
  }
  if ((!isDag(-1)))
  {
    return solve2();
  }
  var maxm: dynamic = 0;
  B = countB(-1);
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(edge.size())))
    {
      var e: dynamic = edge[i];
      var a: dynamic = e.from_cpp;
      var cc: dynamic = 0;
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
    var i: dynamic = 0;
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

func main() -> dynamic
{
  read(N, M);
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      read(a[i], b[i]);
      a[i] -= 1;
      b[i] -= 1;
      i += 1;
    }
  }
  write(( (solve()) ? "YES" : "NO"), "\n");
  return 0;
}
