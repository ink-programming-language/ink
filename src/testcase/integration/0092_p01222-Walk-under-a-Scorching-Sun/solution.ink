// Translated from solution.cpp.

var EPS = 1e-6;

var INF = 1e12;

var PI = acos(-1);

func EQ(n: dynamic, m: dynamic)
{
  return cpp_expression("#include <iostream>");
}

var X = cpp_expression("#inclu");

var Y = cpp_expression("#inclu");

class L
{
  func L(a: dynamic, b: dynamic)
  {
      at(0) = a;
      at(1) = b;
    }
  func L()
  {
    }
}

func operator_less(a: dynamic, b: dynamic)
{
  return if ((!EQ(a.X, b.X))) (a.X < b.X) else ((a.Y + EPS) < b.Y);
}

func operator_equal(a: dynamic, b: dynamic)
{
  return (abs((a - b)) < EPS);
}

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
    return +1;
  }
  if ((cross(b, c) < (-EPS)))
  {
    return -1;
  }
  if ((dot(b, c) < (-EPS)))
  {
    return +2;
  }
  if (((abs(c) - abs(b)) > EPS))
  {
    return -2;
  }
  return 0;
}

func intersectSS(a: dynamic, b: dynamic)
{
  return ((((ccw(a[0], a[1], b[0]) * ccw(a[0], a[1], b[1])) <= 0)) && (((ccw(b[0], b[1], a[0]) * ccw(b[0], b[1], a[1])) <= 0)));
}

func intersectSP(s: dynamic, p: dynamic)
{
  return ((abs(cross((s[0] - p), (s[1] - p))) < EPS) && (dot((s[0] - p), (s[1] - p)) < EPS));
}

func isParallel(a: dynamic, b: dynamic)
{
  return (abs(cross(a, b)) < EPS);
}

func isParallel(a: dynamic, b: dynamic)
{
  return isParallel((a[1] - a[0]), (b[1] - b[0]));
}

func crosspointLL(l: dynamic, m: dynamic)
{
  var A = cross((l[1] - l[0]), (m[1] - m[0]));
  var B = cross((l[1] - l[0]), (l[1] - m[0]));
  return (m[0] + ((B / A) * ((m[1] - m[0]))));
}

func in_poly(p: dynamic, poly: dynamic)
{
  var n = poly.size();
  var ret = -1;
  {
    var i = 0;
    while ((i < n))
    {
      var a = (poly[i] - p);
      var b = (poly[(((i + 1)) % n)] - p);
      if ((a.Y > b.Y))
      {
        swap(a, b);
      }
      if (intersectSP(L(a, b), P(0, 0)))
      {
        return 0;
      }
      if ((((a.Y <= 0) && (b.Y > 0)) && (cross(a, b) < 0)))
      {
        ret = (-ret);
      }
      i += 1;
    }
  }
  return ret;
}

func convex(v: dynamic)
{
  var ret: dynamic;
  var n = v.size();
  sort(v.begin(), v.end());
  {
    var i = 0;
    while ((i < n))
    {
      while (((cpp_cast(ret.size()) > 1) && (cross((ret.back() - ret[(ret.size() - 2)]), (v[i] - ret.back())) < EPS)))
      {
        ret.pop_back();
      }
      ret.push_back(v[i]);
      i += 1;
    }
  }
  var t = ret.size();
  {
    var i = (n - 2);
    while ((i >= 0))
    {
      while (((cpp_cast(ret.size()) > t) && (cross((ret.back() - ret[(ret.size() - 2)]), (v[i] - ret.back())) < EPS)))
      {
        ret.pop_back();
      }
      ret.push_back(v[i]);
      i -= 1;
    }
  }
  if ((cpp_cast(ret.size()) > 1))
  {
    ret.pop_back();
  }
  return ret;
}

func arrangementEX(l: dynamic, p: dynamic)
{
  var cp = cpp_construct(l.size());
  var plist = p;
  {
    var i = 0;
    while ((i < cpp_cast(l.size())))
    {
      {
        var j = (i + 1);
        while ((j < cpp_cast(l.size())))
        {
          if (((!isParallel(l[i], l[j])) && intersectSS(l[i], l[j])))
          {
            var cpij = crosspointLL(l[i], l[j]);
            cp[i].push_back(cpij);
            cp[j].push_back(cpij);
            plist.push_back(cpij);
          }
          j += 1;
        }
      }
      {
        var j = 0;
        while ((j < cpp_cast(p.size())))
        {
          if (intersectSP(l[i], p[j]))
          {
            cp[i].push_back(p[j]);
          }
          j += 1;
        }
      }
      cp[i].push_back(l[i][0]);
      cp[i].push_back(l[i][1]);
      plist.push_back(l[i][0]);
      plist.push_back(l[i][1]);
      sort(cp[i].begin(), cp[i].end());
      cp[i].erase(unique(cp[i].begin(), cp[i].end()), cp[i].end());
      i += 1;
    }
  }
  sort(plist.begin(), plist.end());
  plist.erase(unique(plist.begin(), plist.end()), plist.end());
  var n = plist.size();
  var conv: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      conv[plist[i]] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < cpp_cast(cp.size())))
    {
      {
        var j = 0;
        while ((j < (cpp_cast(cp[i].size()) - 1)))
        {
          var jidx = conv[cp[i][j]];
          var jp1idx = conv[cp[i][(j + 1)]];
          adj[jidx][jp1idx] = cpp_assign(adj[jp1idx][jidx], "=", 0);
          j += 1;
        }
      }
      i += 1;
    }
  }
  return make_pair(adj, plist);
}

func main()
{
  while (1)
  {
    var n: dynamic;
    var m: dynamic;
    read(n, m);
    if ((n == 0))
    {
      break;
    }
    {
      var i = 0;
      while ((i < n))
      {
        var nv: dynamic;
        read(nv, h[i]);
        poly[i].resize(nv);
        {
          var j = 0;
          while ((j < nv))
          {
            var x: dynamic;
            var y: dynamic;
            read(x, y);
            poly[i][j] = P(x, y);
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < m))
      {
        var xs: dynamic;
        var ys: dynamic;
        var xt: dynamic;
        var yt: dynamic;
        read(xs, ys, xt, yt);
        lines[i] = L(P(xs, ys), P(xt, yt));
        i += 1;
      }
    }
    var th: dynamic;
    var phi: dynamic;
    read(th, phi);
    th *= (PI / 180);
    phi *= (PI / 180);
    var dir = (P(cos((th + PI)), sin((th + PI))) / tan(phi));
    {
      var i = 0;
      while ((i < n))
      {
        var tmp = poly[i];
        var idir = (dir * h[i]);
        {
          var j = 0;
          while ((j < cpp_cast(poly[i].size())))
          {
            tmp.push_back((poly[i][j] + idir));
            j += 1;
          }
        }
        poly[i] = convex(tmp);
        i += 1;
      }
    }
    var sg = cpp_construct(2);
    var sx: dynamic;
    var sy: dynamic;
    var tx: dynamic;
    var ty: dynamic;
    read(sx, sy, tx, ty);
    sg[0] = P(sx, sy);
    sg[1] = P(tx, ty);
    var ret = arrangementEX(lines, sg);
    var adj = ret.first;
    var plist = ret.second;
    var pn = plist.size();
    var sidx = (lower_bound(plist.begin(), plist.end(), sg[0]) - plist.begin());
    var gidx = (lower_bound(plist.begin(), plist.end(), sg[1]) - plist.begin());
    {
      var i = 0;
      while ((i < pn))
      {
        {
          var j = (i + 1);
          while ((j < pn))
          {
            if ((adj[i][j] == INF))
            {
              j += 1;
              continue;
            }
            var cp = cpp_construct(2);
            cp[0] = e[0];
            cp[1] = e[1];
            {
              var k = 0;
              while ((k < n))
              {
                var vn = poly[k].size();
                {
                  var l = 0;
                  while ((l < vn))
                  {
                    if (((!isParallel(e, edge)) && intersectSS(e, edge)))
                    {
                      cp.push_back(crosspointLL(e, edge));
                    }
                    l += 1;
                  }
                }
                k += 1;
              }
            }
            sort(cp.begin(), cp.end());
            cp.erase(unique(cp.begin(), cp.end()), cp.end());
            var cost = 0;
            {
              var k = 0;
              while ((k < (cpp_cast(cp.size()) - 1)))
              {
                var mid = (((cp[k] + cp[(k + 1)])) / 2.0);
                var in_cpp = false;
                {
                  var l = 0;
                  while ((l < n))
                  {
                    if ((in_poly(mid, poly[l]) >= 0))
                    {
                      in_cpp = true;
                      break;
                    }
                    l += 1;
                  }
                }
                if ((!in_cpp))
                {
                  cost += abs((cp[(k + 1)] - cp[k]));
                }
                k += 1;
              }
            }
            adj[i][j] = cpp_assign(adj[j][i], "=", cost);
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var k = 0;
      while ((k < pn))
      {
        {
          var i = 0;
          while ((i < pn))
          {
            {
              var j = 0;
              while ((j < pn))
              {
                adj[i][j] = min(adj[i][j], (adj[i][k] + adj[k][j]));
                j += 1;
              }
            }
            i += 1;
          }
        }
        k += 1;
      }
    }
    write(fixed, setprecision(4));
    write(adj[sidx][gidx], "\n");
  }
  return 0;
}
