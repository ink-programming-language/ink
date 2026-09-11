// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i<b;i++)");
}

func REP(i: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

var PB: dynamic = cpp_expression("#include");

func read() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  scanf("%d", (&i));
  return i;
}

var inf: dynamic = 1000000007;

class Seg
{
  var mn: dynamic = cpp_array((1 << 18));
  var lz: dynamic = cpp_array((1 << 18));
  var s: dynamic = cpp_uninitialized();
  func Init(n: dynamic) -> dynamic
  {
      s = 1;
      while ((s < n))
      {
        s *= 2;
      }
      fill(mn, (mn + (s * 2)), 0);
      fill(lz, (lz + (s * 2)), 0);
    }
  func NodeMin(i: dynamic) -> dynamic
  {
      return (mn[i] + lz[i]);
    }
  func Propagate(i: dynamic) -> dynamic
  {
      lz[(i * 2)] += lz[i];
      lz[((i * 2) + 1)] += lz[i];
      mn[i] += lz[i];
      lz[i] = 0;
    }
  func Add(b: dynamic, e: dynamic, l: dynamic, r: dynamic, i: dynamic, v: dynamic) -> dynamic
  {
      if (((e <= l) || (r <= b)))
      {
        return;
      }
      if (((b <= l) && (r <= e)))
      {
        lz[i] += v;
        return;
      }
      Propagate(i);
      Add(b, e, l, (((l + r)) / 2), (i * 2), v);
      Add(b, e, (((l + r)) / 2), r, ((i * 2) + 1), v);
      mn[i] = min(NodeMin((i * 2)), NodeMin(((i * 2) + 1)));
    }
  func Add(b: dynamic, e: dynamic, v: dynamic) -> dynamic
  {
      Add(b, e, 0, s, 1, v);
    }
  func Get(b: dynamic, e: dynamic, l: dynamic, r: dynamic, i: dynamic) -> dynamic
  {
      if (((e <= l) || (r <= b)))
      {
        return inf;
      }
      if (((b <= l) && (r <= e)))
      {
        return NodeMin(i);
      }
      Propagate(i);
      return min(Get(b, e, l, (((l + r)) / 2), (i * 2)), Get(b, e, (((l + r)) / 2), r, ((i * 2) + 1)));
    }
  func Get(b: dynamic, e: dynamic) -> dynamic
  {
      return Get(b, e, 0, s, 1);
    }
}

var seg: dynamic = cpp_uninitialized();

var Nmax: dynamic = 334893;

class Pos
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

var ps: dynamic = cpp_array(Nmax);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

class Query
{
  var y: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var e: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  func operator_less(rhs: dynamic) -> dynamic
  {
      return (y < rhs.y);
    }
}

var qs: dynamic = cpp_array(Nmax);

func GetRange(y: dynamic) -> dynamic
{
  var b: dynamic = abs(y);
  var e: dynamic = (((((n - 1)) + ((m - 1))) - abs((y - ((((m - 1)) - ((n - 1))))))) + 1);
  return pii(b, e);
}

func Solve(s: dynamic) -> dynamic
{
  qs[(k * 2)] = [inf, 0, 0, 0];
  sort(qs, ((qs + (2 * k)) + 1));
  {
    var modify: dynamic = __cpp_lambda_1;
    seg.Init((n + m));
    var lastY: dynamic = (-((n - 1)));
    modify(lastY);
    REP(i, ((2 * k) + 1));
    {
      var q: dynamic = qs[i];
      modify(q.y);
      modify(q.b);
      modify(q.e);
      if ((lastY < q.y))
      {
        var u: dynamic = GetRange(lastY);
        var b: dynamic = GetRange((q.y - 1));
        if (((lastY <= 0) && (0 <= (q.y - 1))))
        {
          u.first = 0;
        }
        if (((lastY <= (((m - 1)) - ((n - 1)))) && ((((m - 1)) - ((n - 1))) <= (q.y - 1))))
        {
          u.second = ((((m - 1)) + ((n - 1))) + 1);
        }
        modify(u.first);
        modify(u.second);
        modify(b.first);
        modify(b.second);
        if ((seg.Get((min(u.first, b.first) / 2), (max(u.second, b.second) / 2)) == 0))
        {
          return true;
        }
      }
      seg.Add((q.b / 2), (q.e / 2), q.v);
      lastY = q.y;
    }
  }
  {
    var modify: dynamic = __cpp_lambda_2;
    seg.Init((n + m));
    var lastY: dynamic = (-((n - 1)));
    modify(lastY);
    REP(i, ((2 * k) + 1));
    {
      var q: dynamic = qs[i];
      modify(q.y);
      modify(q.b);
      modify(q.e);
      if ((lastY < q.y))
      {
        var u: dynamic = GetRange(lastY);
        var b: dynamic = GetRange((q.y - 1));
        if (((lastY <= 0) && (0 <= (q.y - 1))))
        {
          u.first = 0;
        }
        if (((lastY <= (((m - 1)) - ((n - 1)))) && ((((m - 1)) - ((n - 1))) <= (q.y - 1))))
        {
          u.second = ((((m - 1)) + ((n - 1))) + 1);
        }
        modify(u.first);
        modify(u.second);
        modify(b.first);
        modify(b.second);
        if ((seg.Get((min(u.first, b.first) / 2), (max(u.second, b.second) / 2)) == 0))
        {
          return true;
        }
      }
      seg.Add((q.b / 2), (q.e / 2), q.v);
      lastY = q.y;
    }
  }
  return false;
}

func main() -> dynamic
{
  n = read();
  m = read();
  k = read();
  var l: dynamic = -1;
  var r: dynamic = (n + m);
  while (((r - l) > 1))
  {
    var mid: dynamic = (((r + l)) / 2);
    if (Solve(mid))
    {
      l = mid;
    } else
    {
      r = mid;
    }
  }
  write(r, "\n");
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    qs[(i * 2)] = [(ps[i].y - s), max(0, (ps[i].x - s)), min((n + m), ((ps[i].x + s) + 1)), 1];
    qs[((i * 2) + 1)] = [((ps[i].y + s) + 1), max(0, (ps[i].x - s)), min((n + m), ((ps[i].x + s) + 1)), -1];
  }

func __cpp_lambda_1(v: dynamic) -> dynamic
{
  if ((v & 1))
  {
    v += 1;
  }
}

func __cpp_lambda_2(v: dynamic) -> dynamic
{
  if ((!((v & 1))))
  {
    v += 1;
  }
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var x: dynamic = read();
    var y: dynamic = read();
    ps[i] = [(x + y), (y - x)];
  }
