// Translated from solution.cpp.

var MAX: dynamic = (800000 + 10);

var INF: dynamic = 1e30;

var EPS: dynamic = 0.02;

class point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func point(a: dynamic, b: dynamic) -> dynamic
  {
      x = a;
      y = b;
    }
  func point() -> dynamic
  {
    }
  func print() -> dynamic
  {
      printf("%lf %lf\n", x, y);
    }
}

func sqr(x: dynamic) -> dynamic
{
  return (x * x);
}

func dist(a: dynamic, b: dynamic) -> dynamic
{
  return sqrt((sqr((a.x - b.x)) + sqr((a.y - b.y))));
}

func chaji(s: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  return ((((a.x - s.x)) * ((b.y - s.y))) - (((a.y - s.y)) * ((b.x - s.x))));
}

var n: dynamic = cpp_uninitialized();

var S: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var d: dynamic = cpp_array(MAX);

var TOP: dynamic = cpp_construct(0, INF);

var head: dynamic = cpp_array((MAX * 4));

var t: dynamic = cpp_array((MAX * 4));

var tot: dynamic = cpp_uninitialized();

var nxt: dynamic = cpp_array((MAX * 4));

var c: dynamic = cpp_array((MAX * 4));

func addedge(x: dynamic, y: dynamic, v: dynamic) -> dynamic
{
  t[cpp_update(tot, "++")] = y;
  nxt[tot] = head[x];
  head[x] = tot;
  c[tot] = v;
}

class line
{
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var type_cpp: dynamic = cpp_uninitialized();
  var number: dynamic = cpp_uninitialized();
  func line() -> dynamic
  {
    }
  func line(a: dynamic, b: dynamic) -> dynamic
  {
      s = a;
      t = b;
      type_cpp = 0;
    }
  func print() -> dynamic
  {
      d[s].print();
      d[t].print();
      printf("\n");
    }
}

var l: dynamic = cpp_array(MAX);

func get(b: dynamic, c: dynamic, x: dynamic) -> dynamic
{
  var k: dynamic = (((x - b.x)) / ((c.x - b.x)));
  return point((b.x + (((c.x - b.x)) * k)), (b.y + (((c.y - b.y)) * k)));
}

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  var l: dynamic = max(d[a.s].x, d[b.s].x);
  var r: dynamic = min(d[a.t].x, d[b.t].x);
  var x: dynamic = (((l + r)) * 0.5);
  var s1: dynamic = get(d[a.s], d[a.t], x);
  var s2: dynamic = get(d[b.s], d[b.t], x);
  if ((s1.y != s2.y))
  {
    return (s1.y < s2.y);
  } else
  {
    return (a.number < b.number);
  }
}

var st: dynamic = cpp_uninitialized();

class accident
{
  var x: dynamic = cpp_uninitialized();
  var num: dynamic = cpp_uninitialized();
  var flag: dynamic = cpp_uninitialized();
  func accident(c: dynamic, a: dynamic, b: dynamic) -> dynamic
  {
      x = c;
      num = a;
      flag = b;
    }
  func accident() -> dynamic
  {
    }
}

var p: dynamic = cpp_array(MAX);

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  return (a.x < b.x);
}

var num: dynamic = cpp_uninitialized();

func add(x: dynamic, A: dynamic, B: dynamic) -> dynamic
{
  var a: dynamic = d[A.s];
  var h: dynamic = d[A.t];
  var b: dynamic = d[B.s];
  var c: dynamic = d[B.t];
  var e: dynamic = get(b, c, x);
  var f: dynamic = get(a, h, x);
  addedge(A.s, B.s, ((dist(a, f) + dist(f, e)) + dist(e, b)));
  addedge(A.s, B.t, ((dist(a, f) + dist(f, e)) + dist(e, c)));
  addedge(A.t, B.s, ((dist(h, f) + dist(f, e)) + dist(e, b)));
  addedge(A.t, B.t, ((dist(h, f) + dist(f, e)) + dist(e, c)));
}

func same(x: dynamic, A: dynamic, B: dynamic) -> dynamic
{
  var a: dynamic = d[A.s];
  var h: dynamic = d[A.t];
  var b: dynamic = d[B.s];
  var c: dynamic = d[B.t];
  var e: dynamic = get(b, c, x);
  var f: dynamic = get(a, h, x);
  return (fabs((e.y - f.y)) < 0.0000000001);
}

var q: dynamic = cpp_uninitialized();

var hh: dynamic = cpp_array(MAX);

var dis: dynamic = cpp_array(MAX);

func SPFA() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
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

func update(now: dynamic) -> dynamic
{
  var ii: dynamic = cpp_uninitialized();
  var it: dynamic = st.find(l[p[now].num]);
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

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var now: dynamic = cpp_uninitialized();
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
      j = ( ((i == n)) ? 1 : (i + 1));
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
  var it: dynamic = cpp_uninitialized();
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
  var ans: dynamic = SPFA();
  printf("%lf\n", ans);
  return 0;
}
