// Translated from solution.cpp.

var MAX = (800000 + 10);

var INF = 1e30;

var EPS = 0.02;

class point
{
  var x: dynamic;
  var y: dynamic;
  func point(a: dynamic, b: dynamic)
  {
      x = a;
      y = b;
    }
  func point()
  {
    }
  func print()
  {
      printf("%lf %lf\n", x, y);
    }
}

func sqr(x: dynamic)
{
  return (x * x);
}

func dist(a: dynamic, b: dynamic)
{
  return sqrt((sqr((a.x - b.x)) + sqr((a.y - b.y))));
}

func chaji(s: dynamic, a: dynamic, b: dynamic)
{
  return ((((a.x - s.x)) * ((b.y - s.y))) - (((a.y - s.y)) * ((b.x - s.x))));
}

var n: dynamic;

var S: dynamic;

var T: dynamic;

var d = cpp_array(MAX);

var TOP = cpp_construct(0, INF);

var head = cpp_array((MAX * 4));

var t = cpp_array((MAX * 4));

var tot: dynamic;

var nxt = cpp_array((MAX * 4));

var c = cpp_array((MAX * 4));

func addedge(x: dynamic, y: dynamic, v: dynamic)
{
  t[cpp_update(tot, "++")] = y;
  nxt[tot] = head[x];
  head[x] = tot;
  c[tot] = v;
}

class line
{
  var s: dynamic;
  var t: dynamic;
  var type_cpp: dynamic;
  var number: dynamic;
  func line()
  {
    }
  func line(a: dynamic, b: dynamic)
  {
      s = a;
      t = b;
      type_cpp = 0;
    }
  func print()
  {
      d[s].print();
      d[t].print();
      printf("\n");
    }
}

var l = cpp_array(MAX);

func get(b: dynamic, c: dynamic, x: dynamic)
{
  var k = (((x - b.x)) / ((c.x - b.x)));
  return point((b.x + (((c.x - b.x)) * k)), (b.y + (((c.y - b.y)) * k)));
}

func operator_less(a: dynamic, b: dynamic)
{
  var l = max(d[a.s].x, d[b.s].x);
  var r = min(d[a.t].x, d[b.t].x);
  var x = (((l + r)) * 0.5);
  var s1 = get(d[a.s], d[a.t], x);
  var s2 = get(d[b.s], d[b.t], x);
  if ((s1.y != s2.y))
  {
    return (s1.y < s2.y);
  } else
  {
    return (a.number < b.number);
  }
}

var st: dynamic;

class accident
{
  var x: dynamic;
  var num: dynamic;
  var flag: dynamic;
  func accident(c: dynamic, a: dynamic, b: dynamic)
  {
      x = c;
      num = a;
      flag = b;
    }
  func accident()
  {
    }
}

var p = cpp_array(MAX);

func operator_less(a: dynamic, b: dynamic)
{
  return (a.x < b.x);
}

var num: dynamic;

func add(x: dynamic, A: dynamic, B: dynamic)
{
  var a = d[A.s];
  var h = d[A.t];
  var b = d[B.s];
  var c = d[B.t];
  var e = get(b, c, x);
  var f = get(a, h, x);
  addedge(A.s, B.s, ((dist(a, f) + dist(f, e)) + dist(e, b)));
  addedge(A.s, B.t, ((dist(a, f) + dist(f, e)) + dist(e, c)));
  addedge(A.t, B.s, ((dist(h, f) + dist(f, e)) + dist(e, b)));
  addedge(A.t, B.t, ((dist(h, f) + dist(f, e)) + dist(e, c)));
}

func same(x: dynamic, A: dynamic, B: dynamic)
{
  var a = d[A.s];
  var h = d[A.t];
  var b = d[B.s];
  var c = d[B.t];
  var e = get(b, c, x);
  var f = get(a, h, x);
  return (fabs((e.y - f.y)) < 0.0000000001);
}

var q: dynamic;

var hh = cpp_array(MAX);

var dis = cpp_array(MAX);

func SPFA()
{
  var i: dynamic;
  var u: dynamic;
  var v: dynamic;
  {
    i = 1;
    while ((i <= n))
    {
      dis[i] = INF;
      i += 1;
    }
  }
  q.push(S);
  dis[S] = 0;
  hh[S] = 1;
  while ((!q.empty()))
  {
    u = q.front();
    q.pop();
    hh[u] = 0;
    {
      i = head[u];
      while (i)
      {
        v = t[i];
        if ((dis[v] > (dis[u] + c[i])))
        {
          dis[v] = (dis[u] + c[i]);
          if ((!hh[v]))
          {
            hh[v] = 1;
            q.push(v);
          }
        }
        i = nxt[i];
      }
    }
  }
  return dis[T];
}

func update(now: dynamic)
{
  var ii: dynamic;
  var it = st.find(l[p[now].num]);
  if ((it == st.end()))
  {
    return;
  }
  if ((it->type_cpp == -1))
  {
    ii = it;
    ii += 1;
    while ((same(p[now].x, (*ii), (*it)) && (ii->type_cpp == it->type_cpp)))
    {
      ii += 1;
      if ((ii == st.end()))
      {
        return;
      }
    }
    if ((ii == st.end()))
    {
      return;
    }
  }
  if ((it->type_cpp == 1))
  {
    if ((it == st.begin()))
    {
      return;
    }
    ii = it;
    ii -= 1;
    while ((same(p[now].x, (*ii), (*it)) && (ii->type_cpp == it->type_cpp)))
    {
      if ((ii == st.begin()))
      {
        return;
      }
      ii -= 1;
    }
    swap(ii, it);
  }
  if ((it->type_cpp == ii->type_cpp))
  {
    return;
  }
  add(p[now].x, (*ii), (*it));
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var now: dynamic;
  scanf("%d", (&n));
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%lf%lf", (&d[i].x), (&d[i].y));
      i += 1;
    }
  }
  scanf("%d%d", (&S), (&T));
  {
    i = 1;
    while ((i <= n))
    {
      j = (if ((i == n)) 1 else (i + 1));
      addedge(i, j, dist(d[i], d[j]));
      addedge(j, i, dist(d[j], d[i]));
      l[i] = line(i, j);
      l[i].number = i;
      if ((d[i].x != d[j].x))
      {
        if ((chaji(d[i], d[j], TOP) >= 0))
        {
          l[i].type_cpp = -1;
        } else
        {
          l[i].type_cpp = 1;
        }
        if ((d[l[i].s].x > d[l[i].t].x))
        {
          swap(l[i].s, l[i].t);
        }
        p[cpp_update(num, "++")] = accident(cpp_cast(d[l[i].s].x), i, 1);
        p[cpp_update(num, "++")] = accident(cpp_cast(d[l[i].t].x), i, -1);
      }
      i += 1;
    }
  }
  var it: dynamic;
  sort((p + 1), ((p + num) + 1));
  {
    now = 1;
    while ((now <= num))
    {
      j = now;
      while (((j <= num) && (p[j].x == p[now].x)))
      {
        if ((p[j].flag == 1))
        {
          st.insert(l[p[j].num]);
        }
        j += 1;
      }
      j = now;
      while (((j <= num) && (p[j].x == p[now].x)))
      {
        update(j);
        j += 1;
      }
      j = now;
      while (((j <= num) && (p[j].x == p[now].x)))
      {
        if ((p[j].flag == -1))
        {
          st.erase(l[p[j].num]);
        }
        j += 1;
      }
      now = j;
    }
  }
  var ans = SPFA();
  printf("%lf\n", ans);
  return 0;
}
