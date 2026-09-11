// Translated from solution.cpp.

var que: dynamic = cpp_uninitialized();

var size: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var S: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var dis: dynamic = cpp_array(2010000);

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var g: dynamic = cpp_array(2010000);

var p: dynamic = cpp_array(2010000);

var flow: dynamic = cpp_array(2010000);

var num: dynamic = cpp_array(2010000);

var tot: dynamic = cpp_uninitialized();

var ex: dynamic = cpp_array(2010000);

var cnt: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(2010000);

var h: dynamic = cpp_array(2010000);

class node
{
  var to: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array(2010000);

func add1(o: dynamic, p: dynamic, q: dynamic, w: dynamic) -> dynamic
{
  e[cpp_update(size, "++")].to = p;
  e[size].next = g[o];
  g[o] = size;
  e[size].f = q;
  e[size].v = w;
}

func add(o: dynamic, p: dynamic, q: dynamic, w: dynamic) -> dynamic
{
  add1(o, p, q, w);
  add1(p, o, 0, (-w));
}

class node1
{
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var val: dynamic = cpp_uninitialized();
}

var a: dynamic = cpp_array(2010000);

func check(i: dynamic, j: dynamic) -> dynamic
{
  var tmp: dynamic = ((a[i].s + a[i].t) - 1);
  return (tmp < a[j].s);
}

func init() -> dynamic
{
  sort((ex + 1), ((ex + 1) + cnt));
  tot = ((unique((ex + 1), ((ex + 1) + cnt)) - ex) - 1);
  {
    i = 1;
    while ((i <= n))
    {
      a[i].s = (lower_bound((ex + 1), ((ex + 1) + tot), a[i].s) - ex);
      a[i].t = (lower_bound((ex + 1), ((ex + 1) + tot), a[i].t) - ex);
      i += 1;
    }
  }
}

func mcmf() -> dynamic
{
  {
    i = 1;
    while ((i <= T))
    {
      {
        var x: dynamic = S;
        while ((x <= T))
        {
          {
            var k: dynamic = g[x];
            while (k)
            {
              if ((e[k].f == 0))
              {
                k = e[k].next;
                continue;
              }
              var y: dynamic = e[k].to;
              if ((h[y] < (h[x] + e[k].v)))
              {
                h[y] = (h[x] + e[k].v);
              }
              k = e[k].next;
            }
          }
          x += 1;
        }
      }
      i += 1;
    }
  }
  while (1)
  {
    {
      i = S;
      while ((i <= T))
      {
        dis[i] = -2000000000;
        vis[i] = 0;
        i += 1;
      }
    }
    dis[S] = 0;
    que.push(make_pair(0, S));
    flow[S] = 2000000000;
    while ((!que.empty()))
    {
      var x: dynamic = que.top().second;
      que.pop();
      if ((vis[x] == 1))
      {
        continue;
      }
      vis[x] = 1;
      {
        var k: dynamic = g[x];
        while (k)
        {
          var y: dynamic = e[k].to;
          var cost: dynamic = ((e[k].v + h[x]) - h[y]);
          if ((e[k].f && (dis[y] < (dis[x] + cost))))
          {
            dis[y] = (dis[x] + cost);
            flow[y] = min(flow[x], e[k].f);
            p[y] = k;
            que.push(make_pair(dis[y], y));
          }
          k = e[k].next;
        }
      }
    }
    if ((vis[T] == 0))
    {
      break;
    }
    {
      i = S;
      while ((i <= T))
      {
        h[i] += dis[i];
        i += 1;
      }
    }
    var now: dynamic = p[T];
    while (now)
    {
      e[now].f -= flow[T];
      e[(now ^ 1)].f += flow[T];
      now = p[e[(now ^ 1)].to];
    }
  }
}

func main() -> dynamic
{
  scanf("%d %d", (&n), (&k));
  size = 1;
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d %d %d", (&a[i].s), (&a[i].t), (&a[i].val));
      a[i].t += a[i].s;
      ex[cpp_update(cnt, "++")] = a[i].s;
      ex[cpp_update(cnt, "++")] = a[i].t;
      i += 1;
    }
  }
  init();
  S = 0;
  T = (tot + 1);
  {
    i = 0;
    while ((i <= tot))
    {
      add(i, (i + 1), k, 0);
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      add(a[i].s, a[i].t, 1, a[i].val);
      num[i] = (size - 1);
      i += 1;
    }
  }
  mcmf();
  {
    i = 1;
    while ((i <= n))
    {
      printf("%d ", (1 - e[num[i]].f));
      i += 1;
    }
  }
}
