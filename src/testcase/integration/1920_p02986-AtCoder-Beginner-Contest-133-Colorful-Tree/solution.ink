// Translated from solution.cpp.

var LL: dynamic = dynamic;

func MEM(x: dynamic, y: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

func MOD(x: dynamic) -> dynamic
{
  return cpp_expression("#include<");
}

var mod: dynamic = cpp_expression("#include<b");

var pb: dynamic = cpp_expression("#include<");

var STREAM_FAST: dynamic = cpp_expression("#include<bits/stdc++.h> #de");

var maxn: dynamic = (1e5 + 7);

var head: dynamic = cpp_array(maxn);

class node
{
  var v: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array((maxn << 1));

var ans: dynamic = cpp_array(maxn);

class Node
{
  var id: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var op: dynamic = cpp_uninitialized();
  func Node(id: dynamic, c: dynamic, y: dynamic, op: dynamic) -> dynamic
  {
      self->id = cpp_construct(id);
      self->c = cpp_construct(c);
      self->y = cpp_construct(y);
      self->op = cpp_construct(op);
    }
  func Node() -> dynamic
  {
    }
}

var Q: dynamic = cpp_array(maxn);

var tot: dynamic = 1;

func add(u: dynamic, v: dynamic, c: dynamic, w: dynamic) -> dynamic
{
  e[cpp_update(tot, "++")] = [v, head[u], c, w];
  head[u] = tot;
}

var f: dynamic = cpp_array(20, maxn);

var dep: dynamic = cpp_array(maxn);

func dfs(u: dynamic, fa: dynamic) -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= 19))
    {
      f[u][i] = f[f[u][(i - 1)]][(i - 1)];
      i += 1;
    }
  }
  {
    var i: dynamic = head[u];
    while (i)
    {
      var v: dynamic = e[i].v;
      if ((v == fa))
      {
        i = e[i].next;
        continue;
      }
      dep[v] = (dep[u] + 1);
      f[v][0] = u;
      dfs(v, u);
      i = e[i].next;
    }
  }
}

func LCA(x: dynamic, y: dynamic) -> dynamic
{
  if ((dep[x] > dep[y]))
  {
    swap(x, y);
  }
  {
    var i: dynamic = 19;
    while ((i >= 0))
    {
      if (((dep[y] > dep[x]) && (dep[f[y][i]] >= dep[x])))
      {
        y = f[y][i];
      }
      i -= 1;
    }
  }
  {
    var i: dynamic = 19;
    while ((i >= 0))
    {
      if ((f[x][i] != f[y][i]))
      {
        x = f[x][i];
        y = f[y][i];
      }
      i -= 1;
    }
  }
  return  ((x == y)) ? x : f[x][0];
}

var cnt: dynamic = cpp_array(maxn);

var sum: dynamic = cpp_array(maxn);

var dis: dynamic = 0;

func DFS(u: dynamic, fa: dynamic) -> dynamic
{
  for (var qy: dynamic in Q[u])
  {
    ans[qy.id] += (qy.op * (((dis - sum[qy.c]) + (cnt[qy.c] * qy.y))));
  }
  {
    var i: dynamic = head[u];
    while (i)
    {
      var v: dynamic = e[i].v;
      if ((v == fa))
      {
        i = e[i].next;
        continue;
      }
      cnt[e[i].c] += 1;
      sum[e[i].c] += e[i].w;
      dis += e[i].w;
      DFS(v, u);
      cnt[e[i].c] -= 1;
      sum[e[i].c] -= e[i].w;
      dis -= e[i].w;
      i = e[i].next;
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&q));
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var c: dynamic = cpp_uninitialized();
      var d: dynamic = cpp_uninitialized();
      scanf("%d%d%d%d", (&u), (&v), (&c), (&d));
      add(u, v, c, d);
      add(v, u, c, d);
      i += 1;
    }
  }
  dep[1] = 1;
  dfs(1, -1);
  {
    var i: dynamic = 1;
    while ((i <= q))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%d%d%d%d", (&x), (&y), (&u), (&v));
      var lca: dynamic = LCA(u, v);
      Q[u].pb(Node(i, x, y, 1));
      Q[v].pb(Node(i, x, y, 1));
      Q[lca].pb(Node(i, x, y, -2));
      i += 1;
    }
  }
  DFS(1, -1);
  {
    var i: dynamic = 1;
    while ((i <= q))
    {
      printf("%lld\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
