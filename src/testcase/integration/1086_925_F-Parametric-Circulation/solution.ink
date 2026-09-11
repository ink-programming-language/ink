// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * f);
}

var d: dynamic = cpp_array((1001 + 5));

var in_cpp: dynamic = cpp_array((1001 + 5));

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var q: dynamic = cpp_array((1001 + 5));

var head: dynamic = cpp_array((1001 + 5));

var cnt: dynamic = cpp_uninitialized();

var top: dynamic = cpp_uninitialized();

var c: dynamic = cpp_array((1001 + 5));

class Edge
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
}

var s: dynamic = cpp_array((2000 + 5));

class edge
{
  var to: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array(100005);

func ins(f: dynamic, t: dynamic, w: dynamic) -> dynamic
{
  e[cpp_update(cnt, "++")] = [t, head[f], w];
  head[f] = cnt;
  e[cpp_update(cnt, "++")] = [f, head[t], 0];
  head[t] = cnt;
}

func bfs() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= 1001))
    {
      d[i] = 1e9;
      i += 1;
    }
  }
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  {
    d[cpp_assign(q[cpp_assign(top, "=", cpp_assign(i, "=", 1))], "=", 0)] = 0;
    while ((i <= top))
    {
      {
        j = cpp_assign(c[q[i]], "=", head[q[i]]);
        while (j)
        {
          if (((e[j].w > 1e-11) && ((d[q[i]] + 1) < d[e[j].to])))
          {
            d[cpp_assign(q[cpp_update(top, "++")], "=", e[j].to)] = (d[q[i]] + 1);
          }
          j = e[j].next;
        }
      }
      i += 1;
    }
  }
  return (d[1001] < 1e8);
}

func dfs(x: dynamic, f: dynamic) -> dynamic
{
  if ((x == 1001))
  {
    return f;
  }
  var used: dynamic = 0;
  {
    var i: dynamic = c[x];
    while (i)
    {
      if (((e[i].w > 1e-11) && (d[e[i].to] == (d[x] + 1))))
      {
        var w: dynamic = dfs(e[i].to, min((f - used), e[i].w));
        used += w;
        e[i].w -= w;
        e[(i ^ 1)].w += w;
        if (((f - used) < 1e-11))
        {
          return f;
        }
      }
      i = e[i].next;
    }
  }
  return used;
}

func Solve(t: dynamic) -> dynamic
{
  cnt = 1;
  memset(head, 0, cpp_sizeof((head)));
  memset(in_cpp, 0, cpp_sizeof((in_cpp)));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var l: dynamic = ((t * s[i].a) + s[i].b);
      var r: dynamic = ((t * s[i].c) + s[i].d);
      ins(s[i].u, s[i].v, (r - l));
      in_cpp[s[i].v] += l;
      in_cpp[s[i].u] -= l;
      i += 1;
    }
  }
  var res: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((in_cpp[i] > 0))
      {
        res += in_cpp[i];
        ins(0, i, in_cpp[i]);
      } else
      {
        ins(i, 1001, (-in_cpp[i]));
      }
      i += 1;
    }
  }
  while (bfs())
  {
    res -= dfs(0, 1e9);
  }
  return res;
}

func main() -> dynamic
{
  n = read();
  m = read();
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      s[i].u = read();
      s[i].v = read();
      s[i].a = read();
      s[i].b = read();
      s[i].c = read();
      s[i].d = read();
      i += 1;
    }
  }
  var l: dynamic = 0;
  var r: dynamic = 1;
  var ok: dynamic = -1;
  {
    var i: dynamic = 1;
    while ((i <= 50))
    {
      var m1: dynamic = (l + (((r - l)) / 3));
      var m2: dynamic = (m1 + (((r - l)) / 3));
      var r1: dynamic = Solve(m1);
      var r2: dynamic = Solve(m2);
      if ((r1 < 1e-11))
      {
        ok = m1;
        break;
      }
      if ((r2 < 1e-11))
      {
        ok = m2;
        break;
      }
      if ((r1 < r2))
      {
        r = m2;
      } else
      {
        l = m1;
      }
      i += 1;
    }
  }
  if ((ok < 0))
  {
    return (0 * puts("0"));
  }
  l = 0;
  r = ok;
  var L: dynamic = ok;
  var R: dynamic = ok;
  {
    var i: dynamic = 1;
    while ((i <= 50))
    {
      var mid: dynamic = (((l + r)) * 0.5);
      if ((Solve(mid) < 1e-11))
      {
        L = mid;
        r = mid;
      } else
      {
        l = mid;
      }
      i += 1;
    }
  }
  l = ok;
  r = 1;
  {
    var i: dynamic = 1;
    while ((i <= 50))
    {
      var mid: dynamic = (((l + r)) * 0.5);
      if ((Solve(mid) < 1e-11))
      {
        R = mid;
        l = mid;
      } else
      {
        r = mid;
      }
      i += 1;
    }
  }
  printf("%.10lf\n", cpp_cast(((R - L))));
  return 0;
}
