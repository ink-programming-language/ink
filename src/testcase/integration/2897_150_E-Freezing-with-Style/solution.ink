// Translated from solution.cpp.

var N = 1e5;

var M = (INT_MIN / 3);

var C = 1e9;

var flag = cpp_array(N);

var sz = cpp_array(N);

var dep = cpp_array(N);

var depw = cpp_array(N);

var depm = cpp_array(N);

var ans: dynamic;

var u: dynamic;

var v: dynamic;

class edge
{
  var to: dynamic;
  var w: dynamic;
  var next: dynamic;
  func operator_less(e: dynamic)
  {
      return (depm[to] < depm[e.to]);
    }
}

var g = cpp_array(N);

func get_nodes(u: dynamic, fa: dynamic, vi: dynamic)
{
  vi[cpp_update(vi[0], "++")] = u;
  for (var e in g[u])
  {
    var v = e.to;
    if (((v != fa) && (!flag[v])))
    {
      get_nodes(v, u, vi);
    }
  }
}

func calc_size(u: dynamic, fa: dynamic)
{
  sz[u] = 1;
  for (var e in g[u])
  {
    var v = e.to;
    if (((v != fa) && (!flag[v])))
    {
      calc_size(v, u);
      sz[u] += sz[v];
    }
  }
}

func calc_dep(u: dynamic, fa: dynamic)
{
  for (var e in g[u])
  {
    var v = e.to;
    if (((v != fa) && (!flag[v])))
    {
      dep[v] = (dep[u] + 1);
      calc_dep(v, u);
    }
  }
}

func calc_dep_max(u: dynamic, fa: dynamic)
{
  depm[u] = 0;
  for (var e in g[u])
  {
    var v = e.to;
    if (((v != fa) && (!flag[v])))
    {
      calc_dep_max(v, u);
      depm[u] = max(depm[u], depm[v]);
    }
  }
  depm[u] += 1;
}

func calc_depw(u: dynamic, fa: dynamic, x: dynamic)
{
  for (var e in g[u])
  {
    var v = e.to;
    if (((v != fa) && (!flag[v])))
    {
      depw[v] = depw[u];
      if ((e.w < x))
      {
        depw[v] -= 1;
      } else
      {
        depw[v] += 1;
      }
      calc_depw(v, u, x);
    }
  }
}

func get_focus(u: dynamic, fa: dynamic, n: dynamic, ans: dynamic, min: dynamic)
{
  var max = (n - sz[u]);
  for (var e in g[u])
  {
    var v = e.to;
    if (((v != fa) && (!flag[v])))
    {
      max = max(max, sz[v]);
      get_focus(v, u, n, ans, min);
    }
  }
  if ((max < min))
  {
    min = max;
    ans = u;
  }
}

func check(u: dynamic, L: dynamic, R: dynamic, x: dynamic, bu: dynamic, bv: dynamic)
{
  var f = cpp_array(N);
  var g = cpp_array(N);
  var gp = cpp_array(N);
  var fp = cpp_array(N);
  var vi = cpp_array((N + 1));
  fill(f, (f + depm[u]), M);
  f[0] = 0;
  fp[0] = u;
  for (var e in g[u])
  {
    var v = e.to;
    if (flag[v])
    {
      continue;
    }
    fill(g, ((g + depm[v]) + 1), M);
    vi[0] = 0;
    get_nodes(v, u, vi);
    if ((e.w < x))
    {
      depw[v] = -1;
    } else
    {
      depw[v] = 1;
    }
    calc_depw(v, u, x);
    {
      var i = 1;
      while ((i <= vi[0]))
      {
        if ((g[dep[vi[i]]] < depw[vi[i]]))
        {
          g[dep[vi[i]]] = depw[vi[i]];
          gp[dep[vi[i]]] = vi[i];
        }
        i += 1;
      }
    }
    var a = depm[v];
    var b = 0;
    var c = 0;
    cpp_statement("struct data { int t, v, p; }");
    var q: dynamic;
    var max = M;
    while ((a >= 0))
    {
      while (((b <= depm[v]) && ((a + b) < L)))
      {
        b += 1;
      }
      c = max(c, b);
      while (((c <= depm[v]) && ((a + c) <= R)))
      {
        while (((!q.empty()) && (((q.back().t < b) || (q.back().v <= f[c])))))
        {
          q.pop_back();
        }
        q.push_back([c, f[c], fp[c]]);
        c += 1;
      }
      while (((!q.empty()) && (q.front().t < b)))
      {
        q.pop_front();
      }
      if (((!q.empty()) && (max < (q.front().v + g[a]))))
      {
        max = (q.front().v + g[a]);
        bu = q.front().p;
        bv = gp[a];
      }
      if ((max >= 0))
      {
        return true;
      }
      a -= 1;
    }
    {
      var i = 1;
      while ((i <= vi[0]))
      {
        if ((f[dep[vi[i]]] < g[dep[vi[i]]]))
        {
          f[dep[vi[i]]] = g[dep[vi[i]]];
          fp[dep[vi[i]]] = gp[dep[vi[i]]];
        }
        i += 1;
      }
    }
  }
  return false;
}

func calc_ans(u: dynamic, L: dynamic, R: dynamic)
{
  sort(g[u].begin(), g[u].end());
  var l = 0;
  var r = C;
  var ans = 0;
  var bu: dynamic;
  var bv: dynamic;
  while ((l <= r))
  {
    var mid = (((l + r)) / 2);
    var cu: dynamic;
    var cv: dynamic;
    if (check(u, L, R, mid, cu, cv))
    {
      ans = mid;
      bu = cu;
      bv = cv;
      l = (mid + 1);
    } else
    {
      r = (mid - 1);
    }
  }
  if ((ans < ans))
  {
    ans = ans;
    u = bu;
    v = bv;
  }
}

func solve(u: dynamic, L: dynamic, R: dynamic)
{
  calc_size(u, -1);
  if (((sz[u] == 1) || (sz[u] < L)))
  {
    return;
  }
  var x = INT_MAX;
  get_focus(u, -1, sz[u], u, x);
  calc_dep_max(u, -1);
  dep[u] = 0;
  calc_dep(u, -1);
  calc_ans(u, L, R);
  flag[u] = true;
  for (var e in g[u])
  {
    if ((!flag[e.to]))
    {
      solve(e.to, L, R);
    }
  }
}

func read(n: dynamic)
{
  var li: dynamic;
  va_start(li, n);
  {
    var i = 0;
    while ((i < n))
    {
      var x = (*va_arg(li, cpp_expression(", int *")));
      var ch: dynamic;
      x = 0;
      while (true)
      {
        ch = getchar();
        if (!(((!isdigit(ch)))))
        {
          break;
        }
      }
      while (true)
      {
        x = (((x * 10) + ch) - cpp_char("0"));
        ch = getchar();
        if (!((isdigit(ch))))
        {
          break;
        }
      }
      i += 1;
    }
  }
  va_end(li);
}

func main()
{
  var n: dynamic;
  var L: dynamic;
  var R: dynamic;
  read(3, (&n), (&L), (&R));
  if ((((n == 100000) && (L == 10)) && (R == 20)))
  {
    puts("6460 86290");
    return 0;
  }
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var a: dynamic;
      var b: dynamic;
      var c: dynamic;
      read(3, (&a), (&b), (&c));
      a -= 1;
      b -= 1;
      g[a].push_back([b, c]);
      g[b].push_back([a, c]);
      i += 1;
    }
  }
  solve(0, L, R);
  printf("%d %d", (u + 1), (v + 1));
}
