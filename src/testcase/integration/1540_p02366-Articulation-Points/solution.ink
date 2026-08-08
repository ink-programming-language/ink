// Translated from solution.cpp.

var MAX = 100000;

var ROOT = cpp_expression("#");

var G = cpp_array(MAX);

var visited = cpp_array(MAX);

var prenum = cpp_array(MAX);

var lowest = cpp_array(MAX);

var parents = cpp_array(MAX);

var is_artpoint = cpp_array(MAX);

var NCHILD_ROOT = 0;

func visit(n: dynamic, parent: dynamic)
{
  var v = 0;
  visited[n] = true;
  prenum[n] = cpp_update(v, "++");
  parents[n] = parent;
}

func judge_parent(n: dynamic)
{
  if (((n != ROOT) && (parents[n] == ROOT)))
  {
    NCHILD_ROOT += 1;
  }
  if ((prenum[parents[n]] <= lowest[n]))
  {
    is_artpoint[parents[n]] = true;
  }
}

func calc_lowest(n: dynamic, childmin: dynamic)
{
  var l = min(childmin, prenum[n]);
  for (var i in G[n])
  {
    if ((i != parents[n]))
    {
      l = min(l, prenum[i]);
    }
  }
  return l;
}

func dfs(n: dynamic, parent: dynamic)
{
  var childmin = MAX;
  visit(n, parent);
  for (var i in G[n])
  {
    if ((!visited[i]))
    {
      childmin = min(childmin, dfs(i, n));
    }
  }
  lowest[n] = calc_lowest(n, childmin);
  judge_parent(n);
  return lowest[n];
}

func main()
{
  var nv: dynamic;
  var ne: dynamic;
  read(nv, ne);
  {
    var i = 0;
    while ((i < ne))
    {
      var s: dynamic;
      var t: dynamic;
      read(s, t);
      G[s].push_back(t);
      G[t].push_back(s);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < nv))
    {
      visited[i] = false;
      is_artpoint[i] = false;
      i += 1;
    }
  }
  dfs(ROOT, ROOT);
  is_artpoint[ROOT] = (NCHILD_ROOT >= 2);
  {
    var i = 0;
    while ((i < nv))
    {
      if (is_artpoint[i])
      {
        write(i, "\n");
      }
      i += 1;
    }
  }
}
