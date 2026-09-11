// Translated from solution.cpp.

func add(a: dynamic, b: dynamic) -> dynamic
{
  return  ((abs((a + b)) < ((1e-11) * ((abs(a) + abs(b)))))) ? 0.0 : (a + b);
}

class vec
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func operator_subtract(b: dynamic) -> dynamic
  {
      return [add(x, (-b.x)), add(y, (-b.y))];
    }
  func operator_add(b: dynamic) -> dynamic
  {
      return [add(x, b.x), add(y, b.y)];
    }
  func operator_multiply(d: dynamic) -> dynamic
  {
      return [(x * d), (y * d)];
    }
  func operator_equal(b: dynamic) -> dynamic
  {
      return ((x == b.x) && (y == b.y));
    }
  func operator_not_equal(b: dynamic) -> dynamic
  {
      return ((x != b.x) || (y != b.y));
    }
  func dot(v: dynamic) -> dynamic
  {
      return add((x * v.x), (y * v.y));
    }
  func cross(v: dynamic) -> dynamic
  {
      return add((x * v.y), ((-y) * v.x));
    }
  func norm() -> dynamic
  {
      return sqrt(((x * x) + (y * y)));
    }
}

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  var ab: dynamic = (b - a);
  var ac: dynamic = (c - a);
  var o: dynamic = ab.cross(ac);
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

func ternarysearch(p0: dynamic, p1: dynamic, p2: dynamic, tg: dynamic, tw: dynamic) -> dynamic
{
  var left: dynamic = (1.0 / 3.0);
  var right: dynamic = (2.0 / 3.0);
  var t: dynamic = cpp_uninitialized();
  var t1: dynamic = cpp_uninitialized();
  var t2: dynamic = cpp_uninitialized();
  var P1: dynamic = cpp_uninitialized();
  var P2: dynamic = cpp_uninitialized();
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
  var begin: dynamic = cpp_uninitialized();
  var end: dynamic = cpp_uninitialized();
  var P: dynamic = cpp_uninitialized();
  func Segment(a: dynamic, b: dynamic) -> dynamic
  {
      begin = a;
      end = b;
      P.push_back(a);
      P.push_back(b);
    }
}

class Edge
{
  var to: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  func Edge(a: dynamic, b: dynamic) -> dynamic
  {
      to = a;
      t = b;
    }
}

class Point
{
  var v: dynamic = cpp_uninitialized();
  var E: dynamic = cpp_uninitialized();
  func Point(v: dynamic) -> dynamic
  {
      self->v = v;
    }
}

func solve(v: dynamic, S: dynamic, T: dynamic, tg: dynamic, tw: dynamic) -> dynamic
{
  var n: dynamic = v.size();
  var V: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((((v[i] - S)).norm() < 1e-8))
      {
        V.push_back(Point(S));
        {
          var j: dynamic = (i + 1);
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
          var j: dynamic = 0;
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
  var L: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < V.size()))
    {
      L.push_back(Segment(i, (((i + 1)) % n)));
      i += 1;
    }
  }
  V.push_back(T);
  n = V.size();
  var iT: dynamic = (V.size() - 1);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      for (var s: dynamic in L)
      {
        if (((s.begin == i) || (s.end == i)))
        {
          continue;
        }
        var M: dynamic = cpp_uninitialized();
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
    var i: dynamic = 0;
    while ((i <= iT))
    {
      {
        var j: dynamic = (i + 2);
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
  for (var s: dynamic in L)
  {
    for (var p1: dynamic in s.P)
    {
      for (var p2: dynamic in s.P)
      {
        if ((p1 != p2))
        {
          V[p1].E.push_back(Edge(p2, (((V[p1].v - V[p2].v)).norm() * tg)));
          V[p2].E.push_back(Edge(p1, (((V[p1].v - V[p2].v)).norm() * tg)));
        }
      }
    }
  }
  var que: dynamic = cpp_uninitialized();
  var minimum_time: dynamic = cpp_array(V.size());
  fill(minimum_time, (minimum_time + V.size()), 1e10);
  var s: dynamic = 0;
  minimum_time[s] = 0;
  que.push(Q(0, s));
  while ((!que.empty()))
  {
    var q: dynamic = que.top();
    que.pop();
    var i: dynamic = q.second;
    if ((i == iT))
    {
      return minimum_time[i];
    }
    if ((minimum_time[i] < q.first))
    {
      continue;
    }
    for (var e: dynamic in V[i].E)
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

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var tg: dynamic = cpp_uninitialized();
  var tw: dynamic = cpp_uninitialized();
  var S: dynamic = cpp_uninitialized();
  var T: dynamic = cpp_uninitialized();
  var V: dynamic = cpp_uninitialized();
  while (cpp_comma((cin >> n), (n != 0)))
  {
    V.clear();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var x: dynamic = cpp_uninitialized();
        var y: dynamic = cpp_uninitialized();
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
