// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var V_MAX: dynamic = 109;

var E_MAX: dynamic = 1000;

class graph
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var head: dynamic = cpp_array(V_MAX);
  var next: dynamic = cpp_array((2 * E_MAX));
  var to: dynamic = cpp_array((2 * E_MAX));
  var capa: dynamic = cpp_array((2 * E_MAX));
  var flow: dynamic = cpp_array((2 * E_MAX));
  func init(N: dynamic) -> dynamic
  {
      n = N;
      m = 0;
      rep(u, n)[u] = -1;
    }
  func add_directed_edge(u: dynamic, v: dynamic, ca: dynamic) -> dynamic
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
  func add_undirected_edge(u: dynamic, v: dynamic, ca: dynamic) -> dynamic
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

var INF: dynamic = (1 << 61);

var layer: dynamic = cpp_array(V_MAX);

var now: dynamic = cpp_array(V_MAX);

func make_layer(G: dynamic, s: dynamic, t: dynamic) -> dynamic
{
  var n: dynamic = G.n;
  rep(u, n)[u] = ( ((u == s)) ? 0 : -1);
  var head: dynamic = 0;
  var tail: dynamic = 0;
  var Q: dynamic = cpp_array(V_MAX);
  Q[cpp_update(tail, "++")] = s;
  while (((head < tail) && (layer[t] == -1)))
  {
    var u: dynamic = Q[cpp_update(head, "++")];
    {
      var e: dynamic = G.head[u];
      while ((e != -1))
      {
        var v: dynamic = G.to[e];
        var capa: dynamic = G.capa[e];
        var flow: dynamic = G.flow[e];
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

func augment(G: dynamic, u: dynamic, t: dynamic, water: dynamic) -> dynamic
{
  if ((u == t))
  {
    return water;
  }
  {
    var e: dynamic = now[u];
    while ((e != -1))
    {
      var v: dynamic = G.to[e];
      var capa: dynamic = G.capa[e];
      var flow: dynamic = G.flow[e];
      if ((((capa - flow) > 0) && (layer[v] > layer[u])))
      {
        var w: dynamic = augment(G, v, t, min(water, (capa - flow)));
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

func Dinic(G: dynamic, s: dynamic, t: dynamic) -> dynamic
{
  var n: dynamic = G.n;
  var ans: dynamic = 0;
  while (make_layer(G, s, t))
  {
    rep(u, n)[u] = G.head[u];
    {
      var water: dynamic = 1;
      while ((water > 0))
      {
        water = augment(G, s, t, INF);
        ans += water;
      }
    }
  }
  return ans;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  {
    var W: dynamic = cpp_uninitialized();
    while (cpp_comma(scanf("%d%lld", (&n), (&W)), n))
    {
      var aki: dynamic = [];
      var need: dynamic = cpp_array(100);
      var s: dynamic = (n + 7);
      var t: dynamic = (s + 1);
      var G: dynamic = cpp_uninitialized();
      G.init((n + 9));
      rep(u, 7).add_directed_edge(s, u, W);
      rep(i, n);
      rep(j, 7);
      if (aki[i][j])
      {
        var u: dynamic = j;
        var v: dynamic = (7 + i);
        G.add_directed_edge(u, v, W);
      }
      puts( ((Dinic(G, s, t) == accumulate(need, (need + n), 0))) ? "Yes" : "No");
    }
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
          var s: dynamic = cpp_array(16);
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var m: dynamic = cpp_uninitialized();
        scanf("%lld%d", (need + i), (&m));
      }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var v: dynamic = (7 + i);
        G.add_directed_edge(v, t, need[i]);
      }
