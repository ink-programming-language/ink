// Translated from solution.cpp.

var N = 200005;

class Doll
{
  var out: dynamic;
  var in_cpp: dynamic;
  func operator_less(b: dynamic)
  {
      return (in_cpp < b.in_cpp);
    }
}

var a = cpp_array(N);

class Edge
{
  var to: dynamic;
  var nxt: dynamic;
  var val: dynamic;
  func Edge(to: dynamic = 0, nxt: dynamic = 0, val: dynamic = 0)
  {
      this->to = cpp_construct(to);
      this->nxt = cpp_construct(nxt);
      this->val = cpp_construct(val);
    }
}

var edge = cpp_array((N << 1));

var head = cpp_array(N);

var tot: dynamic;

func add(u: dynamic, v: dynamic, val: dynamic)
{
  edge[cpp_update(tot, "++")] = [v, head[u], val];
  head[u] = tot;
}

var n: dynamic;

func Erfen(x: dynamic)
{
  var l = 1;
  var r = n;
  while ((l < r))
  {
    var mid = (((l + r)) >> 1);
    if ((a[mid].in_cpp >= x))
    {
      r = mid;
    } else
    {
      l = (mid + 1);
    }
  }
  if ((a[l].in_cpp >= x))
  {
    return l;
  } else
  {
    return -1;
  }
}

var mod = (1e9 + 7);

var dis = cpp_array(N);

var cnt = cpp_array(N);

var vis = cpp_array(N);

func topo()
{
  memset(dis, 0x7f7f7f, cpp_sizeof((dis)));
  memset(cnt, 0, cpp_sizeof((cnt)));
  memset(vis, 0, cpp_sizeof((vis)));
  cnt[0] = 1;
  dis[0] = 0;
  {
    var i = 0;
    while ((i <= n))
    {
      {
        var j = head[i];
        while ((j != -1))
        {
          var v = edge[j].to;
          if ((dis[v] > (dis[i] + edge[j].val)))
          {
            dis[v] = (dis[i] + edge[j].val);
            cnt[v] = cnt[i];
          } else if ((dis[v] == (dis[i] + edge[j].val)))
          {
            (cpp_assign(cnt[v], "+=", cnt[i])) %= mod;
          }
          j = edge[j].nxt;
        }
      }
      i += 1;
    }
  }
}

func main()
{
  memset(head, -1, cpp_sizeof((head)));
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d%d", (&a[i].out), (&a[i].in_cpp));
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1));
  {
    var i = 0;
    while ((i < n))
    {
      add(i, (i + 1), (a[(i + 1)].in_cpp - a[i].in_cpp));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var up = Erfen(a[i].out);
      if ((up == -1))
      {
        i += 1;
        continue;
      }
      add(i, up, (a[up].in_cpp - a[i].out));
      i += 1;
    }
  }
  topo();
  var ans = 0;
  var mindis = 1e15;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((a[i].out > a[n].in_cpp))
      {
        mindis = min(mindis, dis[i]);
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if (((a[i].out > a[n].in_cpp) && (mindis == dis[i])))
      {
        (cpp_assign(ans, "+=", cnt[i])) %= mod;
      }
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
