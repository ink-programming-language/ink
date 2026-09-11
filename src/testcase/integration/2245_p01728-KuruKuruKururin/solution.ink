// Translated from solution.cpp.

var EPS: dynamic = 1e-8;

var PI: dynamic = acos(-1);

func EQ(n: dynamic, m: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc+");
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

class C
{
  var p: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  func C(p: dynamic, r: dynamic) -> dynamic
  {
      self->p = cpp_construct(p);
      self->r = cpp_construct(r);
    }
  func C() -> dynamic
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
  if ((dot(b, c) < EPS))
  {
    return +2;
  }
  if ((dot(b, (b - c)) < EPS))
  {
    return -2;
  }
  return 0;
}

func unit(p: dynamic) -> dynamic
{
  return (p / abs(p));
}

func rotate(p: dynamic, rad: dynamic) -> dynamic
{
  return (p * P(cos(rad), sin(rad)));
}

func strictItsSS(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_binary((((ccw(a[0], a[1], b[0]) * ccw(a[0], a[1], b[1])) == -1)), "and", (((ccw(b[0], b[1], a[0]) * ccw(b[0], b[1], a[1])) == -1)));
}

func intersectSP(s: dynamic, p: dynamic) -> dynamic
{
  return cpp_binary((abs(cross((s[0] - p), (s[1] - p))) < EPS), "and", (dot((s[0] - p), (s[1] - p)) < EPS));
}

func projection(l: dynamic, p: dynamic) -> dynamic
{
  var t: dynamic = (dot((p - l[0]), (l[0] - l[1])) / norm((l[0] - l[1])));
  return (l[0] + (t * ((l[0] - l[1]))));
}

func distanceLP(l: dynamic, p: dynamic) -> dynamic
{
  return (abs(cross((l[1] - l[0]), (p - l[0]))) / abs((l[1] - l[0])));
}

func isParallel(a: dynamic, b: dynamic) -> dynamic
{
  return (abs(cross(a, b)) < EPS);
}

func isInConvex(p: dynamic, poly: dynamic) -> dynamic
{
  var n: dynamic = poly.size();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((cross((poly[(((i + 1)) % n)] - poly[i]), (p - poly[i])) < EPS))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func crosspointLL(l: dynamic, m: dynamic) -> dynamic
{
  var A: dynamic = cross((l[1] - l[0]), (m[1] - m[0]));
  var B: dynamic = cross((l[1] - l[0]), (l[1] - m[0]));
  return (m[0] + ((B / A) * ((m[1] - m[0]))));
}

func crosspointCL(c: dynamic, l: dynamic) -> dynamic
{
  var res: dynamic = cpp_uninitialized();
  var mid: dynamic = projection(l, c.p);
  var d: dynamic = distanceLP(l, c.p);
  if (EQ(d, c.r))
  {
    res.push_back(mid);
  } else if ((d < c.r))
  {
    var len: dynamic = sqrt(((c.r * c.r) - (d * d)));
    res.push_back((mid + (len * unit((l[1] - l[0])))));
    res.push_back((mid - (len * unit((l[1] - l[0])))));
  }
  return res;
}

func crosspointCC(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = cpp_uninitialized();
  if ((a.r < b.r))
  {
    swap(a, b);
  }
  var dist: dynamic = abs((b.p - a.p));
  var dir: dynamic = (a.r * unit((b.p - a.p)));
  if (cpp_binary(EQ(dist, (a.r + b.r)), "or", EQ(dist, (a.r - b.r))))
  {
    res.push_back((a.p + dir));
  } else if (cpp_binary(((a.r - b.r) < dist), "and", (dist < (a.r + b.r))))
  {
    var cos: dynamic = (((((a.r * a.r) + (dist * dist)) - (b.r * b.r))) / (((2 * a.r) * dist)));
    var sin: dynamic = sqrt((1 - (cos * cos)));
    res.push_back((a.p + (dir * P(cos, sin))));
    res.push_back((a.p + (dir * P(cos, (-sin)))));
  }
  return res;
}

class DA
{
  var c: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_array(2);
  func DA(c: dynamic, s: dynamic, t: dynamic) -> dynamic
  {
      self->c = cpp_construct(c);
      v[0] = s;
      v[1] = t;
    }
  func DA() -> dynamic
  {
    }
}

func crosspointSDA(l: dynamic, da: dynamic) -> dynamic
{
  var cp: dynamic = crosspointCL(da.c, l);
  var res: dynamic = cpp_uninitialized();
  for (var p: dynamic in cp)
  {
    var v: dynamic = (p - da.c.p);
    if (cpp_binary(intersectSP(l, p), "and", ((cross(v, da.v[0]) * cross(v, da.v[1])) < EPS)))
    {
      res.push_back(p);
    }
  }
  return res;
}

func input_P(in_cpp: dynamic) -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(x, y);
  in_cpp = P(x, y);
}

func make_rect(l: dynamic, e: dynamic, d: dynamic) -> dynamic
{
  return [((l[0] - e) + d), ((l[0] - e) - d), ((l[1] + e) - d), ((l[1] + e) + d)];
}

func insertvec(a: dynamic, b: dynamic) -> dynamic
{
  a.insert(a.end(), b.begin(), b.end());
}

var weps: dynamic = 1e-5;

var rot: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var g: dynamic = cpp_uninitialized();

var dir: dynamic = cpp_uninitialized();

var wall: dynamic = cpp_uninitialized();

var slide_rect: dynamic = cpp_uninitialized();

var nonrotate_rect: dynamic = cpp_uninitialized();

var nonrotate_da: dynamic = cpp_uninitialized();

var cand_rot: dynamic = cpp_uninitialized();

var graph: dynamic = cpp_uninitialized();

func input() -> dynamic
{
  read(l, rot);
  l += weps;
  input_P(s);
  input_P(g);
  read(n);
  wall.resize(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      for (var p: dynamic in wall[i])
      {
        input_P(p);
      }
      i += 1;
    }
  }
}

func misc() -> dynamic
{
  dir.resize((rot + 1));
  {
    var i: dynamic = 0;
    while ((i < (rot + 1)))
    {
      dir[i] = rotate(P(l, 0), ((PI / rot) * i));
      i += 1;
    }
  }
}

func make_slide_rect() -> dynamic
{
  slide_rect.resize(rot);
  {
    var r: dynamic = 0;
    while ((r < rot))
    {
      for (var w: dynamic in wall)
      {
        var vw: dynamic = (w[1] - w[0]);
        if (isParallel(vw, dir[r]))
        {
          slide_rect[r].push_back(make_rect(w, (l * unit(vw)), (weps * unit(rotate(vw, (PI / 2))))));
        } else
        {
          slide_rect[r].push_back(make_rect(w, (weps * unit(vw)),  (((cross(vw, dir[r]) > 0))) ? dir[r] : (-dir[r])));
        }
      }
      r += 1;
    }
  }
}

func make_nonrot_area() -> dynamic
{
  nonrotate_rect.resize(rot);
  nonrotate_da.resize(rot);
  {
    var r: dynamic = 0;
    while ((r < rot))
    {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          nonrotate_rect[r].push_back(slide_rect[r][i]);
          nonrotate_rect[r].push_back(slide_rect[(((r + 1)) % rot)][i]);
          var vert: dynamic = rotate((wall[i][1] - wall[i][0]), (PI / 2));
          if (((cross(vert, dir[r]) * cross(vert, dir[(r + 1)])) < (-EPS)))
          {
            nonrotate_rect[r].push_back(make_rect(wall[i], P(0, 0), (l * unit(vert))));
          }
          nonrotate_da[r].emplace_back(C(wall[i][0], l), dir[r], dir[(r + 1)]);
          nonrotate_da[r].emplace_back(C(wall[i][1], l), dir[r], dir[(r + 1)]);
          i += 1;
        }
      }
      r += 1;
    }
  }
}

func enum_rotate_candidate() -> dynamic
{
  cand_rot.resize(rot);
  {
    var r: dynamic = 0;
    while ((r < rot))
    {
      var w: dynamic = cpp_uninitialized();
      for (var v: dynamic in nonrotate_rect[r])
      {
        var n: dynamic = v.size();
        {
          var i: dynamic = 0;
          while ((i < n))
          {
            w.emplace_back(v[i], v[(((i + 1)) % n)]);
            i += 1;
          }
        }
      }
      var arc: dynamic = nonrotate_da[r];
      var cand: dynamic = cand_rot[r];
      var n: dynamic = w.size();
      var m: dynamic = arc.size();
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          {
            var j: dynamic = (i + 1);
            while ((j < n))
            {
              if (strictItsSS(w[i], w[j]))
              {
                cand.push_back(crosspointLL(w[i], w[j]));
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          {
            var j: dynamic = 0;
            while ((j < m))
            {
              insertvec(cand, crosspointSDA(w[i], arc[j]));
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
          {
            var j: dynamic = (i + 1);
            while ((j < m))
            {
              insertvec(cand, crosspointCC(arc[i].c, arc[j].c));
              j += 1;
            }
          }
          i += 1;
        }
      }
      sort(cand.begin(), cand.end());
      cand.erase(unique(cand.begin(), cand.end()), cand.end());
      r += 1;
    }
  }
}

func eliminate_nonrot_point() -> dynamic
{
  {
    var r: dynamic = 0;
    while ((r < rot))
    {
      var cand: dynamic = cand_rot[r];
      for (var p: dynamic in cand)
      {
        var enable: dynamic = true;
        for (var rect: dynamic in nonrotate_rect[r])
        {
          if (isInConvex(p, rect))
          {
            enable = false;
          }
        }
        for (var da: dynamic in nonrotate_da[r])
        {
          if (cpp_binary((((abs((p - da.c.p)) + EPS) < l)), "and", (((cross((p - da.c.p), da.v[0]) * cross((p - da.c.p), da.v[1])) < EPS))))
          {
            enable = false;
          }
        }
        if (enable)
        {
          res[r].push_back(p);
        }
      }
      r += 1;
    }
  }
  cand_rot = res;
}

func cansee(s: dynamic, g: dynamic, rects: dynamic) -> dynamic
{
  var mid: dynamic = (((s + g)) / 2.0);
  for (var rect: dynamic in rects)
  {
    if (isInConvex(mid, rect))
    {
      return false;
    }
    {
      var i: dynamic = 0;
      while ((i < 4))
      {
        if (strictItsSS(L(rect[i], rect[(((i + 1)) % 4)]), L(s, g)))
        {
          return false;
        }
        i += 1;
      }
    }
  }
  return true;
}

func make_visible_graph() -> dynamic
{
  graph = vector(rot);
  var vlist: dynamic = cpp_construct(rot, [s, g]);
  {
    var r: dynamic = 0;
    while ((r < rot))
    {
      var v: dynamic = vlist[r];
      for (var rect: dynamic in slide_rect[r])
      {
        insertvec(v, rect);
      }
      insertvec(v, cand_rot[r]);
      insertvec(v, cand_rot[((((r - 1) + rot)) % rot)]);
      r += 1;
    }
  }
  {
    var r: dynamic = 0;
    while ((r < rot))
    {
      var v: dynamic = vlist[r];
      graph[r].resize(v.size());
      {
        var i: dynamic = 0;
        while ((i < v.size()))
        {
          {
            var j: dynamic = (i + 1);
            while ((j < v.size()))
            {
              if (cansee(v[i], v[j], slide_rect[r]))
              {
                graph[r][i].emplace_back(r, j);
                graph[r][j].emplace_back(r, i);
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < cand_rot[r].size()))
        {
          graph[r][((i + (4 * n)) + 2)].emplace_back((((r + 1)) % rot), (((i + (4 * n)) + 2) + cand_rot[(((r + 1)) % rot)].size()));
          i += 1;
        }
      }
      r += 1;
    }
  }
}

func shortest_path() -> dynamic
{
  var inf: dynamic = 1e9;
  var wait: dynamic = cpp_uninitialized();
  var mincost: dynamic = cpp_construct(rot, vector(10000, inf));
  mincost[0][0] = 0;
  wait.push_front(pii(0, 0));
  while ((!wait.empty()))
  {
    var curr: dynamic = wait.front();
    wait.pop_front();
    var cr: dynamic = curr.first;
    var ci: dynamic = curr.second;
    if ((ci == 1))
    {
      return mincost[cr][ci];
    }
    for (var next: dynamic in graph[cr][ci])
    {
      var nr: dynamic = next.first;
      var ni: dynamic = next.second;
      var cost: dynamic =  (((cr == nr))) ? 0 : 1;
      if (((mincost[cr][ci] + cost) < mincost[nr][ni]))
      {
        mincost[nr][ni] = (mincost[cr][ci] + cost);
        if ((cost == 0))
        {
          wait.push_front(pii(nr, ni));
        } else
        {
          wait.push_back(pii(nr, ni));
        }
      }
    }
  }
  return -1;
}

func main() -> dynamic
{
  input();
  misc();
  make_slide_rect();
  make_nonrot_area();
  enum_rotate_candidate();
  eliminate_nonrot_point();
  make_visible_graph();
  write(shortest_path(), "\n");
  return 0;
}
