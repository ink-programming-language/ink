// Translated from solution.cpp.

var buf: dynamic = cpp_array((1 << 21));

var p1: dynamic = buf;

var p2: dynamic = buf;

func cmax(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a < b)) ? cpp_comma(cpp_assign(a, "=", b), 1) : 0;
}

func read() -> dynamic
{
  var ch: dynamic = cpp_uninitialized();
  var flag: dynamic = 0;
  var res: dynamic = cpp_uninitialized();
  while ((!isdigit(cpp_assign(ch, "=",  ((cpp_comma(((p1 == p2) && (cpp_assign(p2, "=", ((cpp_assign(p1, "=", buf)) + fread(buf, 1, (1 << 21), stdin))))), (p1 == p2)))) ? EOF : (*cpp_update(p1, "++"))))))
  {
    (((ch == cpp_char("-"))) && (cpp_assign(flag, "=", true)));
  }
  {
    res = (ch - cpp_char("0"));
    while (isdigit(cpp_assign(ch, "=",  ((cpp_comma(((p1 == p2) && (cpp_assign(p2, "=", ((cpp_assign(p1, "=", buf)) + fread(buf, 1, (1 << 21), stdin))))), (p1 == p2)))) ? EOF : (*cpp_update(p1, "++")))))
    {
      res = (((res * 10) + ch) - cpp_char("0"));
    }
  }
  ((flag) && (cpp_assign(res, "=", (-res))));
  return res;
}

var N: dynamic = (1e5 + 5);

var mod: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var head: dynamic = cpp_array(N);

var Next: dynamic = cpp_array((N << 1));

var ver: dynamic = cpp_array((N << 1));

var edge: dynamic = cpp_array((N << 1));

var S: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var lim: dynamic = cpp_uninitialized();

var b: dynamic = cpp_array((N << 1));

var rt: dynamic = cpp_array(N);

var Pre: dynamic = cpp_array(N);

var tot: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var L: dynamic = cpp_array((N * 120));

var R: dynamic = cpp_array((N * 120));

var sum: dynamic = cpp_array((N * 120));

func add(u: dynamic, v: dynamic, e: dynamic) -> dynamic
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

func cmp(u: dynamic, v: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if ((l == r))
  {
    return (sum[u] > sum[v]);
  }
  var mid: dynamic = (((l + r)) >> 1);
  if ((sum[R[u]] == sum[R[v]]))
  {
    return cmp(L[u], L[v], l, mid);
  } else
  {
    return cmp(R[u], R[v], (mid + 1), r);
  }
}

func update(last: dynamic, now: dynamic, l: dynamic, r: dynamic, k: dynamic) -> dynamic
{
  L[cpp_assign(now, "=", cpp_update(cnt, "++"))] = L[last];
  R[now] = R[last];
  if ((l == r))
  {
    sum[now] = (sum[last] ^ 1);
    return sum[last];
  }
  var mid: dynamic = (((l + r)) >> 1);
  var res: dynamic = cpp_uninitialized();
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
  var x: dynamic = cpp_uninitialized();
  var rt: dynamic = cpp_uninitialized();
  func operator_less(b: dynamic) -> dynamic
  {
      return cmp(rt, b.rt, 0, lim);
    }
}

var q: dynamic = cpp_uninitialized();

func dfs(u: dynamic, dep: dynamic) -> dynamic
{
  if ((u == S))
  {
    printf("%d\n%d ", dep, u);
    return;
  }
  dfs(Pre[u], (dep + 1));
  printf("%d ", u);
}

func print(u: dynamic) -> dynamic
{
  printf("%d\n", sum[rt[u]]);
  dfs(u, 1);
  exit(0);
}

func main() -> dynamic
{
  n = read();
  m = read();
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var e: dynamic = cpp_uninitialized();
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
    var i: dynamic = 1;
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
    var u: dynamic = q.top();
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
      var i: dynamic = head[u.x];
      while (i)
      {
        var v: dynamic = ver[i];
        var RT: dynamic = cpp_uninitialized();
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
