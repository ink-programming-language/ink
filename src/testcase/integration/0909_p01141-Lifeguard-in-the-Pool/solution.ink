// Translated from solution.cpp.

func add(a: dynamic, b: dynamic)
{
  return if ((abs((a + b)) < ((1e-11) * ((abs(a) + abs(b)))))) 0.0 else (a + b);
}

class vec
{
  var x: dynamic;
  var y: dynamic;
  func operator_subtract(b: dynamic)
  {
      return [add(x, (-b.x)), add(y, (-b.y))];
    }
  func operator_add(b: dynamic)
  {
      return [add(x, b.x), add(y, b.y)];
    }
  func operator_multiply(d: dynamic)
  {
      return [(x * d), (y * d)];
    }
  func operator_equal(b: dynamic)
  {
      return ((x == b.x) && (y == b.y));
    }
  func operator_not_equal(b: dynamic)
  {
      return ((x != b.x) || (y != b.y));
    }
  func dot(v: dynamic)
  {
      return add((x * v.x), (y * v.y));
    }
  func cross(v: dynamic)
  {
      return add((x * v.y), ((-y) * v.x));
    }
  func norm()
  {
      return sqrt(((x * x) + (y * y)));
    }
}

func ccw(a: dynamic, b: dynamic, c: dynamic)
{
  var ab = (b - a);
  var ac = (c - a);
  var o = ab.cross(ac);
  if ((o > 0))
  {
    return 1;
  }
  if ((o < 0))
  {
    return -1;
  }
  if ((ab.dot(ac) < 0))
  {
    return 2;
  } else
  {
    if ((ab.dot(ab) < ac.dot(ac)))
    {
      return -2;
    } else
    {
      return 0;
    }
  }
}

func ternarysearch(p0: dynamic, p1: dynamic, p2: dynamic, tg: dynamic, tw: dynamic)
{
  var left = (1.0 / 3.0);
  var right = (2.0 / 3.0);
  var t: dynamic;
  var t1: dynamic;
  var t2: dynamic;
  var P1: dynamic;
  var P2: dynamic;
  while (true)
  {
    P1 = ((p1 * ((1 - left))) + (p2 * left));
    P2 = ((p1 * ((1 - right))) + (p2 * right));
    t1 = ((((p0 - P1)).norm() * tw) + (((p2 - P1)).norm() * tg));
    t2 = ((((p0 - P2)).norm() * tw) + (((p2 - P2)).norm() * tg));
    if ((abs((t1 - t2)) < 1e-11))
    {
      t = (((left + right)) / 2);
      if (((1.0 - t) < 1e-8))
      {
        return p2;
      } else if ((t < 1e-8))
      {
        return p1;
      } else
      {
        return P1;
      }
    }
    if ((t1 < t2))
    {
      right = ((((2 * left) + right)) / 3);
      left = ((2 * left) - right);
    } else
    {
      left = ((((2 * right) + left)) / 3);
      right = ((2 * right) - left);
    }
  }
}

class Segment
{
  var begin: dynamic;
  var end: dynamic;
  var P: dynamic;
  func Segment(a: dynamic, b: dynamic)
  {
      begin = a;
      end = b;
      P.push_back(a);
      P.push_back(b);
    }
}

class Edge
{
  var to: dynamic;
  var t: dynamic;
  func Edge(a: dynamic, b: dynamic)
  {
      to = a;
      t = b;
    }
}

class Point
{
  var v: dynamic;
  var E: dynamic;
  func Point(v: dynamic)
  {
      this->v = v;
    }
}

func solve(v: dynamic, S: dynamic, T: dynamic, tg: dynamic, tw: dynamic)
{
  var n = v.size();
  var V: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      if ((((v[i] - S)).norm() < 1e-8))
      {
        V.push_back(Point(S));
        {
          var j = (i + 1);
          while ((j != i))
          {
            V.push_back(Point(v[j]));
            j = (((j + 1)) % n);
          }
        }
        break;
      } else if ((!ccw(v[i], v[(((i + 1)) % n)], S)))
      {
        V.push_back(Point(S));
        i = (((i + 1)) % n);
        {
          var j = 0;
          while ((j < n))
          {
            V.push_back(Point(v[(((i + j)) % n)]));
            j += 1;
          }
        }
        break;
      }
      i += 1;
    }
  }
  n = V.size();
  var L: dynamic;
  {
    var i = 0;
    while ((i < V.size()))
    {
      L.push_back(Segment(i, (((i + 1)) % n)));
      i += 1;
    }
  }
  V.push_back(T);
  n = V.size();
  var iT = (V.size() - 1);
  {
    var i = 0;
    while ((i < n))
    {
      for (var s in L)
      {
        if (((s.begin == i) || (s.end == i)))
        {
          continue;
        }
        var M: dynamic;
        M = ternarysearch(V[i].v, V[s.begin].v, V[s.end].v, tg, tw);
        if (((M != V[s.begin].v) && (M != V[s.end].v)))
        {
          V.push_back(M);
          V.back().E.push_back(Edge(i, (((V[i].v - M)).norm() * tw)));
          V[i].E.push_back(Edge((V.size() - 1), (((V[i].v - M)).norm() * tw)));
          s.P.push_back((V.size() - 1));
        }
        M = ternarysearch(V[i].v, V[s.end].v, V[s.begin].v, tg, tw);
        if (((M != V[s.begin].v) && (M != V[s.end].v)))
        {
          V.push_back(M);
          V.back().E.push_back(Edge(i, (((V[i].v - M)).norm() * tw)));
          V[i].E.push_back(Edge((V.size() - 1), (((V[i].v - M)).norm() * tw)));
          s.P.push_back((V.size() - 1));
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= iT))
    {
      {
        var j = (i + 2);
        while ((j <= iT))
        {
          V[i].E.push_back(Edge(j, (((V[i].v - V[j].v)).norm() * tw)));
          V[j].E.push_back(Edge(i, (((V[i].v - V[j].v)).norm() * tw)));
          j += 1;
        }
      }
      i += 1;
    }
  }
  for (var s in L)
  {
    for (var p1 in s.P)
    {
      for (var p2 in s.P)
      {
        if ((p1 != p2))
        {
          V[p1].E.push_back(Edge(p2, (((V[p1].v - V[p2].v)).norm() * tg)));
          V[p2].E.push_back(Edge(p1, (((V[p1].v - V[p2].v)).norm() * tg)));
        }
      }
    }
  }
  var que: dynamic;
  var minimum_time = cpp_array(V.size());
  fill(minimum_time, (minimum_time + V.size()), 1e10);
  var s = 0;
  minimum_time[s] = 0;
  que.push(Q(0, s));
  while ((!que.empty()))
  {
    var q = que.top();
    que.pop();
    var i = q.second;
    if ((i == iT))
    {
      return minimum_time[i];
    }
    if ((minimum_time[i] < q.first))
    {
      continue;
    }
    for (var e in V[i].E)
    {
      if ((minimum_time[e.to] > (minimum_time[i] + e.t)))
      {
        minimum_time[e.to] = (minimum_time[i] + e.t);
        que.push(Q(minimum_time[e.to], e.to));
      }
    }
  }
  return 0;
}

func main()
{
  var n: dynamic;
  var tg: dynamic;
  var tw: dynamic;
  var S: dynamic;
  var T: dynamic;
  var V: dynamic;
  while (cpp_comma((cin >> n), (n != 0)))
  {
    V.clear();
    {
      var i = 0;
      while ((i < n))
      {
        var x: dynamic;
        var y: dynamic;
        read(x, y);
        V.push_back([cpp_expression(".x=x"), cpp_expression(".y=y")]);
        i += 1;
      }
    }
    read(tg, tw);
    read(S.x, S.y);
    read(T.x, T.y);
    cout.precision(8);
    write(fixed);
    write(solve(V, S, T, tg, tw), "\n");
  }
  return 0;
}
