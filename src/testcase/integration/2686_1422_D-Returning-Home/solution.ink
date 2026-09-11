// Translated from solution.cpp.

var MAX_M: dynamic = (1e5 + 5);

class Point
{
  func Point() -> dynamic
  {
    }
  func Point(x: dynamic, y: dynamic, id: dynamic) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
      self->id = cpp_construct(id);
    }
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

class Edge
{
  func Edge(from_cpp: dynamic, to: dynamic, w: dynamic) -> dynamic
  {
      self->from_cpp = cpp_construct(from_cpp);
      self->to = cpp_construct(to);
      self->w = cpp_construct(w);
    }
  var from_cpp: dynamic = cpp_uninitialized();
  var to: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var g: dynamic = cpp_array(MAX_M);

var pts: dynamic = cpp_array(MAX_M);

func manhattan(i: dynamic, j: dynamic) -> dynamic
{
  return (abs((pts[i].x - pts[j].x)) + abs((pts[i].y - pts[j].y)));
}

func cost(i: dynamic, j: dynamic) -> dynamic
{
  return min(abs((pts[i].x - pts[j].x)), abs((pts[i].y - pts[j].y)));
}

func dijkstra(s: dynamic, t: dynamic) -> dynamic
{
  var dist: dynamic = cpp_construct((m + 2), -1);
  var pq: dynamic = cpp_uninitialized();
  dist[s] = 0;
  pq.emplace(0, s);
  while ((!pq.empty()))
  {
    var d: dynamic = cpp_uninitialized();
    var cur: dynamic = cpp_uninitialized();
    tie(d, cur) = pq.top();
    pq.pop();
    for (var ed: dynamic in g[cur])
    {
      if (((dist[ed.to] == -1) || ((d + ed.w) < dist[ed.to])))
      {
        dist[ed.to] = (d + ed.w);
        pq.emplace(dist[ed.to], ed.to);
      }
    }
  }
  return dist;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  read(n, m);
  read(pts[0].x, pts[0].y);
  pts[0].id = 0;
  read(pts[1].x, pts[1].y);
  pts[1].id = 1;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(pts[(2 + i)].x, pts[(2 + i)].y);
      pts[(2 + i)].id = (2 + i);
      i += 1;
    }
  }
  g[0].emplace_back(0, 1, manhattan(0, 1));
  {
    var i: dynamic = 2;
    while ((i < (2 + m)))
    {
      g[0].emplace_back(0, i, cost(0, i));
      g[i].emplace_back(i, 1, manhattan(i, 1));
      i += 1;
    }
  }
  sort((pts + 2), ((pts + 2) + m), __cpp_lambda_1);
  {
    var i: dynamic = 2;
    while ((i < (m + 2)))
    {
      if ((i > 2))
      {
        g[pts[i].id].emplace_back(pts[i].id, pts[(i - 1)].id, cost(i, (i - 1)));
      }
      if (((i + 1) < (m + 2)))
      {
        g[pts[i].id].emplace_back(pts[i].id, pts[(i + 1)].id, cost(i, (i + 1)));
      }
      i += 1;
    }
  }
  sort((pts + 2), ((pts + 2) + m), __cpp_lambda_2);
  {
    var i: dynamic = 2;
    while ((i < (m + 2)))
    {
      if ((i > 2))
      {
        g[pts[i].id].emplace_back(pts[i].id, pts[(i - 1)].id, cost(i, (i - 1)));
      }
      if (((i + 1) < (m + 2)))
      {
        g[pts[i].id].emplace_back(pts[i].id, pts[(i + 1)].id, cost(i, (i + 1)));
      }
      i += 1;
    }
  }
  sort((pts + 2), ((pts + 2) + m), __cpp_lambda_3);
  write(dijkstra(0, 1)[1], "\n");
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  return (a.x < b.x);
}

func __cpp_lambda_2(a: dynamic, b: dynamic) -> dynamic
{
  return (a.y < b.y);
}

func __cpp_lambda_3(a: dynamic, b: dynamic) -> dynamic
{
  return (a.id < b.id);
}
