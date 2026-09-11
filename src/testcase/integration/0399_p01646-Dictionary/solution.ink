// Translated from solution.cpp.

func REP(i: dynamic, s: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=s;i<n;i++)");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include<b");
}

var G: dynamic = cpp_array(510);

var arr: dynamic = cpp_uninitialized();

var edges: dynamic = cpp_uninitialized();

var found: dynamic = cpp_array(510);

var used: dynamic = cpp_array(510);

var cycle: dynamic = cpp_uninitialized();

func inValid(a: dynamic, b: dynamic) -> dynamic
{
  if ((a == b))
  {
    return false;
  }
  var diff: dynamic = -1;
  rep(i, min(a.size(), b.size()));
  if ((a[i] != b[i]))
  {
    diff = i;
    break;
  }
  if (((diff == -1) && (a.size() > b.size())))
  {
    return true;
  }
  return false;
}

func add(a: dynamic, b: dynamic) -> dynamic
{
  if ((a == b))
  {
    return;
  }
  var diff: dynamic = -1;
  rep(i, min(a.size(), b.size()));
  if ((a[i] != b[i]))
  {
    diff = i;
    break;
  }
  if ((diff == -1))
  {
    return;
  }
  edges.push_back(ii((a[diff] - cpp_char("a")), (b[diff] - cpp_char("a"))));
}

func visit(v: dynamic, order: dynamic, color: dynamic) -> dynamic
{
  color[v] = 1;
  rep(i, G[v].size());
  {
    var e: dynamic = G[v][i];
    if ((color[e] == 2))
    {
      continue;
    }
    if ((color[e] == 1))
    {
      return false;
    }
    if ((!visit(e, order, color)))
    {
      return false;
    }
  }
  order.push_back(v);
  color[v] = 2;
  return true;
}

func topologicalSort(order: dynamic) -> dynamic
{
  var color: dynamic = cpp_construct(26, 0);
  {
    var u: dynamic = 0;
    while ((u < 26))
    {
      if (((!color[u]) && (!visit(u, order, color))))
      {
        return false;
      }
      u += 1;
    }
  }
  reverse(order.begin(), order.end());
  return true;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while (cpp_comma((cin >> n), n))
  {
    cpp_statement("rep(i,510)");
    {
      G[i].clear();
      found[i] = cpp_assign(used[i], "=", false);
    }
    var fin: dynamic = false;
    cycle = false;
    arr.clear();
    arr.resize(n);
    edges.clear();
    rep(i, n);
    read(arr[i]);
    rep(i, (n - 1));
    {
      if (inValid(arr[i], arr[(i + 1)]))
      {
        puts("no");
        fin = true;
        break;
      }
      add(arr[i], arr[(i + 1)]);
    }
    if (fin)
    {
      continue;
    }
    rep(i, edges.size());
    {
      var src: dynamic = edges[i].first;
      var dst: dynamic = edges[i].second;
      G[src].push_back(dst);
    }
    var order: dynamic = cpp_uninitialized();
    if ((!topologicalSort(order)))
    {
      puts("no");
      continue;
    }
    puts("yes");
  }
  return 0;
}
