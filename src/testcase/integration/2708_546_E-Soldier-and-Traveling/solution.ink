// Translated from solution.cpp.

var N: dynamic = 203;

var M: dynamic = 700;

var MAXINT: dynamic = (1 << 20);

class NetWorkFlow
{
  var a: dynamic = cpp_array(N);
  var b: dynamic = cpp_array((M * 2));
  var d: dynamic = cpp_array(M);
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  func clear(nn: dynamic, ss: dynamic, tt: dynamic) -> dynamic
  {
      n = nn;
      s = ss;
      t = tt;
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          a[i].fe = -1;
          a[i].v = 0;
          i += 1;
        }
      }
      p = 0;
    }
  func putedge(x: dynamic, y: dynamic, f: dynamic) -> dynamic
  {
      b[p].t = y;
      b[p].f = f;
      b[p].ne = a[x].fe;
      a[x].fe = cpp_update(p, "++");
      b[p].t = x;
      b[p].f = 0;
      b[p].ne = a[y].fe;
      a[y].fe = cpp_update(p, "++");
    }
  func bfs() -> dynamic
  {
      var i: dynamic = cpp_uninitialized();
      var p: dynamic = cpp_uninitialized();
      var q: dynamic = cpp_uninitialized();
      var j: dynamic = cpp_uninitialized();
      {
        i = 1;
        while ((i <= n))
        {
          a[i].h = 0;
          i += 1;
        }
      }
      a[s].h = 1;
      p = cpp_assign(q, "=", 0);
      d[cpp_update(q, "++")] = s;
      while ((p < q))
      {
        i = d[p];
        {
          j = a[i].fe;
          while ((j != -1))
          {
            if (((a[b[j].t].h == 0) && (b[j].f > 0)))
            {
              a[b[j].t].h = (a[i].h + 1);
              if ((b[j].t == t))
              {
                return true;
              }
              d[cpp_update(q, "++")] = b[j].t;
            }
            j = b[j].ne;
          }
        }
        p += 1;
      }
      return false;
    }
  func dfs(i: dynamic, v: dynamic) -> dynamic
  {
      if ((i == t))
      {
        a[t].v += v;
        return v;
      }
      var ans: dynamic = 0;
      {
        var j: dynamic = a[i].cur;
        while ((j != -1))
        {
          if (((b[j].f > 0) && (a[b[j].t].h > a[i].h)))
          {
            var tmp: dynamic = dfs(b[j].t, min(b[j].f, v));
            v -= tmp;
            b[j].f -= tmp;
            b[(j ^ 1)].f += tmp;
            ans += tmp;
            if ((v == 0))
            {
              return ans;
            }
          }
          j = b[j].ne;
        }
      }
      a[i].v += ans;
      return ans;
    }
  func flow() -> dynamic
  {
      var i: dynamic = cpp_uninitialized();
      a[s].v = MAXINT;
      while (bfs())
      {
        {
          i = 1;
          while ((i <= n))
          {
            a[i].cur = a[i].fe;
            i += 1;
          }
        }
        dfs(s, MAXINT);
      }
      return a[t].v;
    }
}

var c: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var suma: dynamic = cpp_uninitialized();

var sumb: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_array(101, 101);

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  s = ((n + n) + 1);
  t = ((n + n) + 2);
  c.clear(((n + n) + 2), s, t);
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&x));
      c.putedge(s, i, x);
      c.putedge(i, (i + n), x);
      suma += x;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&x));
      c.putedge((i + n), t, x);
      sumb += x;
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < m))
    {
      scanf("%d%d", (&x), (&y));
      c.putedge(x, (y + n), MAXINT);
      c.putedge(y, (x + n), MAXINT);
      i += 1;
    }
  }
  if (((suma != sumb) || (c.flow() != sumb)))
  {
    printf("NO\n");
  } else
  {
    printf("YES\n");
    {
      i = 1;
      while ((i <= n))
      {
        {
          j = c.a[i].fe;
          while ((j != -1))
          {
            if ((c.b[j].t <= (n + n)))
            {
              ans[i][(c.b[j].t - n)] = c.b[(j ^ 1)].f;
            }
            j = c.b[j].ne;
          }
        }
        i += 1;
      }
    }
    {
      i = 1;
      while ((i <= n))
      {
        {
          j = 1;
          while ((j <= n))
          {
            printf("%d ", ans[i][j]);
            j += 1;
          }
        }
        printf("\n");
        i += 1;
      }
    }
  }
  return 0;
}
