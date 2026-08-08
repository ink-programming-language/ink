// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(int)n;i++)");
}

func fr(i: dynamic, c: dynamic)
{
  cpp_macro("for(__typeof(c.begin()) i=c.begin();i!=c.end();i++)");
}

var pb = cpp_expression("#include<");

var mp = cpp_expression("#include<");

func all(c: dynamic)
{
  return cpp_expression("#include<iostream");
}

func dbg(x: dynamic)
{
  return cpp_expression("#include<iostream> #includ");
}

var inf = cpp_cast(1e9);

var EPS = 1e-9;

var INF = 1e12;

class Edge
{
  var src: dynamic;
  var dst: dynamic;
  var weight: dynamic;
  func Edge(src: dynamic, dst: dynamic, weight: dynamic)
  {
      this->src = cpp_construct(src);
      this->dst = cpp_construct(dst);
      this->weight = cpp_construct(weight);
    }
}

func operator_less(e: dynamic, f: dynamic)
{
  return if ((e.weight != f.weight)) (e.weight > f.weight) else if ((e.src != f.src)) (e.src < f.src) else (e.dst < f.dst);
}

func k_shortestPath(g: dynamic, s: dynamic, t: dynamic, k: dynamic, h: dynamic)
{
  var n = g.size();
  var dist = cpp_array(n);
  var Q: dynamic;
  Q.push(Edge(-1, s, 0));
  while ((!Q.empty()))
  {
    var e = Q.top();
    Q.pop();
    if ((dist[e.dst].size() >= k))
    {
      continue;
    }
    dist[e.dst].push_back(e.weight);
    fr(f, g[e.dst]).push(Edge(f->src, f->dst, (((f->weight + e.weight) + h[f->src]) - h[f->dst])));
  }
  rep(i, dist[t].size())[t][i] += (h[t] - h[s]);
  return dist[t];
}

func buildFail(p: dynamic)
{
  var m = strlen(p);
  var fail = cpp_new();
  var j = cpp_assign(fail[0], "=", -1);
  {
    var i = 1;
    while ((i <= m))
    {
      while (((j >= 0) && (p[j] != p[(i - 1)])))
      {
        j = fail[j];
      }
      fail[i] = cpp_update(j, "++");
      i += 1;
    }
  }
  return fail;
}

func match_cpp(t: dynamic, p: dynamic, fail: dynamic)
{
  var n = strlen(t);
  var m = strlen(p);
  var count = 0;
  {
    var i = 0;
    var k = 0;
    while ((i < n))
    {
      while (((k >= 0) && (p[k] != t[i])))
      {
        k = fail[k];
      }
      if ((cpp_update(k, "++") >= m))
      {
        count += 1;
        k = fail[k];
      }
      i += 1;
    }
  }
  return count;
}

var n: dynamic;

var col = cpp_array(100);

var m: dynamic;

var a = cpp_array(1000);

var b = cpp_array(1000);

var c = cpp_array(1000);

var k: dynamic;

var ptn = cpp_array(20);

var V: dynamic;

var s: dynamic;

var t: dynamic;

var h = cpp_array(1111);

var G: dynamic;

var rG: dynamic;

var v = cpp_array(1111);

var rv = cpp_array(1111);

func main()
{
  while (cpp_comma((cin >> n), n))
  {
    cpp_statement("rep(i,n)cin>>col[i]; cin>>m; rep(i,m)");
    read(a[i], b[i], c[i]);
    read(k, ptn);
    if ((!make_graph()))
    {
      write("0 0", "\n");
      continue;
    }
    if ((!calc_potential()))
    {
      write(-1, "\n");
      continue;
    }
    var ans = k_shortestPath(G, s, t, k, h);
    var sum = 0;
    rep(i, ans.size()) += ans[i];
    write(ans.size(), " ", sum, "\n");
  }
  return 0;
}

func dfs(c: dynamic, g: dynamic, v: dynamic)
{
  v[c] = 1;
  fr(i, g[c]);
  if ((!v[i->dst]))
  {
    dfs(i->dst, g, v);
  }
}

func calc_potential()
{
  fill(h, ((h + V) + 1), 0);
  rep(k, V);
  rep(i, V);
  if ((v[i] && rv[i]))
  {
    fr(e, G[i]);
  }
  {
    if ((h[e->dst] > (h[e->src] + e->weight)))
    {
      h[e->dst] = (h[e->src] + e->weight);
      if ((k == (V - 1)))
      {
        return 0;
      }
    }
  }
  return 1;
}

func push(g: dynamic, src: dynamic, dst: dynamic, weight: dynamic)
{
  g[src].pb(Edge(src, dst, weight));
}

func make_graph()
{
  var fail = buildFail(ptn);
  var len = strlen(ptn);
  G.clear();
  rG.clear();
  V = ((n * ((len + 1))) + 1);
  s = 0;
  t = (V - 1);
  while (((s >= 0) && ((ptn[s] - cpp_char("0")) != col[0])))
  {
    s = fail[s];
  }
  if ((cpp_update(s, "++") >= len))
  {
    return 0;
  }
  G.resize(V);
  rG.resize(V);
  rep(i, m)[(a[i] - 1)].pb(mp((b[i] - 1), c[i]));
  rep(i, n);
  rep(j, (len + 1));
  {
  }
  rep(i, V)[i] = cpp_assign(rv[i], "=", 0);
  dfs(s, G, v);
  dfs(t, rG, rv);
  return (v[t] && rv[s]);
}

func fr(argument_0: dynamic, i: dynamic)
{
      var to = j;
      while (((to >= 0) && ((ptn[to] - cpp_char("0")) != col[k->first])))
      {
        to = fail[to];
      }
      if ((cpp_update(to, "++") >= len))
      {
        continue;
      }
      push(G, ((i * ((len + 1))) + j), ((k->first * ((len + 1))) + to), k->second);
      push(rG, ((k->first * ((len + 1))) + to), ((i * ((len + 1))) + j), 0);
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    push(G, ((((n - 1)) * ((len + 1))) + j), t, 0);
    push(rG, t, ((((n - 1)) * ((len + 1))) + j), 0);
  }
