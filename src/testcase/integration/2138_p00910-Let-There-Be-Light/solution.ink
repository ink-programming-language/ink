// Translated from solution.cpp.

func rep(i: dynamic, x: dynamic) -> dynamic
{
  cpp_macro("for(int i=1;i<=(int)(x);i++)");
}

func REP(i: dynamic, x: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(int)(x);i++)");
}

func FOR(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();i++)");
}

func RREP(i: dynamic, x: dynamic) -> dynamic
{
  cpp_macro("for(int i=(x);i>=0;i--)");
}

func RFOR(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((c).rbegin())i=(c).rbegin();i!=(c).rend();i++)");
}

func ALL(container: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdio> #include <cmath>");
}

func SZ(container: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdio> #incl");
}

func mp(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdi");
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func operator_shift_left(os: dynamic, t: dynamic) -> dynamic
{
  (os << "{");
  (os << "}");
  return os;
}

func operator_shift_left(os: dynamic, t: dynamic) -> dynamic
{
  return (((((os << "(") << t.first) << ",") << t.second) << ")");
}

var INF: dynamic = (1 << 28);

var EPS: dynamic = 1e-8;

var MOD: dynamic = 1000000007;

class P
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  func P() -> dynamic
  {
    }
  func P(a: dynamic, b: dynamic, c: dynamic) -> dynamic
  {
      self->x = cpp_construct(a);
      self->y = cpp_construct(b);
      self->z = cpp_construct(c);
    }
  func operator_add(opp: dynamic) -> dynamic
  {
      return P((x + opp.x), (y + opp.y), (z + opp.z));
    }
  func operator_subtract(opp: dynamic) -> dynamic
  {
      return P((x - opp.x), (y - opp.y), (z - opp.z));
    }
  func operator_divide(opp: dynamic) -> dynamic
  {
      return P( ((abs(opp.x) < EPS)) ? 0 : (x / opp.x),  ((abs(opp.y) < EPS)) ? 0 : (y / opp.y),  ((abs(opp.z) < EPS)) ? 0 : (z / opp.z));
    }
  func operator_multiply(opp: dynamic) -> dynamic
  {
      return P((x * opp), (y * opp), (z * opp));
    }
}

class C
{
  var p: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  func C() -> dynamic
  {
    }
  func C(q: dynamic, s: dynamic) -> dynamic
  {
      self->p = cpp_construct(q);
      self->r = cpp_construct(s);
    }
}

func inp(v1: dynamic, v2: dynamic) -> dynamic
{
  return (((v1.x * v2.x) + (v1.y * v2.y)) + (v1.z * v2.z));
}

func norm(v: dynamic) -> dynamic
{
  return (((v.x * v.x) + (v.y * v.y)) + (v.z * v.z));
}

func sp_distance(t: dynamic, p1: dynamic, p2: dynamic) -> dynamic
{
  var v1: dynamic = (t - p1);
  var v2: dynamic = (p2 - p1);
  var v3: dynamic = (v2 * ((inp(v1, v2) / norm(v2))));
  var v4: dynamic = (v3 / v2);
  if (((v4.x < (-EPS)) || ((1 + EPS) < v4.x)))
  {
    return sqrt(min(norm((t - p1)), norm((t - p2))));
  }
  if (((v4.y < (-EPS)) || ((1 + EPS) < v4.y)))
  {
    return sqrt(min(norm((t - p1)), norm((t - p2))));
  }
  if (((v4.z < (-EPS)) || ((1 + EPS) < v4.z)))
  {
    return sqrt(min(norm((t - p1)), norm((t - p2))));
  }
  return sqrt(norm((v1 - v3)));
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  while (cpp_comma((((cin >> n) >> m) >> r), n))
  {
    var cir: dynamic = cpp_uninitialized();
    var l: dynamic = cpp_uninitialized();
    var e: dynamic = cpp_uninitialized();
    read(e.x, e.y, e.z);
    REP(i, n);
    var ans: dynamic = 0;
    REP(i, (1 << m));
    {
      var b: dynamic = 0;
      REP(j, n);
      if ((lb[j] & i))
      {
        b += 1;
      }
      if ((b > r))
      {
        continue;
      }
      var sum: dynamic = 0;
      REP(j, m);
      if ((((i >> j)) & 1))
      {
        sum += l[j].second;
      }
      ans = max(ans, sum);
    }
    printf("%.10f\n", ans);
  }
  return 0;
}

func FOR(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    if ((it != t.begin()))
    {
      (os << ",");
    }
    (os << (*it));
  }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var z: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      read(x, y, z, r);
      cir.push_back(C(P(x, y, z), r));
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var z: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(x, y, z, b);
      l.push_back(mp(P(x, y, z), b));
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      l[i].second /= norm((l[i].first - e));
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if (((sp_distance(cir[i].p, l[j].first, e) <= (cir[i].r + EPS)) && (!(((norm((cir[i].p - l[j].first)) <= ((cir[i].r * cir[i].r) + EPS)) && (norm((cir[i].p - e)) <= ((cir[i].r * cir[i].r) + EPS)))))))
      {
        lb[i] |= (1 << j);
      }
    }
