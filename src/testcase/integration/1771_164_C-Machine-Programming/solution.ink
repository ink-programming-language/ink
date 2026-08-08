// Translated from solution.cpp.

var que: dynamic;

var size: dynamic;

var n: dynamic;

var k: dynamic;

var S: dynamic;

var T: dynamic;

var dis = cpp_array(2010000);

var i: dynamic;

var j: dynamic;

var g = cpp_array(2010000);

var p = cpp_array(2010000);

var flow = cpp_array(2010000);

var num = cpp_array(2010000);

var tot: dynamic;

var ex = cpp_array(2010000);

var cnt: dynamic;

var vis = cpp_array(2010000);

var h = cpp_array(2010000);

class node
{
  var to: dynamic;
  var next: dynamic;
  var f: dynamic;
  var v: dynamic;
}

var e = cpp_array(2010000);

func add1(o: dynamic, p: dynamic, q: dynamic, w: dynamic)
{
  e[cpp_update(size, "++")].to = p;
  e[size].next = g[o];
  g[o] = size;
  e[size].f = q;
  e[size].v = w;
}

func add(o: dynamic, p: dynamic, q: dynamic, w: dynamic)
{
  add1(o, p, q, w);
  add1(p, o, 0, (-w));
}

class node1
{
  var s: dynamic;
  var t: dynamic;
  var val: dynamic;
}

var a = cpp_array(2010000);

func check(i: dynamic, j: dynamic)
{
  var tmp = ((a[i].s + a[i].t) - 1);
  return (tmp < a[j].s);
}

func init()
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

func mcmf()
{
  {
    i = 1;
    while ((i <= T))
    {
      {
        var x = S;
        while ((x <= T))
        {
          {
            var k = g[x];
            while (k)
            {
              if ((e[k].f == 0))
              {
                k = e[k].next;
                continue;
              }
              var y = e[k].to;
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
      var x = que.top().second;
      que.pop();
      if ((vis[x] == 1))
      {
        continue;
      }
      vis[x] = 1;
      {
        var k = g[x];
        while (k)
        {
          var y = e[k].to;
          var cost = ((e[k].v + h[x]) - h[y]);
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
    var now = p[T];
    while (now)
    {
      e[now].f -= flow[T];
      e[(now ^ 1)].f += flow[T];
      now = p[e[(now ^ 1)].to];
    }
  }
}

func main()
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
