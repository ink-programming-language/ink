// Translated from solution.cpp.

var EPS = 1e-8;

var PI = acos(-1);

func EQ(n: dynamic, m: dynamic)
{
  return cpp_expression("#include <bits/stdc+");
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

class C
{
  var p: dynamic;
  var r: dynamic;
  func C(p: dynamic, r: dynamic)
  {
      this->p = cpp_construct(p);
      this->r = cpp_construct(r);
    }
  func C()
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

func unit(p: dynamic)
{
  return (p / abs(p));
}

func rotate(p: dynamic, rad: dynamic)
{
  return (p * P(cos(rad), sin(rad)));
}

func strictItsSS(a: dynamic, b: dynamic)
{
  return cpp_binary((((ccw(a[0], a[1], b[0]) * ccw(a[0], a[1], b[1])) == -1)), "and", (((ccw(b[0], b[1], a[0]) * ccw(b[0], b[1], a[1])) == -1)));
}

func intersectSP(s: dynamic, p: dynamic)
{
  return cpp_binary((abs(cross((s[0] - p), (s[1] - p))) < EPS), "and", (dot((s[0] - p), (s[1] - p)) < EPS));
}

func projection(l: dynamic, p: dynamic)
{
  var t = (dot((p - l[0]), (l[0] - l[1])) / norm((l[0] - l[1])));
  return (l[0] + (t * ((l[0] - l[1]))));
}

func distanceLP(l: dynamic, p: dynamic)
{
  return (abs(cross((l[1] - l[0]), (p - l[0]))) / abs((l[1] - l[0])));
}

func isParallel(a: dynamic, b: dynamic)
{
  return (abs(cross(a, b)) < EPS);
}

func isInConvex(p: dynamic, poly: dynamic)
{
  var n = poly.size();
  {
    var i = 0;
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

func crosspointLL(l: dynamic, m: dynamic)
{
  var A = cross((l[1] - l[0]), (m[1] - m[0]));
  var B = cross((l[1] - l[0]), (l[1] - m[0]));
  return (m[0] + ((B / A) * ((m[1] - m[0]))));
}

func crosspointCL(c: dynamic, l: dynamic)
{
  var res: dynamic;
  var mid = projection(l, c.p);
  var d = distanceLP(l, c.p);
  if (EQ(d, c.r))
  {
    res.push_back(mid);
  } else if ((d < c.r))
  {
    var len = sqrt(((c.r * c.r) - (d * d)));
    res.push_back((mid + (len * unit((l[1] - l[0])))));
    res.push_back((mid - (len * unit((l[1] - l[0])))));
  }
  return res;
}

func crosspointCC(a: dynamic, b: dynamic)
{
  var res: dynamic;
  if ((a.r < b.r))
  {
    swap(a, b);
  }
  var dist = abs((b.p - a.p));
  var dir = (a.r * unit((b.p - a.p)));
  if (cpp_binary(EQ(dist, (a.r + b.r)), "or", EQ(dist, (a.r - b.r))))
  {
    res.push_back((a.p + dir));
  } else if (cpp_binary(((a.r - b.r) < dist), "and", (dist < (a.r + b.r))))
  {
    var cos = (((((a.r * a.r) + (dist * dist)) - (b.r * b.r))) / (((2 * a.r) * dist)));
    var sin = sqrt((1 - (cos * cos)));
    res.push_back((a.p + (dir * P(cos, sin))));
    res.push_back((a.p + (dir * P(cos, (-sin)))));
  }
  return res;
}

class DA
{
  var c: dynamic;
  var v: dynamic = cpp_array(2);
  func DA(c: dynamic, s: dynamic, t: dynamic)
  {
      this->c = cpp_construct(c);
      v[0] = s;
      v[1] = t;
    }
  func DA()
  {
    }
}

func crosspointSDA(l: dynamic, da: dynamic)
{
  var cp = crosspointCL(da.c, l);
  var res: dynamic;
  for (var p in cp)
  {
    var v = (p - da.c.p);
    if (cpp_binary(intersectSP(l, p), "and", ((cross(v, da.v[0]) * cross(v, da.v[1])) < EPS)))
    {
      res.push_back(p);
    }
  }
  return res;
}

func input_P(in_cpp: dynamic)
{
  var x: dynamic;
  var y: dynamic;
  read(x, y);
  in_cpp = P(x, y);
}

func make_rect(l: dynamic, e: dynamic, d: dynamic)
{
  return [((l[0] - e) + d), ((l[0] - e) - d), ((l[1] + e) - d), ((l[1] + e) + d)];
}

func insertvec(a: dynamic, b: dynamic)
{
  a.insert(a.end(), b.begin(), b.end());
}

var weps = 1e-5;

var rot: dynamic;

var n: dynamic;

var l: dynamic;

var s: dynamic;

var g: dynamic;

var dir: dynamic;

var wall: dynamic;

var slide_rect: dynamic;

var nonrotate_rect: dynamic;

var nonrotate_da: dynamic;

var cand_rot: dynamic;

var graph: dynamic;

func input()
{
  read(l, rot);
  l += weps;
  input_P(s);
  input_P(g);
  read(n);
  wall.resize(n);
  {
    var i = 0;
    while ((i < n))
    {
      for (var p in wall[i])
      {
        input_P(p);
      }
      i += 1;
    }
  }
}

func misc()
{
  dir.resize((rot + 1));
  {
    var i = 0;
    while ((i < (rot + 1)))
    {
      dir[i] = rotate(P(l, 0), ((PI / rot) * i));
      i += 1;
    }
  }
}

func make_slide_rect()
{
  slide_rect.resize(rot);
  {
    var r = 0;
    while ((r < rot))
    {
      for (var w in wall)
      {
        var vw = (w[1] - w[0]);
        if (isParallel(vw, dir[r]))
        {
          slide_rect[r].push_back(make_rect(w, (l * unit(vw)), (weps * unit(rotate(vw, (PI / 2))))));
        } else
        {
          slide_rect[r].push_back(make_rect(w, (weps * unit(vw)), if (((cross(vw, dir[r]) > 0))) dir[r] else (-dir[r])));
        }
      }
      r += 1;
    }
  }
}

func make_nonrot_area()
{
  nonrotate_rect.resize(rot);
  nonrotate_da.resize(rot);
  {
    var r = 0;
    while ((r < rot))
    {
      {
        var i = 0;
        while ((i < n))
        {
          nonrotate_rect[r].push_back(slide_rect[r][i]);
          nonrotate_rect[r].push_back(slide_rect[(((r + 1)) % rot)][i]);
          var vert = rotate((wall[i][1] - wall[i][0]), (PI / 2));
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

func enum_rotate_candidate()
{
  cand_rot.resize(rot);
  {
    var r = 0;
    while ((r < rot))
    {
      var w: dynamic;
      for (var v in nonrotate_rect[r])
      {
        var n = v.size();
        {
          var i = 0;
          while ((i < n))
          {
            w.emplace_back(v[i], v[(((i + 1)) % n)]);
            i += 1;
          }
        }
      }
      var arc = nonrotate_da[r];
      var cand = cand_rot[r];
      var n = w.size();
      var m = arc.size();
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = (i + 1);
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
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
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
        var i = 0;
        while ((i < m))
        {
          {
            var j = (i + 1);
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

func eliminate_nonrot_point()
{
  {
    var r = 0;
    while ((r < rot))
    {
      var cand = cand_rot[r];
      for (var p in cand)
      {
        var enable = true;
        for (var rect in nonrotate_rect[r])
        {
          if (isInConvex(p, rect))
          {
            enable = false;
          }
        }
        for (var da in nonrotate_da[r])
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

func cansee(s: dynamic, g: dynamic, rects: dynamic)
{
  var mid = (((s + g)) / 2.0);
  for (var rect in rects)
  {
    if (isInConvex(mid, rect))
    {
      return false;
    }
    {
      var i = 0;
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

func make_visible_graph()
{
  graph = vector(rot);
  var vlist = cpp_construct(rot, [s, g]);
  {
    var r = 0;
    while ((r < rot))
    {
      var v = vlist[r];
      for (var rect in slide_rect[r])
      {
        insertvec(v, rect);
      }
      insertvec(v, cand_rot[r]);
      insertvec(v, cand_rot[((((r - 1) + rot)) % rot)]);
      r += 1;
    }
  }
  {
    var r = 0;
    while ((r < rot))
    {
      var v = vlist[r];
      graph[r].resize(v.size());
      {
        var i = 0;
        while ((i < v.size()))
        {
          {
            var j = (i + 1);
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
        var i = 0;
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

func shortest_path()
{
  var inf = 1e9;
  var wait: dynamic;
  var mincost = cpp_construct(rot, vector(10000, inf));
  mincost[0][0] = 0;
  wait.push_front(pii(0, 0));
  while ((!wait.empty()))
  {
    var curr = wait.front();
    wait.pop_front();
    var cr = curr.first;
    var ci = curr.second;
    if ((ci == 1))
    {
      return mincost[cr][ci];
    }
    for (var next in graph[cr][ci])
    {
      var nr = next.first;
      var ni = next.second;
      var cost = if (((cr == nr))) 0 else 1;
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

func main()
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
