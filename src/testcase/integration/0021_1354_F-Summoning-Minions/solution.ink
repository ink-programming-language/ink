// Translated from solution.cpp.

class Edge
{
  var to: dynamic = cpp_uninitialized();
  var dis: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
}

var edge: dynamic = cpp_array(24050);

var num: dynamic = -1;

var vis: dynamic = cpp_array(10010);

var mincost: dynamic = cpp_uninitialized();

var pre: dynamic = cpp_array(10010);

var head: dynamic = cpp_array(10010);

var cost: dynamic = cpp_array(10010);

var last: dynamic = cpp_array(10010);

var flow: dynamic = cpp_array(10010);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(110);

var b: dynamic = cpp_array(110);

var s: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var maxflow: dynamic = cpp_uninitialized();

var to: dynamic = cpp_array(110);

func add(f: dynamic, t: dynamic, dis: dynamic, cost: dynamic) -> dynamic
{
  edge[cpp_update(num, "++")].to = t;
  edge[num].dis = dis;
  edge[num].next = head[f];
  edge[num].cost = cost;
  head[f] = num;
  edge[cpp_update(num, "++")].to = f;
  edge[num].dis = 0;
  edge[num].cost = (-cost);
  edge[num].next = head[t];
  head[t] = num;
}

var q: dynamic = cpp_uninitialized();

func spfa(s: dynamic, t: dynamic) -> dynamic
{
  memset(cost, 0x3f3f3f3f, cpp_sizeof(cost));
  memset(flow, 0x3f3f3f3f, cpp_sizeof(flow));
  memset(vis, 0, cpp_sizeof(vis));
  q.push(s);
  vis[s] = 1;
  cost[s] = 0;
  pre[t] = -1;
  while ((!q.empty()))
  {
    var nowp: dynamic = q.front();
    q.pop();
    vis[nowp] = 0;
    {
      var i: dynamic = head[nowp];
      while ((i != -1))
      {
        if (((edge[i].dis > 0) && (cost[edge[i].to] > (cost[nowp] + edge[i].cost))))
        {
          cost[edge[i].to] = (cost[nowp] + edge[i].cost);
          pre[edge[i].to] = nowp;
          last[edge[i].to] = i;
          flow[edge[i].to] = min(flow[nowp], edge[i].dis);
          if ((!vis[edge[i].to]))
          {
            vis[edge[i].to] = 1;
            q.push(edge[i].to);
          }
        }
        i = edge[i].next;
      }
    }
  }
  return (pre[t] != -1);
}

func MCMF(s: dynamic, t: dynamic) -> dynamic
{
  while (spfa(s, t))
  {
    var now: dynamic = t;
    maxflow += flow[t];
    mincost += (flow[t] * cost[t]);
    while ((now != s))
    {
      edge[last[now]].dis -= flow[t];
      edge[(last[now] ^ 1)].dis += flow[t];
      now = pre[now];
    }
  }
}

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
  read(T);
  while (cpp_update(T, "--"))
  {
    num = -1;
    memset(to, -1, cpp_sizeof(to));
    memset(head, -1, cpp_sizeof(head));
    read(n, k);
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        read(a[i], b[i]);
        i += 1;
      }
    }
    maxflow = cpp_assign(mincost, "=", 0);
    s = 0;
    t = ((2 * n) + 1);
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        add(s, i, 1, 0);
        add((i + n), t, 1, 0);
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        {
          var j: dynamic = 1;
          while ((j <= n))
          {
            var nc: dynamic = 0;
            if ((j <= (k - 1)))
            {
              nc = (a[i] + (b[i] * ((j - 1))));
            } else if ((j != n))
            {
              nc = (b[i] * ((k - 1)));
            } else
            {
              nc = (a[i] + (b[i] * ((k - 1))));
            }
            nc = (0x3f3f3f3f - nc);
            add(i, (j + n), 1, nc);
            j += 1;
          }
        }
        i += 1;
      }
    }
    MCMF(s, t);
    var nowi: dynamic = -1;
    {
      var i: dynamic = (n * 4);
      while ((i <= num))
      {
        nowi += 1;
        if ((edge[i].dis == 0))
        {
          to[(1 + (nowi % n))] = (1 + (nowi / n));
        }
        i += 2;
      }
    }
    write(((2 * n) - k), "\n");
    {
      var i: dynamic = 1;
      while ((i <= (k - 1)))
      {
        write(to[i], " ");
        i += 1;
      }
    }
    {
      var i: dynamic = k;
      while ((i < n))
      {
        write(to[i], " ", (-to[i]), " ");
        i += 1;
      }
    }
    write(to[n]);
    write("\n");
  }
  return 0;
}
