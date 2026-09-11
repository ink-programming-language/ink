// Translated from solution.cpp.

var EPS: dynamic = 1e-6;

var INF: dynamic = 1e12;

var PI: dynamic = acos(-1);

func EQ(n: dynamic, m: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream>");
}

var X: dynamic = cpp_expression("#inclu");

var Y: dynamic = cpp_expression("#inclu");

class L
{
  func L(a: dynamic, b: dynamic) -> dynamic
  {
      at(0) = a;
      at(1) = b;
    }
  func L() -> dynamic
  {
    }
}

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  return  ((!EQ(a.X, b.X))) ? (a.X < b.X) : ((a.Y + EPS) < b.Y);
}

func operator_equal(a: dynamic, b: dynamic) -> dynamic
{
  return (abs((a - b)) < EPS);
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((conj(a) * b)).X;
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((conj(a) * b)).Y;
}

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
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

func intersectSS(a: dynamic, b: dynamic) -> dynamic
{
  return ((((ccw(a[0], a[1], b[0]) * ccw(a[0], a[1], b[1])) <= 0)) && (((ccw(b[0], b[1], a[0]) * ccw(b[0], b[1], a[1])) <= 0)));
}

func intersectSP(s: dynamic, p: dynamic) -> dynamic
{
  return ((abs(cross((s[0] - p), (s[1] - p))) < EPS) && (dot((s[0] - p), (s[1] - p)) < EPS));
}

func isParallel(a: dynamic, b: dynamic) -> dynamic
{
  return (abs(cross(a, b)) < EPS);
}

func isParallel(a: dynamic, b: dynamic) -> dynamic
{
  return isParallel((a[1] - a[0]), (b[1] - b[0]));
}

func crosspointLL(l: dynamic, m: dynamic) -> dynamic
{
  var A: dynamic = cross((l[1] - l[0]), (m[1] - m[0]));
  var B: dynamic = cross((l[1] - l[0]), (l[1] - m[0]));
  return (m[0] + ((B / A) * ((m[1] - m[0]))));
}

func in_poly(p: dynamic, poly: dynamic) -> dynamic
{
  var n: dynamic = poly.size();
  var ret: dynamic = -1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = (poly[i] - p);
      var b: dynamic = (poly[(((i + 1)) % n)] - p);
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

func convex(v: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  var n: dynamic = v.size();
  sort(v.begin(), v.end());
  {
    var i: dynamic = 0;
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
  var t: dynamic = ret.size();
  {
    var i: dynamic = (n - 2);
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

func arrangementEX(l: dynamic, p: dynamic) -> dynamic
{
  var cp: dynamic = cpp_construct(l.size());
  var plist: dynamic = p;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(l.size())))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < cpp_cast(l.size())))
        {
          if (((!isParallel(l[i], l[j])) && intersectSS(l[i], l[j])))
          {
            var cpij: dynamic = crosspointLL(l[i], l[j]);
            cp[i].push_back(cpij);
            cp[j].push_back(cpij);
            plist.push_back(cpij);
          }
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
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
  var n: dynamic = plist.size();
  var conv: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      conv[plist[i]] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(cp.size())))
    {
      {
        var j: dynamic = 0;
        while ((j < (cpp_cast(cp[i].size()) - 1)))
        {
          var jidx: dynamic = conv[cp[i][j]];
          var jp1idx: dynamic = conv[cp[i][(j + 1)]];
          adj[jidx][jp1idx] = cpp_assign(adj[jp1idx][jidx], "=", 0);
          j += 1;
        }
      }
      i += 1;
    }
  }
  return make_pair(adj, plist);
}

func main() -> dynamic
{
  while (1)
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    read(n, m);
    if ((n == 0))
    {
      break;
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var nv: dynamic = cpp_uninitialized();
        read(nv, h[i]);
        poly[i].resize(nv);
        {
          var j: dynamic = 0;
          while ((j < nv))
          {
            var x: dynamic = cpp_uninitialized();
            var y: dynamic = cpp_uninitialized();
            read(x, y);
            poly[i][j] = P(x, y);
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        var xs: dynamic = cpp_uninitialized();
        var ys: dynamic = cpp_uninitialized();
        var xt: dynamic = cpp_uninitialized();
        var yt: dynamic = cpp_uninitialized();
        read(xs, ys, xt, yt);
        lines[i] = L(P(xs, ys), P(xt, yt));
        i += 1;
      }
    }
    var th: dynamic = cpp_uninitialized();
    var phi: dynamic = cpp_uninitialized();
    read(th, phi);
    th *= (PI / 180);
    phi *= (PI / 180);
    var dir: dynamic = (P(cos((th + PI)), sin((th + PI))) / tan(phi));
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var tmp: dynamic = poly[i];
        var idir: dynamic = (dir * h[i]);
        {
          var j: dynamic = 0;
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
    var sg: dynamic = cpp_construct(2);
    var sx: dynamic = cpp_uninitialized();
    var sy: dynamic = cpp_uninitialized();
    var tx: dynamic = cpp_uninitialized();
    var ty: dynamic = cpp_uninitialized();
    read(sx, sy, tx, ty);
    sg[0] = P(sx, sy);
    sg[1] = P(tx, ty);
    var ret: dynamic = arrangementEX(lines, sg);
    var adj: dynamic = ret.first;
    var plist: dynamic = ret.second;
    var pn: dynamic = plist.size();
    var sidx: dynamic = (lower_bound(plist.begin(), plist.end(), sg[0]) - plist.begin());
    var gidx: dynamic = (lower_bound(plist.begin(), plist.end(), sg[1]) - plist.begin());
    {
      var i: dynamic = 0;
      while ((i < pn))
      {
        {
          var j: dynamic = (i + 1);
          while ((j < pn))
          {
            if ((adj[i][j] == INF))
            {
              j += 1;
              continue;
            }
            var cp: dynamic = cpp_construct(2);
            cp[0] = e[0];
            cp[1] = e[1];
            {
              var k: dynamic = 0;
              while ((k < n))
              {
                var vn: dynamic = poly[k].size();
                {
                  var l: dynamic = 0;
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
            var cost: dynamic = 0;
            {
              var k: dynamic = 0;
              while ((k < (cpp_cast(cp.size()) - 1)))
              {
                var mid: dynamic = (((cp[k] + cp[(k + 1)])) / 2.0);
                var in_cpp: dynamic = false;
                {
                  var l: dynamic = 0;
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
      var k: dynamic = 0;
      while ((k < pn))
      {
        {
          var i: dynamic = 0;
          while ((i < pn))
          {
            {
              var j: dynamic = 0;
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
