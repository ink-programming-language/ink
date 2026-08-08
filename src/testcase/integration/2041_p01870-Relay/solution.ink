// Translated from solution.cpp.

var int_cpp = dynamic;

var pb = cpp_expression("#include<");

var mp = cpp_expression("#include<");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func all(v: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func reps(i: dynamic, f: dynamic, n: dynamic)
{
  cpp_macro("for(int i=(f);i<(n);i++)");
}

func each(it: dynamic, v: dynamic)
{
  cpp_macro("for(__typeof((v).begin()) it=(v).begin();it!=(v).end();it++)");
}

func chmin(t: dynamic, f: dynamic)
{
  if ((t > f))
  {
    t = f;
  }
}

func chmax(t: dynamic, f: dynamic)
{
  if ((t < f))
  {
    t = f;
  }
}

class segtree
{
  var SEG: dynamic;
  var dat: dynamic;
  func segtree()
  {
      this->dat = cpp_construct((SEG * 2));
    }
  func update(k: dynamic, x: dynamic)
  {
      k += (SEG - 1);
      dat[k] = x;
      while (k)
      {
        k = (((k - 1)) / 2);
        dat[k] = max(dat[((k * 2) + 1)], dat[((k * 2) + 2)]);
      }
    }
  func get(a: dynamic, b: dynamic, k: dynamic = 0, l: dynamic = 0, r: dynamic = SEG)
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
  var to: dynamic;
  var cost: dynamic;
  func edge(to: dynamic, cost: dynamic)
  {
      this->to = cpp_construct(to);
      this->cost = cpp_construct(cost);
    }
}

var SIZE = 100000;

var N: dynamic;

var G = cpp_array(SIZE);

var tt: dynamic;

var tin = cpp_array(SIZE);

var tout = cpp_array(SIZE);

var seg: dynamic;

var ans: dynamic;

class data
{
  var len: dynamic;
  var cost: dynamic;
  var id: dynamic;
  func data(len: dynamic, cost: dynamic, id: dynamic)
  {
      this->len = cpp_construct(len);
      this->cost = cpp_construct(cost);
      this->id = cpp_construct(id);
    }
  func operator_less(d: dynamic)
  {
      return (len < d.len);
    }
}

var maxlen = cpp_array(SIZE);

var maxcost = cpp_array(SIZE);

var maxpair = cpp_array(SIZE);

func comp(a: dynamic, b: dynamic)
{
  return ((a.fi + a.se) > (b.fi + b.se));
}

func solve(v: dynamic, p: dynamic)
{
  var hoge = max(seg.get(0, (tin[v] + 1)), seg.get(tout[v], segtree.SEG));
  var vec = cpp_construct(3, data(0, 0, -1));
  for (var e in G[v])
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
    var i = 2;
    while ((i < vec.size()))
    {
      chmax(ans, ((vec[0].len + vec[1].len) + vec[i].cost));
      i += 1;
    }
  }
  for (var e in G[v])
  {
    if ((e.to == p))
    {
      continue;
    }
    var tmp = vec[0].len;
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
  var maxl = cpp_construct(vec.size(), 0);
  var maxr = cpp_construct(vec.size(), 0);
  {
    var i = 0;
    while ((i < (vec.size() - 1)))
    {
      chmax(maxl[(i + 1)], max(maxl[i], vec[i].cost));
      i += 1;
    }
  }
  {
    var i = (vec.size() - 1);
    while ((i > 0))
    {
      chmax(maxr[(i - 1)], max(maxr[i], vec[i].cost));
      i -= 1;
    }
  }
  rep(i, vec.size());
  {
    var tmp = max(maxl[i], maxr[i]);
    var p = cpp_construct(vec[i].len, tmp);
    if (comp(p, maxpair[v]))
    {
      maxpair[v] = p;
    }
  }
  maxlen[v] = vec[0].len;
}

func dfs()
{
  var v: dynamic;
  var p: dynamic;
  var sz = cpp_construct(N, 1);
  v.push(0);
  p.push(-1);
  while (v.size())
  {
    var vv = v.top();
    v.pop();
    var pp = p.top();
    p.pop();
    tin[vv] = cpp_update(tt, "++");
    for (var e in G[vv])
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
  var vec: dynamic;
  var par: dynamic;
  vec.pb(0);
  par.pb(-1);
  {
    var i = (N - 1);
    while ((i > 0))
    {
      sz[par[i]] += sz[vec[i]];
      i -= 1;
    }
  }
  rep(i, N)[i] = (tin[i] + sz[i]);
  {
    var i = (N - 1);
    while ((i >= 0))
    {
      solve(vec[i], par[i]);
      i -= 1;
    }
  }
}

func main()
{
  scanf("%lld", (&N));
  reps(i, 1, N);
  {
    var a: dynamic;
    var b: dynamic;
    scanf("%lld%lld", (&a), (&b));
    G[i].pb(edge(a, b));
    G[a].pb(edge(i, b));
  }
  dfs();
  printf("%lld\n", ans);
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    for (var e in G[vec[i]])
    {
      if ((e.to == par[i]))
      {
        continue;
      }
      vec.pb(e.to);
      par.pb(vec[i]);
    }
  }
