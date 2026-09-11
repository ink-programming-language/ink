// Translated from solution.cpp.

func REP(i: dynamic, s: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=s;i<n;i++)");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include<i");
}

var inf: dynamic = cpp_expression("#includ");

var EPS: dynamic = cpp_expression("#includ");

var COUNTER_CLOCKWISE: dynamic = cpp_expression("#");

var CLOCKWISE: dynamic = cpp_expression("#i");

var ONLINE_BACK: dynamic = cpp_expression("#");

var ONLINE_FRONT: dynamic = cpp_expression("#i");

var ON_SEGMENT: dynamic = cpp_expression("#");

var MAX: dynamic = cpp_expression("#in");

func equals(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream> #i");
}

class P
{
  var to: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
  func P(to: dynamic = (-inf), cost: dynamic = (-inf)) -> dynamic
  {
      self->to = cpp_construct(to);
      self->cost = cpp_construct(cost);
    }
  func operator_less(a: dynamic) -> dynamic
  {
      return (cost > a.cost);
    }
}

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Point(x: dynamic = (-inf), y: dynamic = (-inf)) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
    }
  func operator_add(p: dynamic) -> dynamic
  {
      return Point((x + p.x), (y + p.y));
    }
  func operator_subtract(p: dynamic) -> dynamic
  {
      return Point((x - p.x), (y - p.y));
    }
  func operator_multiply(a: dynamic) -> dynamic
  {
      return Point((a * x), (a * y));
    }
  func operator_divide(a: dynamic) -> dynamic
  {
      return Point((x / a), (y / a));
    }
  func operator_less(p: dynamic) -> dynamic
  {
      return  ((x != p.x)) ? (x < p.x) : (y < p.y);
    }
  func operator_equal(p: dynamic) -> dynamic
  {
      return ((fabs((x - p.x)) < EPS) && (fabs((y - p.y)) < EPS));
    }
}

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  (((((((os << setiosflags(ios.fixed)) << setprecision(5)) << "(") << p.x) << ",") << p.y) << ")");
}

class Segment
{
  var p1: dynamic = cpp_uninitialized();
  var p2: dynamic = cpp_uninitialized();
  func Segment(p1: dynamic = Point(-1, -1), p2: dynamic = Point(-1, -1)) -> dynamic
  {
      self->p1 = cpp_construct(p1);
      self->p2 = cpp_construct(p2);
    }
}

class Circle
{
  var p: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  func Circle(p: dynamic = Point(), r: dynamic = (-inf)) -> dynamic
  {
      self->p = cpp_construct(p);
      self->r = cpp_construct(r);
    }
}

class Illuminant
{
  var r: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var theta: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var maxpower: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  func Illuminant(p: dynamic = Point(), r: dynamic = (-inf), theta: dynamic = (-inf), a: dynamic = (-inf), b: dynamic = (-inf), maxpower: dynamic = (-inf)) -> dynamic
  {
      self->p = cpp_construct(p);
      self->r = cpp_construct(r);
      self->theta = cpp_construct(theta);
      self->a = cpp_construct(a);
      self->b = cpp_construct(b);
      self->maxpower = cpp_construct(maxpower);
    }
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.x) + (a.y * b.y));
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.y) - (a.y * b.x));
}

func norm(a: dynamic) -> dynamic
{
  return ((a.x * a.x) + (a.y * a.y));
}

func toRad(agl: dynamic) -> dynamic
{
  return (((agl * M_PI) / 180.0));
}

func rotate(a: dynamic, rad: dynamic) -> dynamic
{
  return Point(((cos(rad) * a.x) - (sin(rad) * a.y)), ((sin(rad) * a.x) + (cos(rad) * a.y)));
}

func ccw(p0: dynamic, p1: dynamic, p2: dynamic) -> dynamic
{
  var a: dynamic = (p1 - p0);
  var b: dynamic = (p2 - p0);
  if ((cross(a, b) > EPS))
  {
    return COUNTER_CLOCKWISE;
  }
  if ((cross(a, b) < (-EPS)))
  {
    return CLOCKWISE;
  }
  if ((dot(a, b) < (-EPS)))
  {
    return ONLINE_BACK;
  }
  if ((norm(a) < norm(b)))
  {
    return ONLINE_FRONT;
  }
  return ON_SEGMENT;
}

func abs(a: dynamic) -> dynamic
{
  return sqrt(norm(a));
}

func getDistanceLP(s: dynamic, p: dynamic) -> dynamic
{
  return (abs(cross((s.p2 - s.p1), (p - s.p1))) / abs((s.p2 - s.p1)));
}

func getDistanceSP(s: dynamic, p: dynamic) -> dynamic
{
  if ((dot((s.p2 - s.p1), (p - s.p1)) < 0.0))
  {
    return abs((p - s.p1));
  }
  if ((dot((s.p1 - s.p2), (p - s.p2)) < 0.0))
  {
    return abs((p - s.p2));
  }
  return getDistanceLP(s, p);
}

func angle(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  var ux: dynamic = (b.x - a.x);
  var uy: dynamic = (b.y - a.y);
  var vx: dynamic = (c.x - a.x);
  var vy: dynamic = (c.y - a.y);
  return acos(((((ux * vx) + (uy * vy))) / sqrt(((((ux * ux) + (uy * uy))) * (((vx * vx) + (vy * vy)))))));
}

func visit(G: dynamic, v: dynamic, order: dynamic, color: dynamic) -> dynamic
{
  color[v] = 1;
  {
    typeof((G[v]).begin()) = G[v].begin();
    while ((e != G[v].end()))
    {
      if ((color[e->to] == 2))
      {
        e += 1;
        continue;
      }
      if ((color[e->to] == 1))
      {
        return false;
      }
      if ((!visit(G, e->to, order, color)))
      {
        return false;
      }
      e += 1;
    }
  }
  order.push_back(v);
  color[v] = 2;
  return true;
}

func topologicalSort(G: dynamic, order: dynamic) -> dynamic
{
  var SizeG: dynamic = G.size();
  {
    var u: dynamic = 0;
    while ((u < SizeG))
    {
      if (((!color[u]) && (!visit(G, u, order, color))))
      {
        return false;
      }
      u += 1;
    }
  }
  reverse(order.begin(), order.end());
  return true;
}

var n: dynamic = cpp_uninitialized();

var illuminants: dynamic = cpp_array(MAX);

var graph: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(MAX);

func check(ill: dynamic, c: dynamic) -> dynamic
{
  var L: dynamic = Point((ill.p.x + (ill.r * cos(toRad((ill.b - (ill.theta / 2.0)))))), (ill.p.y + (ill.r * sin(toRad((ill.b - (ill.theta / 2.0)))))));
  var R: dynamic = Point((ill.p.x + (ill.r * cos(toRad((ill.b + (ill.theta / 2.0)))))), (ill.p.y + (ill.r * sin(toRad((ill.b + (ill.theta / 2.0)))))));
  if ((ccw(ill.p, L, c.p) == CLOCKWISE))
  {
    return false;
  }
  if ((ccw(ill.p, R, c.p) == COUNTER_CLOCKWISE))
  {
    return false;
  }
  var distL: dynamic = getDistanceLP(Line(ill.p, L), c.p);
  var distR: dynamic = getDistanceLP(Line(ill.p, R), c.p);
  if ((!((equals(distL, c.r) || (distL > c.r)))))
  {
    return false;
  }
  if ((!((equals(distR, c.r) || (distR > c.r)))))
  {
    return false;
  }
  var dist: dynamic = (sqrt(norm((ill.p - c.p))) + c.r);
  if ((!((equals(dist, ill.a) || (ill.a > dist)))))
  {
    return false;
  }
  return true;
}

func create_graph() -> dynamic
{
  rep(i, graph.size())[i].clear();
  graph.resize((n + 2), vp());
  REP(i, 1, (n + 1));
  {
    REP(j, 1, (n + 2));
    {
      if ((i == j))
      {
        continue;
      }
      if (check(illuminants[i], Circle(illuminants[j].p, illuminants[j].r)))
      {
        graph[i].push_back(P(j, illuminants[i].maxpower));
      }
    }
  }
}

func compute(vst: dynamic) -> dynamic
{
  graph[0] = vst;
  var order: dynamic = cpp_uninitialized();
  topologicalSort(graph, order);
  var dp: dynamic = cpp_array((n + 3));
  rep(i, (n + 3))[i] = 0;
  dp[0] = illuminants[0].maxpower;
  rep(i, order.size());
  {
    var cur: dynamic = order[i];
    rep(j, graph[cur].size());
    {
      var next: dynamic = graph[cur][j].to;
      dp[next] = min(illuminants[next].maxpower, (dp[next] + dp[cur]));
    }
  }
  return dp[(n + 1)];
}

func getArg(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  return acos((((((b * b) + (c * c)) - (a * a))) / (((2.0 * b) * c))));
}

func main() -> dynamic
{
  while ((cin >> n))
  {
    illuminants[0].p = Point(0, 0);
    illuminants[(n + 1)].maxpower = inf;
    read(illuminants[0].theta, illuminants[0].a, illuminants[0].maxpower);
    REP(i, 1, (n + 1));
    read(illuminants[i].p.x, illuminants[i].p.y, illuminants[i].r, illuminants[i].theta, illuminants[i].a, illuminants[i].b, illuminants[i].maxpower);
    read(illuminants[(n + 1)].p.x, illuminants[(n + 1)].p.y, illuminants[(n + 1)].r);
    create_graph();
    var ans: dynamic = 0;
    var theta: dynamic = illuminants[0].theta;
    var a: dynamic = illuminants[0].a;
    var maxpower: dynamic = illuminants[0].maxpower;
    REP(i, 1, (n + 2));
    {
      var alpha: dynamic = atan2(illuminants[i].p.y, illuminants[i].p.x);
      var A: dynamic = sqrt(norm(illuminants[i].p));
      var B: dynamic = illuminants[i].r;
      var C: dynamic = sqrt(((A * A) - (B * B)));
      var varg: dynamic = (acos((((((C * C) + (A * A)) - (B * B))) / (((2.0 * C) * A)))) + alpha);
      varg = ((varg * 180) / M_PI);
      if (((!equals(varg, 360.0)) && (varg > 360.0)))
      {
        varg -= 360.0;
      }
      varg -= (theta / 2.0);
      if (((!equals(varg, 0.0)) && (varg < 0.0)))
      {
        varg = (360.0 - varg);
      }
      var vst: dynamic = cpp_uninitialized();
      REP(j, 1, (n + 2));
      {
        if (check(Illuminant(Point(0, 0), a, theta, a, varg, maxpower), Circle(illuminants[j].p, illuminants[j].r)))
        {
          vst.push_back(P(j, maxpower));
        }
      }
      ans = max(ans, compute(vst));
      alpha = atan2(illuminants[i].p.y, illuminants[i].p.x);
      A = sqrt(norm(illuminants[i].p));
      B = illuminants[i].r;
      C = sqrt(((A * A) - (B * B)));
      varg = ((-acos((((((C * C) + (A * A)) - (B * B))) / (((2.0 * C) * A))))) + alpha);
      varg = ((varg * 180) / M_PI);
      if (((!equals(varg, 0.0)) && (varg < 0)))
      {
        varg = (360.0 + varg);
      }
      varg += (theta / 2.0);
      if (((!equals(varg, 360.0)) && (varg > 360.0)))
      {
        varg -= 360.0;
      }
      vst.clear();
      REP(j, 1, (n + 2));
      {
        if (check(Illuminant(Point(0, 0), a, theta, a, varg, maxpower), Circle(illuminants[j].p, illuminants[j].r)))
        {
          vst.push_back(P(j, maxpower));
        }
      }
      ans = max(ans, compute(vst));
    }
    write(cpp_cast(ans), "\n");
  }
  return 0;
}
