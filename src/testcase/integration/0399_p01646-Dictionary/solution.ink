// Translated from solution.cpp.

func REP(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for(int i=s;i<n;i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include<b");
}

var G = cpp_array(510);

var arr: dynamic;

var edges: dynamic;

var found = cpp_array(510);

var used = cpp_array(510);

var cycle: dynamic;

func inValid(a: dynamic, b: dynamic)
{
  if ((a == b))
  {
    return false;
  }
  var diff = -1;
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

func add(a: dynamic, b: dynamic)
{
  if ((a == b))
  {
    return;
  }
  var diff = -1;
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

func visit(v: dynamic, order: dynamic, color: dynamic)
{
  color[v] = 1;
  rep(i, G[v].size());
  {
    var e = G[v][i];
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

func topologicalSort(order: dynamic)
{
  var color = cpp_construct(26, 0);
  {
    var u = 0;
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

func main()
{
  var n: dynamic;
  while (cpp_comma((cin >> n), n))
  {
    cpp_statement("rep(i,510)");
    {
      G[i].clear();
      found[i] = cpp_assign(used[i], "=", false);
    }
    var fin = false;
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
      var src = edges[i].first;
      var dst = edges[i].second;
      G[src].push_back(dst);
    }
    var order: dynamic;
    if ((!topologicalSort(order)))
    {
      puts("no");
      continue;
    }
    puts("yes");
  }
  return 0;
}
