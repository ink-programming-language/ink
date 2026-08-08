// Translated from solution.cpp.

func read()
{
  var x = 0;
  var f = 1;
  var ch = getchar();
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

var d = cpp_array((1001 + 5));

var in_cpp = cpp_array((1001 + 5));

var n: dynamic;

var m: dynamic;

var q = cpp_array((1001 + 5));

var head = cpp_array((1001 + 5));

var cnt: dynamic;

var top: dynamic;

var c = cpp_array((1001 + 5));

class Edge
{
  var u: dynamic;
  var v: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
}

var s = cpp_array((2000 + 5));

class edge
{
  var to: dynamic;
  var next: dynamic;
  var w: dynamic;
}

var e = cpp_array(100005);

func ins(f: dynamic, t: dynamic, w: dynamic)
{
  e[cpp_update(cnt, "++")] = [t, head[f], w];
  head[f] = cnt;
  e[cpp_update(cnt, "++")] = [f, head[t], 0];
  head[t] = cnt;
}

func bfs()
{
  {
    var i = 1;
    while ((i <= 1001))
    {
      d[i] = 1e9;
      i += 1;
    }
  }
  var i: dynamic;
  var j: dynamic;
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

func dfs(x: dynamic, f: dynamic)
{
  if ((x == 1001))
  {
    return f;
  }
  var used = 0;
  {
    var i = c[x];
    while (i)
    {
      if (((e[i].w > 1e-11) && (d[e[i].to] == (d[x] + 1))))
      {
        var w = dfs(e[i].to, min((f - used), e[i].w));
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

func Solve(t: dynamic)
{
  cnt = 1;
  memset(head, 0, cpp_sizeof((head)));
  memset(in_cpp, 0, cpp_sizeof((in_cpp)));
  {
    var i = 1;
    while ((i <= m))
    {
      var l = ((t * s[i].a) + s[i].b);
      var r = ((t * s[i].c) + s[i].d);
      ins(s[i].u, s[i].v, (r - l));
      in_cpp[s[i].v] += l;
      in_cpp[s[i].u] -= l;
      i += 1;
    }
  }
  var res = 0;
  {
    var i = 1;
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

func main()
{
  n = read();
  m = read();
  {
    var i = 1;
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
  var l = 0;
  var r = 1;
  var ok = -1;
  {
    var i = 1;
    while ((i <= 50))
    {
      var m1 = (l + (((r - l)) / 3));
      var m2 = (m1 + (((r - l)) / 3));
      var r1 = Solve(m1);
      var r2 = Solve(m2);
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
  var L = ok;
  var R = ok;
  {
    var i = 1;
    while ((i <= 50))
    {
      var mid = (((l + r)) * 0.5);
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
    var i = 1;
    while ((i <= 50))
    {
      var mid = (((l + r)) * 0.5);
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
