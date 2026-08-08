// Translated from solution.cpp.

var maxn = (3e5 + 100);

var md = (1e9 + 7);

var add = cpp_array((maxn * 4));

var seg = cpp_array((maxn * 4));

var lazy1 = cpp_array((maxn * 4));

var lazy2 = cpp_array((maxn * 4));

var adj = cpp_array(maxn);

var hgt = cpp_array(maxn);

var ftm = cpp_array(maxn);

var stm = cpp_array(maxn);

var n: dynamic;

var type_cpp: dynamic;

var tm: dynamic;

var q: dynamic;

func dfs(v: dynamic, h: dynamic)
{
  stm[v] = cpp_update(tm, "++");
  hgt[v] = h;
  {
    var i = 0;
    while ((i < cpp_cast(adj[v].size())))
    {
      dfs(adj[v][i], (h + 1));
      i += 1;
    }
  }
  ftm[v] = tm;
}

func add_num(fi: dynamic, se: dynamic, val1: dynamic, val2: dynamic, be: dynamic, en: dynamic, ind: dynamic)
{
  if (((fi >= en) || (se <= be)))
  {
    return;
  }
  if (((fi <= be) && (en <= se)))
  {
    lazy1[ind] = ((1 * ((lazy1[ind] + val1))) % md);
    lazy2[ind] = ((1 * ((lazy2[ind] + val2))) % md);
    return;
  }
  var mid = (((be + en)) / 2);
  add_num(fi, se, val1, val2, be, mid, ((ind * 2) + 1));
  add_num(fi, se, val1, val2, mid, en, ((ind * 2) + 2));
}

func get_ans(id: dynamic, be: dynamic, en: dynamic, ind: dynamic)
{
  if (((id < be) || (id >= en)))
  {
    return -1;
  }
  if (((en - be) <= 1))
  {
    add[ind] = (((add[ind] + lazy1[ind])) % md);
    lazy1[ind] = 0;
    seg[ind] = (((seg[ind] + lazy2[ind])) % md);
    lazy2[ind] = 0;
    return ind;
  }
  var mid = (((be + en)) / 2);
  lazy1[((ind * 2) + 1)] += lazy1[ind];
  lazy1[((ind * 2) + 2)] += lazy1[ind];
  add[ind] += lazy1[ind];
  lazy1[ind] = 0;
  lazy2[((ind * 2) + 1)] += lazy2[ind];
  lazy2[((ind * 2) + 2)] += lazy2[ind];
  seg[ind] += lazy2[ind];
  lazy2[ind] = 0;
  lazy1[((ind * 2) + 1)] %= md;
  lazy1[((ind * 2) + 2)] %= md;
  add[ind] %= md;
  lazy2[((ind * 2) + 1)] %= md;
  lazy2[((ind * 2) + 2)] %= md;
  seg[ind] %= md;
  var ret1 = get_ans(id, be, mid, ((ind * 2) + 1));
  var ret2 = get_ans(id, mid, en, ((ind * 2) + 2));
  return max(ret1, ret2);
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  read(n);
  var a: dynamic;
  var v: dynamic;
  var type_cpp: dynamic;
  var x: dynamic;
  var k: dynamic;
  {
    var i = 1;
    while ((i < n))
    {
      read(a);
      adj[cpp_update(a, "--")].push_back(i);
      i += 1;
    }
  }
  dfs(0, 0);
  read(q);
  {
    var i = 0;
    while ((i < q))
    {
      read(type_cpp);
      if ((type_cpp == 1))
      {
        read(v, x, k);
        v -= 1;
        add_num(stm[v], ftm[v], ((1 * ((x + ((((1 * hgt[v]) * k) % md))))) % md), k, 0, maxn, 0);
      }
      if ((type_cpp == 2))
      {
        read(v);
        v -= 1;
        var pl = get_ans(stm[v], 0, maxn, 0);
        write(((((1 * ((add[pl] + md))) - (((((1 * hgt[v]) * seg[pl])) % md)))) % md), "\n");
      }
      i += 1;
    }
  }
  return 0;
}
