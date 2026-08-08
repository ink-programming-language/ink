// Translated from solution.cpp.

var g: dynamic;

var c: dynamic;

var numchild: dynamic;

var ans: dynamic;

var segtree: dynamic;

var root: dynamic;

var n: dynamic;

func build(id: dynamic = 1, l: dynamic = 1, r: dynamic = n)
{
  if ((l == r))
  {
    segtree[id] += 1;
    return;
  }
  var mid = (((l + r)) >> 1);
  build((2 * id), l, mid);
  build(((2 * id) + 1), (mid + 1), r);
  segtree[id] = (segtree[(2 * id)] + segtree[((2 * id) + 1)]);
}

func get(val: dynamic, id: dynamic = 1, l: dynamic = 1, r: dynamic = n)
{
  if ((l == r))
  {
    segtree[id] = 0;
    return l;
  }
  var mid = (((l + r)) >> 1);
  var pos: dynamic;
  if ((val <= segtree[(2 * id)]))
  {
    pos = get(val, (2 * id), l, mid);
  } else
  {
    pos = get((val - segtree[(2 * id)]), ((2 * id) + 1), (mid + 1), r);
  }
  segtree[id] = (segtree[(2 * id)] + segtree[((2 * id) + 1)]);
  return pos;
}

func dfs(u: dynamic)
{
  numchild[u] = 1;
  ans[u] = get((c[u] + 1));
  for (var v in g[u])
  {
    dfs(v);
    numchild[u] += numchild[v];
  }
  if ((c[u] >= numchild[u]))
  {
    write("NO");
    exit(0);
  }
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n);
  g.resize((n + 1));
  c.resize((n + 1));
  segtree.resize(((4 * n) + 5));
  numchild.resize((n + 1));
  ans.resize((n + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      var p: dynamic;
      read(p, c[i]);
      if ((!p))
      {
        root = i;
      } else
      {
        g[p].emplace_back(i);
      }
      i += 1;
    }
  }
  build();
  dfs(root);
  write("YES\n");
  {
    var i = 1;
    while ((i < ans.size()))
    {
      write(ans[i], cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
