// Translated from solution.cpp.

var NMAX = (2e5 + 5);

var n: dynamic;

var answer: dynamic;

var root: dynamic;

var curr: dynamic;

var parent = cpp_array(NMAX);

var vis = cpp_array(NMAX);

var roots: dynamic;

func dfs(node: dynamic)
{
  if ((vis[node] != 0))
  {
    if ((vis[node] == curr))
    {
      parent[node] = node;
      roots.push_back(node);
    } else
    {
      return;
    }
  }
  vis[node] = curr;
  if ((node != parent[node]))
  {
    dfs(parent[node]);
  }
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(parent[i]);
      if ((parent[i] == i))
      {
        root = i;
        roots.push_back(i);
      }
      i += 1;
    }
  }
  if (roots.empty())
  {
    answer = 1;
  }
  {
    var i = 1;
    while ((i <= n))
    {
      curr += 1;
      dfs(i);
      i += 1;
    }
  }
  for (var x in roots)
  {
    parent[x] = roots[0];
    answer += 1;
  }
  answer -= 1;
  write(answer, cpp_char("\n"));
  {
    var i = 1;
    while ((i <= n))
    {
      write(parent[i], cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
