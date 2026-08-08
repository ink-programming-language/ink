// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var V_MAX = 109;

var E_MAX = 1000;

class graph
{
  var n: dynamic;
  var m: dynamic;
  var head: dynamic = cpp_array(V_MAX);
  var next: dynamic = cpp_array((2 * E_MAX));
  var to: dynamic = cpp_array((2 * E_MAX));
  var capa: dynamic = cpp_array((2 * E_MAX));
  var flow: dynamic = cpp_array((2 * E_MAX));
  func init(N: dynamic)
  {
      n = N;
      m = 0;
      rep(u, n)[u] = -1;
    }
  func add_directed_edge(u: dynamic, v: dynamic, ca: dynamic)
  {
      next[m] = head[u];
      head[u] = m;
      to[m] = v;
      capa[m] = ca;
      flow[m] = 0;
      m += 1;
      next[m] = head[v];
      head[v] = m;
      to[m] = u;
      capa[m] = 0;
      flow[m] = 0;
      m += 1;
    }
  func add_undirected_edge(u: dynamic, v: dynamic, ca: dynamic)
  {
      next[m] = head[u];
      head[u] = m;
      to[m] = v;
      capa[m] = ca;
      flow[m] = 0;
      m += 1;
      next[m] = head[v];
      head[v] = m;
      to[m] = u;
      capa[m] = ca;
      flow[m] = 0;
      m += 1;
    }
}

var INF = (1 << 61);

var layer = cpp_array(V_MAX);

var now = cpp_array(V_MAX);

func make_layer(G: dynamic, s: dynamic, t: dynamic)
{
  var n = G.n;
  rep(u, n)[u] = (if ((u == s)) 0 else -1);
  var head = 0;
  var tail = 0;
  var Q = cpp_array(V_MAX);
  Q[cpp_update(tail, "++")] = s;
  while (((head < tail) && (layer[t] == -1)))
  {
    var u = Q[cpp_update(head, "++")];
    {
      var e = G.head[u];
      while ((e != -1))
      {
        var v = G.to[e];
        var capa = G.capa[e];
        var flow = G.flow[e];
        if ((((capa - flow) > 0) && (layer[v] == -1)))
        {
          layer[v] = (layer[u] + 1);
          Q[cpp_update(tail, "++")] = v;
        }
        e = G.next[e];
      }
    }
  }
  return (layer[t] != -1);
}

func augment(G: dynamic, u: dynamic, t: dynamic, water: dynamic)
{
  if ((u == t))
  {
    return water;
  }
  {
    var e = now[u];
    while ((e != -1))
    {
      var v = G.to[e];
      var capa = G.capa[e];
      var flow = G.flow[e];
      if ((((capa - flow) > 0) && (layer[v] > layer[u])))
      {
        var w = augment(G, v, t, min(water, (capa - flow)));
        if ((w > 0))
        {
          G.flow[e] += w;
          G.flow[(e ^ 1)] -= w;
          return w;
        }
      }
      e = G.next[e];
    }
  }
  return 0;
}

func Dinic(G: dynamic, s: dynamic, t: dynamic)
{
  var n = G.n;
  var ans = 0;
  while (make_layer(G, s, t))
  {
    rep(u, n)[u] = G.head[u];
    {
      var water = 1;
      while ((water > 0))
      {
        water = augment(G, s, t, INF);
        ans += water;
      }
    }
  }
  return ans;
}

func main()
{
  var n: dynamic;
  {
    var W: dynamic;
    while (cpp_comma(scanf("%d%lld", (&n), (&W)), n))
    {
      var aki = [];
      var need = cpp_array(100);
      var s = (n + 7);
      var t = (s + 1);
      var G: dynamic;
      G.init((n + 9));
      rep(u, 7).add_directed_edge(s, u, W);
      rep(i, n);
      rep(j, 7);
      if (aki[i][j])
      {
        var u = j;
        var v = (7 + i);
        G.add_directed_edge(u, v, W);
      }
      puts(if ((Dinic(G, s, t) == accumulate(need, (need + n), 0))) "Yes" else "No");
    }
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
          var s = cpp_array(16);
          scanf("%s", s);
          if (((s[0] == cpp_char("S")) && (s[1] == cpp_char("u"))))
          {
            aki[i][0] = true;
          }
          if ((s[0] == cpp_char("M")))
          {
            aki[i][1] = true;
          }
          if (((s[0] == cpp_char("T")) && (s[1] == cpp_char("u"))))
          {
            aki[i][2] = true;
          }
          if ((s[0] == cpp_char("W")))
          {
            aki[i][3] = true;
          }
          if (((s[0] == cpp_char("T")) && (s[1] == cpp_char("h"))))
          {
            aki[i][4] = true;
          }
          if ((s[0] == cpp_char("F")))
          {
            aki[i][5] = true;
          }
          if (((s[0] == cpp_char("S")) && (s[1] == cpp_char("a"))))
          {
            aki[i][6] = true;
          }
        }

func rep(argument_0: dynamic, argument_1: dynamic)
{
        var m: dynamic;
        scanf("%lld%d", (need + i), (&m));
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
        var v = (7 + i);
        G.add_directed_edge(v, t, need[i]);
      }
