// Translated from solution.cpp.

func read(first: dynamic)
{
  var ch: dynamic;
  {
    ch = getchar();
    while (((((ch < cpp_char("0")) || (ch > cpp_char("9")))) && (ch != cpp_char("-"))))
    {
      ch = getchar();
    }
  }
  first = 0;
  var t = 1;
  if ((ch == cpp_char("-")))
  {
    ch = getchar();
    t = -1;
  }
  {
    while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
    {
      first = (((first * 10) + ch) - cpp_char("0"));
      ch = getchar();
    }
  }
  first *= t;
}

var N = 100010;

var inf = 1000000000000000000;

class segtree
{
  var ma: dynamic;
  var mi: dynamic;
  var delta: dynamic;
  var lch: dynamic;
  var rch: dynamic;
  var lnum: dynamic;
  var rnum: dynamic;
}

var tree = cpp_array((4 * N));

var cnt: dynamic;

var a = cpp_array(N);

var op = cpp_array(N);

var c = cpp_array(2, N);

var l = cpp_array(N);

var r = cpp_array(N);

var deg = cpp_array(N);

var t = cpp_array(N);

var s = cpp_array(N);

var b = cpp_array((2 * N));

func build(k: dynamic, l: dynamic, r: dynamic)
{
  tree[k].lnum = l;
  tree[k].rnum = r;
  tree[k].delta = 0;
  tree[k].ma = (-inf);
  tree[k].mi = inf;
  if ((l == r))
  {
    return;
  }
  var mid = (((l + r)) >> 1);
  tree[k].lch = cpp_update(cnt, "++");
  build(cnt, l, mid);
  tree[k].rch = cpp_update(cnt, "++");
  build(cnt, (mid + 1), r);
}

func update(k: dynamic, d: dynamic)
{
  tree[k].mi += d;
  tree[k].ma += d;
  tree[k].delta += d;
}

func pushdown(k: dynamic)
{
  if (tree[k].delta)
  {
    update(tree[k].lch, tree[k].delta);
    update(tree[k].rch, tree[k].delta);
    tree[k].delta = 0;
  }
}

func pushup(k: dynamic)
{
  tree[k].mi = min(tree[tree[k].lch].mi, tree[tree[k].rch].mi);
  tree[k].ma = max(tree[tree[k].lch].ma, tree[tree[k].rch].ma);
}

func change1(k: dynamic, l: dynamic, r: dynamic, d: dynamic)
{
  if (((l <= tree[k].lnum) && (r >= tree[k].rnum)))
  {
    update(k, d);
    return;
  }
  pushdown(k);
  if ((l < tree[tree[k].rch].lnum))
  {
    change1(tree[k].lch, l, r, d);
  }
  if ((r > tree[tree[k].lch].rnum))
  {
    change1(tree[k].rch, l, r, d);
  }
  pushup(k);
}

func change2(k: dynamic, p: dynamic, d1: dynamic, d2: dynamic)
{
  if ((tree[k].lnum == tree[k].rnum))
  {
    tree[k].mi = d1;
    tree[k].ma = d2;
    return;
  }
  pushdown(k);
  if ((p < tree[tree[k].rch].lnum))
  {
    change2(tree[k].lch, p, d1, d2);
  } else
  {
    change2(tree[k].rch, p, d1, d2);
  }
  pushup(k);
}

func query(k: dynamic, p: dynamic)
{
  if ((tree[k].lnum == tree[k].rnum))
  {
    return tree[k].mi;
  }
  pushdown(k);
  if ((p < tree[tree[k].rch].lnum))
  {
    return query(tree[k].lch, p);
  } else
  {
    return query(tree[k].rch, p);
  }
}

func main()
{
  var n: dynamic;
  var q: dynamic;
  read(n);
  read(q);
  {
    var i = 1;
    while ((i <= n))
    {
      read(t[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      b[i] = make_pair(a[i], i);
      i += 1;
    }
  }
  var m = n;
  {
    var i = 1;
    while ((i <= q))
    {
      read(op[i]);
      if ((op[i] < 3))
      {
        read(c[i][0]);
        if ((op[i] == 1))
        {
          read(c[i][1]);
          b[cpp_update(m, "++")] = make_pair(c[i][1], c[i][0]);
        }
      }
      i += 1;
    }
  }
  sort((b + 1), ((b + m) + 1));
  var tm = 1;
  {
    var i = 2;
    while ((i <= m))
    {
      if ((b[(i - 1)] != b[i]))
      {
        b[cpp_update(tm, "++")] = b[i];
      }
      i += 1;
    }
  }
  m = tm;
  {
    var i = 1;
    while ((i <= n))
    {
      l[i] = (m + 1);
      r[i] = 0;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      r[b[i].first] = i;
      i += 1;
    }
  }
  {
    var i = m;
    while (i)
    {
      l[b[i].first] = i;
      i -= 1;
    }
  }
  cnt = 0;
  build(0, 1, m);
  {
    var i = 1;
    while ((i <= n))
    {
      deg[a[i]] += 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      s[i] = ((t[i] - ((t[i] / ((deg[i] + 2))) * ((deg[i] + 1)))) + (t[a[i]] / ((deg[a[i]] + 2))));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      s[a[i]] += (t[i] / ((deg[i] + 2)));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      change2(0, (lower_bound((b + 1), ((b + m) + 1), make_pair(a[i], i)) - b), s[i], s[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= q))
    {
      if ((op[i] == 1))
      {
        var first = c[i][0];
        var second = a[c[i][0]];
        var z = c[i][1];
        var w = ((t[second] / ((deg[second] + 1))) - (t[second] / ((deg[second] + 2))));
        change1(0, l[second], r[second], w);
        var p = (lower_bound((b + 1), ((b + m) + 1), make_pair(a[a[second]], a[second])) - b);
        change1(0, p, p, w);
        w = (((((-t[second]) / ((deg[second] + 1))) * deg[second]) + ((t[second] / ((deg[second] + 2))) * ((deg[second] + 1)))) - (t[first] / ((deg[first] + 2))));
        p = (lower_bound((b + 1), ((b + m) + 1), make_pair(a[second], second)) - b);
        change1(0, p, p, w);
        p = (lower_bound((b + 1), ((b + m) + 1), make_pair(second, first)) - b);
        var v = (query(0, p) - (t[second] / ((deg[second] + 1))));
        change2(0, p, inf, (-inf));
        deg[second] -= 1;
        deg[z] += 1;
        w = ((t[z] / ((deg[z] + 2))) - (t[z] / ((deg[z] + 1))));
        change1(0, l[z], r[z], w);
        p = (lower_bound((b + 1), ((b + m) + 1), make_pair(a[a[z]], a[z])) - b);
        change1(0, p, p, w);
        w = (((((-t[z]) / ((deg[z] + 2))) * ((deg[z] + 1))) + ((t[z] / ((deg[z] + 1))) * deg[z])) + (t[first] / ((deg[first] + 2))));
        p = (lower_bound((b + 1), ((b + m) + 1), make_pair(a[z], z)) - b);
        change1(0, p, p, w);
        v += (t[z] / ((deg[z] + 2)));
        p = (lower_bound((b + 1), ((b + m) + 1), make_pair(z, first)) - b);
        change2(0, p, v, v);
        a[first] = z;
      }
      if ((op[i] == 2))
      {
        printf("%I64d\n", query(0, (lower_bound((b + 1), ((b + m) + 1), make_pair(a[c[i][0]], c[i][0])) - b)));
      }
      if ((op[i] == 3))
      {
        printf("%I64d %I64d\n", tree[0].mi, tree[0].ma);
      }
      i += 1;
    }
  }
  return 0;
}
