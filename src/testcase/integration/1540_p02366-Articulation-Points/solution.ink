// Translated from solution.cpp.

var MAX: dynamic = 100000;

var ROOT: dynamic = cpp_expression("#");

var G: dynamic = cpp_array(MAX);

var visited: dynamic = cpp_array(MAX);

var prenum: dynamic = cpp_array(MAX);

var lowest: dynamic = cpp_array(MAX);

var parents: dynamic = cpp_array(MAX);

var is_artpoint: dynamic = cpp_array(MAX);

var NCHILD_ROOT: dynamic = 0;

func visit(n: dynamic, parent: dynamic) -> dynamic
{
  var v: dynamic = 0;
  visited[n] = true;
  prenum[n] = cpp_update(v, "++");
  parents[n] = parent;
}

func judge_parent(n: dynamic) -> dynamic
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

func calc_lowest(n: dynamic, childmin: dynamic) -> dynamic
{
  var l: dynamic = min(childmin, prenum[n]);
  for (var i: dynamic in G[n])
  {
    if ((i != parents[n]))
    {
      l = min(l, prenum[i]);
    }
  }
  return l;
}

func dfs(n: dynamic, parent: dynamic) -> dynamic
{
  var childmin: dynamic = MAX;
  visit(n, parent);
  for (var i: dynamic in G[n])
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

func main() -> dynamic
{
  var nv: dynamic = cpp_uninitialized();
  var ne: dynamic = cpp_uninitialized();
  read(nv, ne);
  {
    var i: dynamic = 0;
    while ((i < ne))
    {
      var s: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      read(s, t);
      G[s].push_back(t);
      G[t].push_back(s);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
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
