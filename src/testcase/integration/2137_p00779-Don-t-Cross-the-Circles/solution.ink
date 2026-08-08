// Translated from solution.cpp.

var pu = cpp_expression("#inc");

var pb = cpp_expression("#include");

var mp = cpp_expression("#include");

var INF = cpp_expression("#include <");

var fi = cpp_expression("#incl");

var sc = cpp_expression("#inclu");

func rep(i: dynamic, x: dynamic)
{
  cpp_macro("for(int i=0;i<x;i++)");
}

func SORT(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h");
}

func ERASE(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using namespace s");
}

func POSL(x: dynamic, v: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std");
}

func POSU(x: dynamic, v: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std");
}

var EPS = cpp_expression("#inc");

var X = cpp_expression("#inclu");

var Y = cpp_expression("#inclu");

var c = cpp_array(105);

var r = cpp_array(105);

var edge = cpp_array(105);

var n: dynamic;

var m: dynamic;

func eq(a: dynamic, b: dynamic)
{
  return ((((-EPS) < (a - b)) && ((a - b) < EPS)));
}

func cmp(a: dynamic, b: dynamic)
{
  assert((a.sc == b.sc));
  var A = atan2((c[a.fi].Y - c[a.sc].Y), (c[a.fi].X - c[a.sc].X));
  var B = atan2((c[b.fi].Y - c[b.sc].Y), (c[b.fi].X - c[b.sc].X));
  return (A < B);
}

var ran = cpp_array(105, 105);

var convex = cpp_array(105, 105);

var used = cpp_array(105, 105);

func dot(a: dynamic, b: dynamic)
{
  return ((conj(a) * b)).X;
}

func cross(a: dynamic, b: dynamic)
{
  return ((conj(a) * b)).Y;
}

func ccw(a: dynamic, b: dynamic, c: dynamic)
{
  b -= a;
  c -= a;
  if ((cross(b, c) > EPS))
  {
    return 1;
  }
  if ((cross(b, c) < (-EPS)))
  {
    return -1;
  }
  if ((dot(b, c) < (-EPS)))
  {
    return 2;
  }
  if ((norm(b) < norm(c)))
  {
    return -2;
  }
  return 0;
}

func crossPoint(a: dynamic, b: dynamic, c: dynamic, d: dynamic)
{
  var A = cross((b - a), (d - c));
  var B = cross((b - a), (b - c));
  if (((fabs(A) < EPS) && (fabs(B) < EPS)))
  {
    return c;
  } else if ((fabs(A) >= EPS))
  {
    return (c + ((B / A) * ((d - c))));
  } else
  {
    pt(1e9, 1e9);
  }
}

func on_segment(a: dynamic, p: dynamic)
{
  return eq(((abs((a.first - a.second)) - abs((a.first - p))) - abs((a.second - p))), 0);
}

func dist_lp(a: dynamic, b: dynamic, c: dynamic)
{
  if ((dot((a - b), (c - b)) <= 0.0))
  {
    return abs((c - b));
  }
  if ((dot((b - a), (c - a)) <= 0.0))
  {
    return abs((c - a));
  }
  return (abs(cross((b - a), (c - a))) / abs((b - a)));
}

func intersect(a: dynamic, b: dynamic, c: dynamic, d: dynamic)
{
  return ((((ccw(a, b, c) * ccw(a, b, d)) <= 0) && ((ccw(c, d, a) * ccw(c, d, b)) <= 0)));
}

func contain_point(ps: dynamic, p: dynamic)
{
  var sum = 0;
  {
    var i = 0;
    while ((i < ps.size()))
    {
      if (on_segment(mp(ps[i], ps[(((i + 1)) % ps.size())]), p))
      {
        return 1;
      }
      sum += arg((((ps[(((i + 1)) % ps.size())] - p)) / ((ps[i] - p))));
      i += 1;
    }
  }
  return ((abs(sum) > 1));
}

func main()
{
  while (1)
  {
    scanf("%d%d", (&n), (&m));
    if (((n == 0) && (m == 0)))
    {
      return 0;
    }
    {
      var i = 1;
      while ((i <= n))
      {
        var x: dynamic;
        var y: dynamic;
        scanf("%lf%lf%lf", (&x), (&y), (&r[i]));
        c[i] = pt(x, y);
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < 105))
      {
        edge[i].clear();
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= n))
      {
        {
          var j = (i + 1);
          while ((j <= n))
          {
            if (((abs((c[i] - c[j])) - EPS) > (r[i] + r[j])))
            {
              j += 1;
              continue;
            }
            if ((abs((c[i] - c[j])) < ((max(r[i], r[j]) - min(r[i], r[j])) - EPS)))
            {
              j += 1;
              continue;
            }
            edge[i].pb(mp(j, i));
            edge[j].pb(mp(i, j));
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= n))
      {
        sort(edge[i].begin(), edge[i].end(), cmp);
        {
          var j = 0;
          while ((j < edge[i].size()))
          {
            ran[i][edge[i][j].fi] = j;
            j += 1;
          }
        }
        i += 1;
      }
    }
    memset(used, 0, cpp_sizeof((used)));
    {
      var i = 1;
      while ((i <= n))
      {
        {
          var j = 0;
          while ((j < edge[i].size()))
          {
            convex[i][j].clear();
            var cur = i;
            var nxt = edge[i][j].fi;
            if (used[cur][nxt])
            {
              j += 1;
              continue;
            }
            while (true)
            {
              convex[i][j].pb(c[cur]);
              used[cur][nxt] = 1;
              var C = edge[nxt][(((ran[nxt][cur] + 1)) % edge[nxt].size())].fi;
              swap(cur, nxt);
              nxt = C;
              if (!(((cur != i))))
              {
                break;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= m))
      {
        var p1: dynamic;
        var p2: dynamic;
        var xa: dynamic;
        var xb: dynamic;
        var xc: dynamic;
        var xd: dynamic;
        scanf("%lf%lf%lf%lf", (&xa), (&xb), (&xc), (&xd));
        p1 = pt(xa, xb);
        p2 = pt(xc, xd);
        var cnt = 0;
        var ok = 1;
        {
          var j = 1;
          while ((j <= n))
          {
            var b1 = 0;
            var b2 = 0;
            if ((abs((c[j] - p1)) < (r[j] + EPS)))
            {
              b1 = 1;
            }
            if ((abs((c[j] - p2)) < (r[j] + EPS)))
            {
              b2 = 1;
            }
            if (((!b1) && b2))
            {
              ok = 0;
              cpp_goto("goto bad;");
            }
            if ((b1 && (!b2)))
            {
              ok = 0;
              cpp_goto("goto bad;");
            }
            cnt += b1;
            j += 1;
          }
        }
        if (cnt)
        {
          cpp_goto("goto bad;");
        }
        {
          var j = 1;
          while ((j <= n))
          {
            {
              var w = 0;
              while ((w < edge[j].size()))
              {
                if ((convex[j][w].size() <= 2))
                {
                  w += 1;
                  continue;
                }
                if (((contain_point(convex[j][w], p1) ^ contain_point(convex[j][w], p2))))
                {
                  ok = 0;
                  cpp_goto("goto bad;");
                }
                w += 1;
              }
            }
            j += 1;
          }
        }
        printf(if (ok) "YES%c" else "NO%c", (if ((i == m)) cpp_char("\n") else cpp_char(" ")));
        i += 1;
      }
    }
  }
}
