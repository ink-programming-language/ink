// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var adj: dynamic = cpp_uninitialized();

var top: dynamic = cpp_uninitialized();

var branched: dynamic = cpp_uninitialized();

var visited: dynamic = cpp_uninitialized();

var fertile: dynamic = cpp_uninitialized();

var root: dynamic = -1;

func fail() -> dynamic
{
  write("No", "\n");
  exit(0);
}

func dfsFindRoot(u: dynamic) -> dynamic
{
  if ((root != -1))
  {
    return;
  }
  visited[u] = true;
  var leaf: dynamic = true;
  for (var child: dynamic in adj[u])
  {
    if (visited[child])
    {
      continue;
    }
    leaf = false;
  }
  if ((!leaf))
  {
    branched[u] = 0;
    for (var child: dynamic in adj[u])
    {
      if (visited[child])
      {
        continue;
      }
      dfsFindRoot(child);
      branched[u] += branched[child];
    }
    if ((root != -1))
    {
      return;
    }
    if ((branched[u] >= 3))
    {
      root = u;
    }
  }
}

func dfs(u: dynamic) -> dynamic
{
  visited[u] = true;
  var leaf: dynamic = true;
  for (var child: dynamic in adj[u])
  {
    if (visited[child])
    {
      continue;
    }
    leaf = false;
  }
  if ((!leaf))
  {
    branched[u] = 0;
    var numTops: dynamic = 0;
    for (var child: dynamic in adj[u])
    {
      if (visited[child])
      {
        continue;
      }
      dfs(child);
      if (fertile[child])
      {
        top[u] = true;
      }
      branched[u] += branched[child];
      if (top[child])
      {
        numTops += 1;
        top[u] = true;
      }
    }
    if (((u != root) && (numTops > 1)))
    {
      fail();
    } else if ((numTops > 2))
    {
      fail();
    }
    if ((branched[u] >= 3))
    {
      top[u] = true;
    } else if ((branched[u] == 2))
    {
      fertile[u] = true;
    }
  }
}

func main() -> dynamic
{
  var m: dynamic = cpp_uninitialized();
  read(n);
  adj = vector(n, vector(0));
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      u -= 1;
      v -= 1;
      adj[u].push_back(v);
      adj[v].push_back(u);
      i += 1;
    }
  }
  if ((n <= 4))
  {
    write("YES", "\n");
    return 0;
  }
  top = vector(n, false);
  branched = vector(n, 1);
  visited = vector(n, false);
  dfsFindRoot(0);
  if ((root == -1))
  {
    write("Yes", "\n");
    return 0;
  }
  top = vector(n, false);
  branched = vector(n, 1);
  visited = vector(n, false);
  fertile = vector(n, false);
  dfs(root);
  write("Yes", "\n");
}
