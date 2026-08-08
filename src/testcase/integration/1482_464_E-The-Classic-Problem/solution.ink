// Translated from solution.cpp.

var buf = cpp_array((1 << 21));

var p1 = buf;

var p2 = buf;

func cmax(a: dynamic, b: dynamic)
{
  return if ((a < b)) cpp_comma(cpp_assign(a, "=", b), 1) else 0;
}

func read()
{
  var ch: dynamic;
  var flag = 0;
  var res: dynamic;
  while ((!isdigit(cpp_assign(ch, "=", if ((cpp_comma(((p1 == p2) && (cpp_assign(p2, "=", ((cpp_assign(p1, "=", buf)) + fread(buf, 1, (1 << 21), stdin))))), (p1 == p2)))) EOF else (*cpp_update(p1, "++"))))))
  {
    (((ch == cpp_char("-"))) && (cpp_assign(flag, "=", true)));
  }
  {
    res = (ch - cpp_char("0"));
    while (isdigit(cpp_assign(ch, "=", if ((cpp_comma(((p1 == p2) && (cpp_assign(p2, "=", ((cpp_assign(p1, "=", buf)) + fread(buf, 1, (1 << 21), stdin))))), (p1 == p2)))) EOF else (*cpp_update(p1, "++")))))
    {
      res = (((res * 10) + ch) - cpp_char("0"));
    }
  }
  ((flag) && (cpp_assign(res, "=", (-res))));
  return res;
}

var N = (1e5 + 5);

var mod = (1e9 + 7);

var n: dynamic;

var m: dynamic;

var head = cpp_array(N);

var Next = cpp_array((N << 1));

var ver = cpp_array((N << 1));

var edge = cpp_array((N << 1));

var S: dynamic;

var T: dynamic;

var lim: dynamic;

var b = cpp_array((N << 1));

var rt = cpp_array(N);

var Pre = cpp_array(N);

var tot: dynamic;

var cnt: dynamic;

var L = cpp_array((N * 120));

var R = cpp_array((N * 120));

var sum = cpp_array((N * 120));

func add(u: dynamic, v: dynamic, e: dynamic)
{
  ver[cpp_update(tot, "++")] = v;
  Next[tot] = head[u];
  head[u] = tot;
  edge[tot] = e;
  ver[cpp_update(tot, "++")] = u;
  Next[tot] = head[v];
  head[v] = tot;
  edge[tot] = e;
}

func cmp(u: dynamic, v: dynamic, l: dynamic, r: dynamic)
{
  if ((l == r))
  {
    return (sum[u] > sum[v]);
  }
  var mid = (((l + r)) >> 1);
  if ((sum[R[u]] == sum[R[v]]))
  {
    return cmp(L[u], L[v], l, mid);
  } else
  {
    return cmp(R[u], R[v], (mid + 1), r);
  }
}

func update(last: dynamic, now: dynamic, l: dynamic, r: dynamic, k: dynamic)
{
  L[cpp_assign(now, "=", cpp_update(cnt, "++"))] = L[last];
  R[now] = R[last];
  if ((l == r))
  {
    sum[now] = (sum[last] ^ 1);
    return sum[last];
  }
  var mid = (((l + r)) >> 1);
  var res: dynamic;
  if ((k > mid))
  {
    res = update(R[last], R[now], (mid + 1), r, k);
  } else
  {
    res = update(L[last], L[now], l, mid, k);
    if (res)
    {
      res = update(R[last], R[now], (mid + 1), r, k);
    }
  }
  sum[now] = (((((1 * sum[R[now]]) * b[((mid - l) + 1)]) + sum[L[now]])) % mod);
  return res;
}

class node
{
  var x: dynamic;
  var rt: dynamic;
  func operator_less(b: dynamic)
  {
      return cmp(rt, b.rt, 0, lim);
    }
}

var q: dynamic;

func dfs(u: dynamic, dep: dynamic)
{
  if ((u == S))
  {
    printf("%d\n%d ", dep, u);
    return;
  }
  dfs(Pre[u], (dep + 1));
  printf("%d ", u);
}

func print(u: dynamic)
{
  printf("%d\n", sum[rt[u]]);
  dfs(u, 1);
  exit(0);
}

func main()
{
  n = read();
  m = read();
  {
    var i = 1;
    while ((i <= m))
    {
      var u: dynamic;
      var v: dynamic;
      var e: dynamic;
      u = read();
      v = read();
      e = read();
      add(u, v, e);
      cmax(lim, e);
      i += 1;
    }
  }
  lim += 18;
  b[0] = 1;
  {
    var i = 1;
    while ((i <= lim))
    {
      b[i] = ((((1 * b[(i - 1)]) << 1)) % mod);
      i += 1;
    }
  }
  S = read();
  T = read();
  q.push([S, rt[S]]);
  while ((!q.empty()))
  {
    var u = q.top();
    q.pop();
    if ((u.rt != rt[u.x]))
    {
      continue;
    }
    if ((u.x == T))
    {
      print(T);
    }
    {
      var i = head[u.x];
      while (i)
      {
        var v = ver[i];
        var RT: dynamic;
        update(u.rt, RT, 0, lim, edge[i]);
        if (((!rt[v]) || cmp(rt[v], RT, 0, lim)))
        {
          rt[v] = RT;
          q.push([v, rt[v]]);
          Pre[v] = u.x;
        }
        i = Next[i];
      }
    }
  }
  puts("-1");
  return 0;
}
