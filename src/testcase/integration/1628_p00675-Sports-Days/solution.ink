// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(int)n;i++)");
}

func fr(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof(c.begin()) i=c.begin();i!=c.end();i++)");
}

var pb: dynamic = cpp_expression("#include<");

var mp: dynamic = cpp_expression("#include<");

func all(c: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream");
}

func dbg(x: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream> #includ");
}

var inf: dynamic = cpp_cast(1e9);

var EPS: dynamic = 1e-9;

var INF: dynamic = 1e12;

class Edge
{
  var src: dynamic = cpp_uninitialized();
  var dst: dynamic = cpp_uninitialized();
  var weight: dynamic = cpp_uninitialized();
  func Edge(src: dynamic, dst: dynamic, weight: dynamic) -> dynamic
  {
      self->src = cpp_construct(src);
      self->dst = cpp_construct(dst);
      self->weight = cpp_construct(weight);
    }
}

func operator_less(e: dynamic, f: dynamic) -> dynamic
{
  return  ((e.weight != f.weight)) ? (e.weight > f.weight) :  ((e.src != f.src)) ? (e.src < f.src) : (e.dst < f.dst);
}

func k_shortestPath(g: dynamic, s: dynamic, t: dynamic, k: dynamic, h: dynamic) -> dynamic
{
  var n: dynamic = g.size();
  var dist: dynamic = cpp_array(n);
  var Q: dynamic = cpp_uninitialized();
  Q.push(Edge(-1, s, 0));
  while ((!Q.empty()))
  {
    var e: dynamic = Q.top();
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

func buildFail(p: dynamic) -> dynamic
{
  var m: dynamic = strlen(p);
  var fail: dynamic = cpp_new();
  var j: dynamic = cpp_assign(fail[0], "=", -1);
  {
    var i: dynamic = 1;
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

func match_cpp(t: dynamic, p: dynamic, fail: dynamic) -> dynamic
{
  var n: dynamic = strlen(t);
  var m: dynamic = strlen(p);
  var count: dynamic = 0;
  {
    var i: dynamic = 0;
    var k: dynamic = 0;
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

var n: dynamic = cpp_uninitialized();

var col: dynamic = cpp_array(100);

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(1000);

var b: dynamic = cpp_array(1000);

var c: dynamic = cpp_array(1000);

var k: dynamic = cpp_uninitialized();

var ptn: dynamic = cpp_array(20);

var V: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var h: dynamic = cpp_array(1111);

var G: dynamic = cpp_uninitialized();

var rG: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(1111);

var rv: dynamic = cpp_array(1111);

func main() -> dynamic
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
    var ans: dynamic = k_shortestPath(G, s, t, k, h);
    var sum: dynamic = 0;
    rep(i, ans.size()) += ans[i];
    write(ans.size(), " ", sum, "\n");
  }
  return 0;
}

func dfs(c: dynamic, g: dynamic, v: dynamic) -> dynamic
{
  v[c] = 1;
  fr(i, g[c]);
  if ((!v[i->dst]))
  {
    dfs(i->dst, g, v);
  }
}

func calc_potential() -> dynamic
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

func push(g: dynamic, src: dynamic, dst: dynamic, weight: dynamic) -> dynamic
{
  g[src].pb(Edge(src, dst, weight));
}

func make_graph() -> dynamic
{
  var fail: dynamic = buildFail(ptn);
  var len: dynamic = strlen(ptn);
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

func fr(argument_0: dynamic, i: dynamic) -> dynamic
{
      var to: dynamic = j;
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    push(G, ((((n - 1)) * ((len + 1))) + j), t, 0);
    push(rG, t, ((((n - 1)) * ((len + 1))) + j), 0);
  }
