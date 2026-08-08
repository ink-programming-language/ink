// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var INF = (1 << 29);

class point
{
  var x: dynamic;
  var y: dynamic;
  func point()
  {
    }
  func point(x: dynamic, y: dynamic)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
  func operator_equal(a: dynamic)
  {
      return ((x == a.x) && (y == a.y));
    }
}

func cmp_point_1(a: dynamic, b: dynamic)
{
  return ((a.x > b.x) || ((a.x == b.x) && (a.y > b.y)));
}

func cmp_point_2(a: dynamic, b: dynamic)
{
  return ((a.y < b.y) || ((a.y == b.y) && (a.x < b.x)));
}

func cmp_point_3(a: dynamic, b: dynamic)
{
  return ((a.x < b.x) || ((a.x == b.x) && (a.y < b.y)));
}

func cmp_point_4(a: dynamic, b: dynamic)
{
  return ((a.y > b.y) || ((a.y == b.y) && (a.x > b.x)));
}

class segment_tree_max
{
  var N_MAX: dynamic;
  var n: dynamic;
  var dat: dynamic = cpp_array((2 * N_MAX));
  func update_max(l: dynamic, r: dynamic, a: dynamic, b: dynamic, u: dynamic, v: dynamic)
  {
      if (((l <= a) && (b <= r)))
      {
        dat[u] = max(dat[u], v);
        return;
      }
      var c = ((((a + b) + 1)) / 2);
      if (((l < c) && (a < r)))
      {
        update_max(l, r, a, c, (2 * u), v);
      }
      if (((l < b) && (c < r)))
      {
        update_max(l, r, c, b, ((2 * u) + 1), v);
      }
    }
  func build(N: dynamic, val: dynamic)
  {
      {
        n = 1;
        while ((n < N))
        {
          n *= 2;
        }
      }
      rep(u, (2 * n))[u] = val;
    }
  func update_max(l: dynamic, r: dynamic, v: dynamic)
  {
      update_max(l, r, 0, n, 1, v);
    }
  func query(u: dynamic)
  {
      u += n;
      var res = dat[u];
      {
        u /= 2;
        while ((u >= 1))
        {
          res = max(res, dat[u]);
          u /= 2;
        }
      }
      return res;
    }
}

class segment_tree_min
{
  var N_MAX: dynamic;
  var n: dynamic;
  var dat: dynamic = cpp_array((2 * N_MAX));
  func update_min(l: dynamic, r: dynamic, a: dynamic, b: dynamic, u: dynamic, v: dynamic)
  {
      if (((l <= a) && (b <= r)))
      {
        dat[u] = min(dat[u], v);
        return;
      }
      var c = ((((a + b) + 1)) / 2);
      if (((l < c) && (a < r)))
      {
        update_min(l, r, a, c, (2 * u), v);
      }
      if (((l < b) && (c < r)))
      {
        update_min(l, r, c, b, ((2 * u) + 1), v);
      }
    }
  func build(N: dynamic, val: dynamic)
  {
      {
        n = 1;
        while ((n < N))
        {
          n *= 2;
        }
      }
      rep(u, (2 * n))[u] = val;
    }
  func update_min(l: dynamic, r: dynamic, v: dynamic)
  {
      update_min(l, r, 0, n, 1, v);
    }
  func query(u: dynamic)
  {
      u += n;
      var res = dat[u];
      {
        u /= 2;
        while ((u >= 1))
        {
          res = min(res, dat[u]);
          u /= 2;
        }
      }
      return res;
    }
}

class event
{
  var type_cpp: dynamic;
  var x1: dynamic;
  var x2: dynamic;
  var y1: dynamic;
  var y2: dynamic;
  func event()
  {
    }
  func event(type_cpp: dynamic, x1: dynamic, x2: dynamic, y1: dynamic, y2: dynamic)
  {
      this->type_cpp = cpp_construct(type_cpp);
      this->x1 = cpp_construct(x1);
      this->x2 = cpp_construct(x2);
      this->y1 = cpp_construct(y1);
      this->y2 = cpp_construct(y2);
    }
}

func cmp_event_1(e: dynamic, f: dynamic)
{
  return (e.y1 < f.y1);
}

func cmp_event_2(e: dynamic, f: dynamic)
{
  return (e.y1 > f.y1);
}

func cmp_event_3(e: dynamic, f: dynamic)
{
  return (e.x1 < f.x1);
}

func cmp_event_4(e: dynamic, f: dynamic)
{
  return (e.x1 > f.x1);
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  var l = cpp_array(40000);
  var t = cpp_array(40000);
  var r = cpp_array(40000);
  var b = cpp_array(40000);
  rep(i, n);
  scanf("%d%d%d%d", (l + i), (t + i), (r + i), (b + i));
  var X: dynamic;
  var Y: dynamic;
  sort(X.begin(), X.end());
  sort(Y.begin(), Y.end());
  X.erase(unique(X.begin(), X.end()), X.end());
  Y.erase(unique(Y.begin(), Y.end()), Y.end());
  var P1: dynamic;
  var P2: dynamic;
  var P3: dynamic;
  var P4: dynamic;
  {
    var Seg1: dynamic;
    Seg1.build(X.size(), (-INF));
    var E: dynamic;
    sort(E.begin(), E.end(), cmp_event_1);
    rep(i, E.size());
    {
      var type_cpp = E[i].type_cpp;
      if ((type_cpp == 0))
      {
        var x = E[i].x1;
        var res = Seg1.query(x);
        if ((res > (-INF)))
        {
          P1.push_back(point(x, res));
        }
      } else
      {
        var y = E[i].y1;
        var x1 = E[i].x1;
        var x2 = E[i].x2;
        if (((x1 + 1) <= (x2 - 1)))
        {
          Seg1.update_max((x1 + 1), x2, y);
        }
      }
    }
  }
  {
    var Seg2: dynamic;
    Seg2.build(X.size(), INF);
    var E: dynamic;
    sort(E.begin(), E.end(), cmp_event_2);
    rep(i, E.size());
    {
      var type_cpp = E[i].type_cpp;
      if ((type_cpp == 0))
      {
        var x = E[i].x1;
        var res = Seg2.query(x);
        if ((res < INF))
        {
          P2.push_back(point(x, res));
        }
      } else
      {
        var y = E[i].y1;
        var x1 = E[i].x1;
        var x2 = E[i].x2;
        if (((x1 + 1) <= (x2 - 1)))
        {
          Seg2.update_min((x1 + 1), x2, y);
        }
      }
    }
  }
  {
    var Seg3: dynamic;
    Seg3.build(Y.size(), (-INF));
    var E: dynamic;
    sort(E.begin(), E.end(), cmp_event_3);
    rep(i, E.size());
    {
      var type_cpp = E[i].type_cpp;
      if ((type_cpp == 0))
      {
        var y = E[i].y1;
        var res = Seg3.query(y);
        if ((res > (-INF)))
        {
          P3.push_back(point(res, y));
        }
      } else
      {
        var x = E[i].x1;
        var y1 = E[i].y1;
        var y2 = E[i].y2;
        if (((y1 + 1) <= (y2 - 1)))
        {
          Seg3.update_max((y1 + 1), y2, x);
        }
      }
    }
  }
  {
    var Seg4: dynamic;
    Seg4.build(Y.size(), INF);
    var E: dynamic;
    sort(E.begin(), E.end(), cmp_event_4);
    rep(i, E.size());
    {
      var type_cpp = E[i].type_cpp;
      if ((type_cpp == 0))
      {
        var y = E[i].y1;
        var res = Seg4.query(y);
        if ((res < INF))
        {
          P4.push_back(point(res, y));
        }
      } else
      {
        var x = E[i].x1;
        var y1 = E[i].y1;
        var y2 = E[i].y2;
        if (((y1 + 1) <= (y2 - 1)))
        {
          Seg4.update_min((y1 + 1), y2, x);
        }
      }
    }
  }
  swap(P2, P4);
  swap(P3, P4);
  sort(P1.begin(), P1.end(), cmp_point_1);
  sort(P2.begin(), P2.end(), cmp_point_2);
  sort(P3.begin(), P3.end(), cmp_point_3);
  sort(P4.begin(), P4.end(), cmp_point_4);
  P1.erase(unique(P1.begin(), P1.end()), P1.end());
  P2.erase(unique(P2.begin(), P2.end()), P2.end());
  P3.erase(unique(P3.begin(), P3.end()), P3.end());
  P4.erase(unique(P4.begin(), P4.end()), P4.end());
  var m1 = P1.size();
  var m2 = P2.size();
  var m3 = P3.size();
  var m4 = P4.size();
  var m = (((m1 + m2) + m3) + m4);
  rep(u, m);
  rep(i, G[u].size())[G[u][i]] += 1;
  var ans = m;
  var Q: dynamic;
  rep(u, m);
  if ((deg[u] == 0))
  {
    Q.push(u);
  }
  while ((!Q.empty()))
  {
    var u = Q.front();
    Q.pop();
    ans -= 1;
    rep(i, G[u].size());
    {
      var v = G[u][i];
      deg[v] -= 1;
      if ((deg[v] == 0))
      {
        Q.push(v);
      }
    }
  }
  printf("%d\n", ans);
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    X.push_back(l[i]);
    X.push_back((l[i] + 1));
    X.push_back((l[i] - 1));
    X.push_back(r[i]);
    X.push_back((r[i] + 1));
    X.push_back((r[i] - 1));
    Y.push_back(t[i]);
    Y.push_back((t[i] + 1));
    Y.push_back((t[i] - 1));
    Y.push_back(b[i]);
    Y.push_back((b[i] + 1));
    Y.push_back((b[i] - 1));
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    l[i] = (lower_bound(X.begin(), X.end(), l[i]) - X.begin());
    r[i] = (lower_bound(X.begin(), X.end(), r[i]) - X.begin());
    t[i] = (lower_bound(Y.begin(), Y.end(), t[i]) - Y.begin());
    b[i] = (lower_bound(Y.begin(), Y.end(), b[i]) - Y.begin());
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      E.push_back(event(0, r[i], -1, t[i], -1));
      E.push_back(event(1, l[i], r[i], b[i], -1));
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      E.push_back(event(0, l[i], -1, b[i], -1));
      E.push_back(event(1, l[i], r[i], t[i], -1));
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      E.push_back(event(0, l[i], -1, t[i], -1));
      E.push_back(event(1, r[i], -1, t[i], b[i]));
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      E.push_back(event(0, r[i], -1, b[i], -1));
      E.push_back(event(1, l[i], -1, t[i], b[i]));
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var q1 = P1[i];
    var j = (upper_bound(P2.begin(), P2.end(), q1, cmp_point_2) - P2.begin());
    if (((j < P2.size()) && (P2[j].y == q1.y)))
    {
      G[i].push_back((m1 + j));
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var q2 = P2[i];
    var j = (upper_bound(P3.begin(), P3.end(), q2, cmp_point_3) - P3.begin());
    if (((j < P3.size()) && (P3[j].x == q2.x)))
    {
      G[(m1 + i)].push_back(((m1 + m2) + j));
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var q3 = P3[i];
    var j = (upper_bound(P4.begin(), P4.end(), q3, cmp_point_4) - P4.begin());
    if (((j < P4.size()) && (P4[j].y == q3.y)))
    {
      G[((m1 + m2) + i)].push_back((((m1 + m2) + m3) + j));
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var q4 = P4[i];
    var j = (upper_bound(P1.begin(), P1.end(), q4, cmp_point_1) - P1.begin());
    if (((j < P1.size()) && (P1[j].x == q4.x)))
    {
      G[(((m1 + m2) + m3) + i)].push_back(j);
    }
  }
