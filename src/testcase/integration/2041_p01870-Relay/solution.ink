// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var pb: dynamic = cpp_expression("#include<");

var mp: dynamic = cpp_expression("#include<");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

func all(v: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func reps(i: dynamic, f: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=(f);i<(n);i++)");
}

func each(it: dynamic, v: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((v).begin()) it=(v).begin();it!=(v).end();it++)");
}

func chmin(t: dynamic, f: dynamic) -> dynamic
{
  if ((t > f))
  {
    t = f;
  }
}

func chmax(t: dynamic, f: dynamic) -> dynamic
{
  if ((t < f))
  {
    t = f;
  }
}

class segtree
{
  var SEG: dynamic = cpp_uninitialized();
  var dat: dynamic = cpp_uninitialized();
  func segtree() -> dynamic
  {
      self->dat = cpp_construct((SEG * 2));
    }
  func update(k: dynamic, x: dynamic) -> dynamic
  {
      k += (SEG - 1);
      dat[k] = x;
      while (k)
      {
        k = (((k - 1)) / 2);
        dat[k] = max(dat[((k * 2) + 1)], dat[((k * 2) + 2)]);
      }
    }
  func get(a: dynamic, b: dynamic, k: dynamic = 0, l: dynamic = 0, r: dynamic = SEG) -> dynamic
  {
      if (((r <= a) || (b <= l)))
      {
        return 0;
      }
      if (((a <= l) && (r <= b)))
      {
        return dat[k];
      }
      return max(get(a, b, ((k * 2) + 1), l, (((l + r)) / 2)), get(a, b, ((k * 2) + 2), (((l + r)) / 2), r));
    }
}

class edge
{
  var to: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
  func edge(to: dynamic, cost: dynamic) -> dynamic
  {
      self->to = cpp_construct(to);
      self->cost = cpp_construct(cost);
    }
}

var SIZE: dynamic = 100000;

var N: dynamic = cpp_uninitialized();

var G: dynamic = cpp_array(SIZE);

var tt: dynamic = cpp_uninitialized();

var tin: dynamic = cpp_array(SIZE);

var tout: dynamic = cpp_array(SIZE);

var seg: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

class data
{
  var len: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
  func data(len: dynamic, cost: dynamic, id: dynamic) -> dynamic
  {
      self->len = cpp_construct(len);
      self->cost = cpp_construct(cost);
      self->id = cpp_construct(id);
    }
  func operator_less(d: dynamic) -> dynamic
  {
      return (len < d.len);
    }
}

var maxlen: dynamic = cpp_array(SIZE);

var maxcost: dynamic = cpp_array(SIZE);

var maxpair: dynamic = cpp_array(SIZE);

func comp(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.fi + a.se) > (b.fi + b.se));
}

func solve(v: dynamic, p: dynamic) -> dynamic
{
  var hoge: dynamic = max(seg.get(0, (tin[v] + 1)), seg.get(tout[v], segtree.SEG));
  var vec: dynamic = cpp_construct(3, data(0, 0, -1));
  for (var e: dynamic in G[v])
  {
    if ((e.to == p))
    {
      continue;
    }
    maxlen[e.to] += e.cost;
    chmax(maxcost[e.to], e.cost);
    maxpair[e.to].fi += e.cost;
    vec.pb(data(maxlen[e.to], maxcost[e.to], e.to));
  }
  sort(all(vec));
  reverse(all(vec));
  chmax(ans, ((vec[0].len + vec[1].len) + hoge));
  chmax(ans, ((vec[1].len + vec[2].len) + vec[0].cost));
  chmax(ans, ((vec[0].len + vec[2].len) + vec[1].cost));
  {
    var i: dynamic = 2;
    while ((i < vec.size()))
    {
      chmax(ans, ((vec[0].len + vec[1].len) + vec[i].cost));
      i += 1;
    }
  }
  for (var e: dynamic in G[v])
  {
    if ((e.to == p))
    {
      continue;
    }
    var tmp: dynamic = vec[0].len;
    if ((e.to == vec[0].id))
    {
      tmp = vec[1].len;
    }
    chmax(ans, ((tmp + maxpair[e.to].fi) + maxpair[e.to].se));
    chmax(maxcost[v], maxcost[e.to]);
    if (comp(maxpair[e.to], maxpair[v]))
    {
      maxpair[v] = maxpair[e.to];
    }
  }
  var maxl: dynamic = cpp_construct(vec.size(), 0);
  var maxr: dynamic = cpp_construct(vec.size(), 0);
  {
    var i: dynamic = 0;
    while ((i < (vec.size() - 1)))
    {
      chmax(maxl[(i + 1)], max(maxl[i], vec[i].cost));
      i += 1;
    }
  }
  {
    var i: dynamic = (vec.size() - 1);
    while ((i > 0))
    {
      chmax(maxr[(i - 1)], max(maxr[i], vec[i].cost));
      i -= 1;
    }
  }
  rep(i, vec.size());
  {
    var tmp: dynamic = max(maxl[i], maxr[i]);
    var p: dynamic = cpp_construct(vec[i].len, tmp);
    if (comp(p, maxpair[v]))
    {
      maxpair[v] = p;
    }
  }
  maxlen[v] = vec[0].len;
}

func dfs() -> dynamic
{
  var v: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var sz: dynamic = cpp_construct(N, 1);
  v.push(0);
  p.push(-1);
  while (v.size())
  {
    var vv: dynamic = v.top();
    v.pop();
    var pp: dynamic = p.top();
    p.pop();
    tin[vv] = cpp_update(tt, "++");
    for (var e: dynamic in G[vv])
    {
      if ((e.to == pp))
      {
        continue;
      }
      seg.update(tt, e.cost);
      v.push(e.to);
      p.push(vv);
    }
  }
  var vec: dynamic = cpp_uninitialized();
  var par: dynamic = cpp_uninitialized();
  vec.pb(0);
  par.pb(-1);
  {
    var i: dynamic = (N - 1);
    while ((i > 0))
    {
      sz[par[i]] += sz[vec[i]];
      i -= 1;
    }
  }
  rep(i, N)[i] = (tin[i] + sz[i]);
  {
    var i: dynamic = (N - 1);
    while ((i >= 0))
    {
      solve(vec[i], par[i]);
      i -= 1;
    }
  }
}

func main() -> dynamic
{
  scanf("%lld", (&N));
  reps(i, 1, N);
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    scanf("%lld%lld", (&a), (&b));
    G[i].pb(edge(a, b));
    G[a].pb(edge(i, b));
  }
  dfs();
  printf("%lld\n", ans);
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    for (var e: dynamic in G[vec[i]])
    {
      if ((e.to == par[i]))
      {
        continue;
      }
      vec.pb(e.to);
      par.pb(vec[i]);
    }
  }
