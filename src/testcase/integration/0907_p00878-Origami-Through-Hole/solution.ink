// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0; i<(int)(n); i++)");
}

var mp: dynamic = cpp_expression("#include");

var EPS: dynamic = cpp_expression("#inclu");

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return imag((conj(a) * b));
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return real((conj(a) * b));
}

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  b -= a;
  c -= a;
  if ((cross(b, c) > 0))
  {
    return 1;
  }
  if ((cross(b, c) < 0))
  {
    return -1;
  }
  if ((dot(b, c) < 0))
  {
    return 2;
  }
  if ((norm(b) < norm(c)))
  {
    return -2;
  }
  return 0;
}

func projection(l0: dynamic, l1: dynamic, p: dynamic) -> dynamic
{
  var t: dynamic = (dot((p - l0), (l1 - l0)) / norm((l1 - l0)));
  return (l0 + (t * ((l1 - l0))));
}

func intersectSP(s0: dynamic, s1: dynamic, p: dynamic) -> dynamic
{
  return (((abs((s0 - p)) + abs((s1 - p))) - abs((s1 - s0))) < EPS);
}

func crosspoint(l0: dynamic, l1: dynamic, m0: dynamic, m1: dynamic) -> dynamic
{
  var a: dynamic = cross((l1 - l0), (m1 - m0));
  var b: dynamic = cross((l1 - l0), (l1 - m0));
  if (((abs(a) < EPS) && (abs(b) < EPS)))
  {
    return m0;
  }
  return (m0 + ((b / a) * ((m1 - m0))));
}

func convex_cut(v: dynamic, l0: dynamic, l1: dynamic) -> dynamic
{
  var r: dynamic = cpp_uninitialized();
  rep(i, v.size());
  {
    var a: dynamic = cpp_uninitialized();
    if ((ccw(l0, l1, a) != -1))
    {
      r.push_back(a);
    }
    if (((ccw(l0, l1, a) * ccw(l0, l1, b)) < 0))
    {
      r.push_back(crosspoint(l0, l1, a, b));
    }
  }
  return r;
}

func contains(ps: dynamic, p: dynamic) -> dynamic
{
  var in_cpp: dynamic = false;
  rep(i, ps.size());
  {
    var j: dynamic = (((i + 1)) % ps.size());
    var a: dynamic = cpp_construct((ps[i] - p));
    var b: dynamic = cpp_construct((ps[j] - p));
    if ((imag(a) > imag(b)))
    {
      swap(a, b);
    }
    if ((((imag(a) <= 0) && (0 < imag(b))) && (cross(a, b) < 0)))
    {
      in_cpp = (!in_cpp);
    }
    if (((cross(a, b) == 0) && (dot(a, b) <= 0)))
    {
      return true;
    }
  }
  return in_cpp;
}

func to_convex(v: dynamic) -> dynamic
{
  var r: dynamic = cpp_uninitialized();
  rep(i, v.size()).push_back(v[i].first);
  return r;
}

func index(v: dynamic, s0: dynamic, s1: dynamic) -> dynamic
{
  cpp_statement("const P& mid(0.5*(s0+s1)); rep(i, v.size())");
  {
    var j: dynamic = (((i + 1)) % v.size());
    if (intersectSP(v[i], v[j], mid))
    {
      return i;
    }
  }
  return -1;
}

func intersectCVCV(a: dynamic, b: dynamic) -> dynamic
{
  rep(i, a.size());
  if (contains(b, a[i]))
  {
    return true;
  }
  rep(i, b.size());
  if (contains(a, b[i]))
  {
    return true;
  }
  return false;
}

func turnover(vs: dynamic, p: dynamic, q: dynamic) -> dynamic
{
  var qu: dynamic = cpp_uninitialized();
  {
    var i: dynamic = (cpp_cast(vs.size()) - 1);
    while ((i >= 0))
    {
      if (contains(to_convex(vs[i]), p))
      {
        qu.push(i);
        break;
      }
      i -= 1;
    }
  }
  var m: dynamic = cpp_construct((0.5 * ((p + q))));
  var dir: dynamic = cpp_construct((((p - q)) * P(0, 1)));
  var is: dynamic = cpp_construct(vs.size(), 0);
  while ((!qu.empty()))
  {
    var ix: dynamic = qu.front();
    qu.pop();
    if (is[ix])
    {
      continue;
    }
    is[ix] = 1;
    var a: dynamic = cpp_construct(convex_cut(cv, m, (m - dir)));
    {
      var i: dynamic = (ix + 1);
      while ((i < cpp_cast(vs.size())))
      {
        if ((!is[i]))
        {
          if (intersectCVCV(a, to_convex(vs[i])))
          {
            qu.push(i);
          }
        }
        i += 1;
      }
    }
    rep(i1, a.size());
    {
      var i2: dynamic = (((i1 + 1)) % a.size());
      var k: dynamic = index(cv, a[i1], a[i2]);
      if (((k != -1) && (vs[ix][k].second != -1)))
      {
        qu.push(vs[ix][k].second);
      }
    }
  }
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  rep(k, vs.size());
  {
    if ((is[k] == 0))
    {
      var sg: dynamic = cpp_uninitialized();
      rep(i, vs[k].size());
      {
        sg.push_back(mp(vs[k][i].first, (vs[k][i].second + 1)));
      }
      s.push_back(mp((k + 1), sg));
      continue;
    }
    var a: dynamic = cpp_construct(convex_cut(cv, m, (m + dir)));
    if ((a.size() > 0))
    {
      var as_cpp: dynamic = cpp_uninitialized();
      rep(i, a.size());
      {
        var j: dynamic = (((i + 1)) % a.size());
        var ix: dynamic = index(cv, a[i], a[j]);
        as_cpp.push_back(mp(a[i],  ((ix == -1)) ? (-((k + 1))) : (vs[k][ix].second + 1)));
      }
      s.push_back(mp((k + 1), as_cpp));
    }
    var b: dynamic = cpp_construct(convex_cut(cv, m, (m - dir)));
    if ((b.size() > 0))
    {
      var bs: dynamic = cpp_uninitialized();
      rep(i, b.size());
      {
        var j: dynamic = (((i + 1)) % b.size());
        var ix: dynamic = index(cv, b[i], b[j]);
        var to: dynamic = ((2.0 * projection(m, (m + dir), b[i])) - b[i]);
        bs.push_back(mp(to,  ((ix == -1)) ? (k + 1) : (-((vs[k][ix].second + 1)))));
      }
      reverse(bs.begin(), bs.end());
      var x: dynamic = bs[0].second;
      rep(i, (bs.size() - 1))[i].second = bs[(i + 1)].second;
      bs.back().second = x;
      t.push_back(mp((k + 1), bs));
    }
  }
  var of: dynamic = cpp_uninitialized();
  rep(i, s.size())[s[i].first] = (i + 1);
  rep(i, t.size())[(-t[i].first)] = ((s.size() + t.size()) - i);
  var r: dynamic = cpp_uninitialized();
  rep(i, s.size()).push_back(s[i].second);
  rep(i, t.size()).push_back(t[((t.size() - 1) - i)].second);
  rep(i, r.size());
  rep(j, r[i].size())[i][j].second = (of[r[i][j].second] - 1);
  return r;
}

var N: dynamic = cpp_uninitialized();

var Px: dynamic = cpp_array(16);

var Py: dynamic = cpp_array(16);

var Qx: dynamic = cpp_array(16);

var Qy: dynamic = cpp_array(16);

var Hx: dynamic = cpp_uninitialized();

var Hy: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  {
    while (true)
    {
      scanf("%d", (&N));
      if ((N == 0))
      {
        return 0;
      }
      rep(i, N);
      scanf("%d%d%d%d", (Px + i), (Py + i), (Qx + i), (Qy + i));
      scanf("%d%d", (&Hx), (&Hy));
      var v: dynamic = cpp_uninitialized();
      v.push_back(mp(P(0, 0), -1));
      v.push_back(mp(P(100, 0), -1));
      v.push_back(mp(P(100, 100), -1));
      v.push_back(mp(P(0, 100), -1));
      var vs: dynamic = cpp_uninitialized();
      vs.push_back(v);
      rep(i, N) = turnover(vs, P(Px[i], Py[i]), P(Qx[i], Qy[i]));
      var ans: dynamic = 0;
      rep(i, vs.size());
      if (contains(to_convex(vs[i]), P(Hx, Hy)))
      {
        ans += 1;
      }
      printf("%d\n", ans);
    }
  }
}
