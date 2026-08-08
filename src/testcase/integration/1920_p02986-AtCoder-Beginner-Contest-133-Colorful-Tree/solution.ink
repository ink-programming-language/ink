// Translated from solution.cpp.

var LL = dynamic;

func MEM(x: dynamic, y: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

func MOD(x: dynamic)
{
  return cpp_expression("#include<");
}

var mod = cpp_expression("#include<b");

var pb = cpp_expression("#include<");

var STREAM_FAST = cpp_expression("#include<bits/stdc++.h> #de");

var maxn = (1e5 + 7);

var head = cpp_array(maxn);

class node
{
  var v: dynamic;
  var next: dynamic;
  var c: dynamic;
  var w: dynamic;
}

var e = cpp_array((maxn << 1));

var ans = cpp_array(maxn);

class Node
{
  var id: dynamic;
  var c: dynamic;
  var y: dynamic;
  var op: dynamic;
  func Node(id: dynamic, c: dynamic, y: dynamic, op: dynamic)
  {
      this->id = cpp_construct(id);
      this->c = cpp_construct(c);
      this->y = cpp_construct(y);
      this->op = cpp_construct(op);
    }
  func Node()
  {
    }
}

var Q = cpp_array(maxn);

var tot = 1;

func add(u: dynamic, v: dynamic, c: dynamic, w: dynamic)
{
  e[cpp_update(tot, "++")] = [v, head[u], c, w];
  head[u] = tot;
}

var f = cpp_array(20, maxn);

var dep = cpp_array(maxn);

func dfs(u: dynamic, fa: dynamic)
{
  {
    var i = 1;
    while ((i <= 19))
    {
      f[u][i] = f[f[u][(i - 1)]][(i - 1)];
      i += 1;
    }
  }
  {
    var i = head[u];
    while (i)
    {
      var v = e[i].v;
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

func LCA(x: dynamic, y: dynamic)
{
  if ((dep[x] > dep[y]))
  {
    swap(x, y);
  }
  {
    var i = 19;
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
    var i = 19;
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
  return if ((x == y)) x else f[x][0];
}

var cnt = cpp_array(maxn);

var sum = cpp_array(maxn);

var dis = 0;

func DFS(u: dynamic, fa: dynamic)
{
  for (var qy in Q[u])
  {
    ans[qy.id] += (qy.op * (((dis - sum[qy.c]) + (cnt[qy.c] * qy.y))));
  }
  {
    var i = head[u];
    while (i)
    {
      var v = e[i].v;
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

func main()
{
  var n: dynamic;
  var q: dynamic;
  scanf("%d%d", (&n), (&q));
  {
    var i = 1;
    while ((i < n))
    {
      var u: dynamic;
      var v: dynamic;
      var c: dynamic;
      var d: dynamic;
      scanf("%d%d%d%d", (&u), (&v), (&c), (&d));
      add(u, v, c, d);
      add(v, u, c, d);
      i += 1;
    }
  }
  dep[1] = 1;
  dfs(1, -1);
  {
    var i = 1;
    while ((i <= q))
    {
      var u: dynamic;
      var v: dynamic;
      var x: dynamic;
      var y: dynamic;
      scanf("%d%d%d%d", (&x), (&y), (&u), (&v));
      var lca = LCA(u, v);
      Q[u].pb(Node(i, x, y, 1));
      Q[v].pb(Node(i, x, y, 1));
      Q[lca].pb(Node(i, x, y, -2));
      i += 1;
    }
  }
  DFS(1, -1);
  {
    var i = 1;
    while ((i <= q))
    {
      printf("%lld\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
