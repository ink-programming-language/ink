// Translated from solution.cpp.

func REP(i: dynamic, x: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(int)(x);i++)");
}

func REPS(i: dynamic, x: dynamic) -> dynamic
{
  cpp_macro("for(int i=1;i<=(int)(x);i++)");
}

func RREP(i: dynamic, x: dynamic) -> dynamic
{
  cpp_macro("for(int i=((int)(x)-1);i>=0;i--)");
}

func RREPS(i: dynamic, x: dynamic) -> dynamic
{
  cpp_macro("for(int i=((int)(x));i>0;i--)");
}

func FOR(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();i++)");
}

func RFOR(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((c).rbegin())i=(c).rbegin();i!=(c).rend();i++)");
}

func ALL(container: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdio> #include <cmath> #in");
}

func RALL(container: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdio> #include <cmath> #incl");
}

func SZ(container: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdio> #incl");
}

func mp(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdi");
}

func UNIQUE(v: dynamic) -> dynamic
{
  cpp_macro("v.erase( unique(v.begin(), v.end()), v.end() );");
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
  (os << "[");
  (os << "]");
  return os;
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

func operator_add(s: dynamic, t: dynamic) -> dynamic
{
  return pair((s.first + t.first), (s.second + t.second));
}

func operator_subtract(s: dynamic, t: dynamic) -> dynamic
{
  return pair((s.first - t.first), (s.second - t.second));
}

var X: dynamic = cpp_expression("#inclu");

var Y: dynamic = cpp_expression("#inclu");

func at(i: dynamic) -> dynamic
{
  return cpp_expression("#include <cs");
}

var SELF: dynamic = cpp_expression("#includ");

enum cpp_enum_1
{
  enum_field TRUE = 1;
  enum_field FALSE = 0;
  enum_field BORDER = -1;
}

var INF: dynamic = 1e8;

var EPS: dynamic = 1e-6;

var PI: dynamic = 3.1415926535897932384626;

func sig(x: dynamic) -> dynamic
{
  return ( ((abs(x) < EPS)) ? 0 :  ((x > 0)) ? 1 : -1);
}

func less(x: dynamic, y: dynamic) -> dynamic
{
  return  (sig((x - y))) ? (x < y) : BORDER;
}

func norm(p: dynamic) -> dynamic
{
  return ((p.X * p.X) + (p.Y * p.Y));
}

func inp(a: dynamic, b: dynamic) -> dynamic
{
  return ((conj(a) * b)).X;
}

func outp(a: dynamic, b: dynamic) -> dynamic
{
  return ((conj(a) * b)).Y;
}

func unit(p: dynamic) -> dynamic
{
  return (p / abs(p));
}

func proj(s: dynamic, t: dynamic) -> dynamic
{
  return ((t * inp(s, t)) / norm(t));
}

func ccw(s: dynamic, t: dynamic, p: dynamic, adv: dynamic = 0) -> dynamic
{
  var res: dynamic = sig(outp((t - s), (p - s)));
  if ((res || (!adv)))
  {
    return res;
  }
  if ((sig(inp((t - s), (p - s))) < 0))
  {
    return -2;
  }
  if ((sig(inp((s - t), (p - t))) < 0))
  {
    return 2;
  }
  return 0;
}

class L
{
  func L(p1: dynamic, p2: dynamic) -> dynamic
  {
      self->push_back(p1);
      self->push_back(p2);
    }
  func L() -> dynamic
  {
    }
  func dir() -> dynamic
  {
      return (at(1) - at(0));
    }
  func online(p: dynamic) -> dynamic
  {
      return (!sig(outp((p - at(0)), dir())));
    }
}

class S
{
  func S(p1: dynamic, p2: dynamic) -> dynamic
  {
      self->L = cpp_construct(p1, p2);
    }
  func S() -> dynamic
  {
    }
  func online(p: dynamic) -> dynamic
  {
      if (((!sig(norm((p - at(0))))) || (!sig(norm((p - at(1)))))))
      {
        return BORDER;
      }
      return  ((((!sig(outp((p - at(0)), dir()))) && (inp((p - at(0)), dir()) > (-EPS))) && (inp((p - at(1)), (-dir())) > (-EPS)))) ? true : false;
      return (!sig(((abs((at(0) - p)) + abs((at(1) - p))) - abs((at(0) - at(1))))));
    }
}

class G
{
  func G(size: dynamic = 0) -> dynamic
  {
      self->vector = cpp_construct(size);
    }
  func edge(i: dynamic) -> dynamic
  {
      return S(at(i), at( (((i + 1) == size())) ? 0 : (i + 1)));
    }
}

func intersect(s: dynamic, l: dynamic) -> dynamic
{
  return (((sig(outp(l.dir(), (s[0] - l[0]))) * sig(outp(l.dir(), (s[1] - l[0])))) <= 0));
}

func crosspoint(l: dynamic, m: dynamic) -> dynamic
{
  var A: dynamic = outp(l.dir(), m.dir());
  var B: dynamic = outp(l.dir(), (l[1] - m[0]));
  if (((!sig(abs(A))) && (!sig(abs(B)))))
  {
    return m[0];
  }
  if ((abs(A) < EPS))
  {
    assert(false);
  }
  return (m[0] + ((B / A) * ((m[1] - m[0]))));
}

class Arrangement
{
  var p: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
  func Arrangement() -> dynamic
  {
    }
  func Arrangement(seg: dynamic) -> dynamic
  {
      var m: dynamic = seg.size();
      sort(ALL(p));
      UNIQUE(p);
      var n: dynamic = p.size();
      g.resize(n);
    }
  func getIdx(q: dynamic) -> dynamic
  {
      var it: dynamic = lower_bound(ALL(p), q);
      if (((it == p.end()) || ((*it) != q)))
      {
        return -1;
      }
      return (it - p.begin());
    }
}

class DualGraph
{
  var n: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
  func DualGraph(p: dynamic) -> dynamic
  {
      self->p = cpp_construct(p);
      self->g = cpp_construct(p.size());
      self->n = cpp_construct(p.size());
    }
  func add_edge(s: dynamic, t: dynamic) -> dynamic
  {
      var a: dynamic = arg((p[t] - p[s]));
      g[s].emplace_back(s, t, a);
      g[t].emplace_back(t, s,  ((a > 0)) ? (a - PI) : (a + PI));
    }
  func add_polygon(s: dynamic, t: dynamic, a: dynamic) -> dynamic
  {
      var e: dynamic = lower_bound(ALL(g[s]), (a - EPS));
      if ((e == g[s].end()))
      {
        e = g[s].begin();
      }
      if (e->f)
      {
        return;
      }
      e->f = 1;
      t.push_back(p[s]);
      add_polygon(e->v, t,  ((e->a > 0)) ? (e->a - PI) : (e->a + PI));
    }
  func dual() -> dynamic
  {
      var s: dynamic = (min_element(ALL(p)) - p.begin());
      var poly: dynamic = cpp_uninitialized();
      add_polygon(s, poly, ((-PI) * cpp_cast(0.5)));
      return poly;
    }
}

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  return  (sig((a.X - b.X))) ? (a.X < b.X) : ((a.Y + EPS) < b.Y);
}

func operator_equal(a: dynamic, b: dynamic) -> dynamic
{
  return (abs((a - b)) < EPS);
}

func operator_shift_right(is: dynamic, p: dynamic) -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  ((is >> x) >> y);
  p = P(x, y);
  return is;
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var vil: dynamic = cpp_uninitialized();

class MSQ
{
  func MSQ() -> dynamic
  {
    }
  var p: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  func MSQ(m: dynamic, k: dynamic) -> dynamic
  {
      self->m = cpp_construct(m);
      self->k = cpp_construct(k);
      REP(i, m).push_back(polar(cpp_cast(1), ((((2 * PI) * i) / m) + (PI * cpp_cast(0.5)))));
      REP(i, m).emplace_back(p[i], p[(((i + k)) % m)]);
      var dg: dynamic = cpp_construct(a.p);
      REP(i, a.g.size());
      REP(j, a.g[i].size());
      {
        var u: dynamic = a.g[i][j].u;
        var v: dynamic = a.g[i][j].v;
        if ((u < v))
        {
          dg.add_edge(u, v);
        }
      }
      cpp_cast(cpp_assign(((*self)), "=", dg.dual()));
      reverse(self->begin(), self->end());
    }
  func copy(r: dynamic, c: dynamic, msq: dynamic) -> dynamic
  {
      msq.resize(size());
      msq.p.resize(p.size());
      msq.s.resize(s.size());
      msq.m = m;
      msq.k = k;
      REP(i, size())[i] = ((at(i) * r) + c);
      REP(i, p.size()).p[i] = ((p[i] * r) + c);
      REP(i, s.size()).s[i] = S(msq.p[i], msq.p[(((i + k)) % m)]);
    }
  func segment(i: dynamic) -> dynamic
  {
      return s[i];
    }
}

func convex_contains(msq: dynamic, g: dynamic, p: dynamic) -> dynamic
{
  var n: dynamic = msq.size();
  var a: dynamic = 0;
  var b: dynamic = n;
  var pg: dynamic = (p - g);
  while (((a + 1) < b))
  {
    var c: dynamic = (((a + b)) / 2);
    if (((outp((msq[a] - g), pg) > 0) && (outp((msq[c] - g), pg) < 0)))
    {
      b = c;
    } else
    {
      a = c;
    }
  }
  b %= n;
  if ((outp((msq[a] - p), (msq[b] - p)) < (-EPS)))
  {
    return 0;
  }
  return 1;
}

func check(temp: dynamic, r: dynamic, i: dynamic, j: dynamic) -> dynamic
{
  var msq: dynamic = cpp_uninitialized();
  var gp: dynamic = (vil[i] - (temp.segment(j)[0] * r));
  temp.copy(r, gp, msq);
  var l: dynamic = msq.segment(j);
  var p: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_construct(0);
  var u: dynamic = l.dir();
  if ((u < b))
  {
    swap(b, u);
  }
  return false;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  while (cpp_comma((((cin >> n) >> m) >> k), n))
  {
    vil = vector(n);
    REP(i, n);
    read(vil[i]);
    var best: dynamic = 2000;
    REP(i, n);
    printf("%.10f\n", cpp_cast(best));
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
      p.push_back(seg[i][0]);
      p.push_back(seg[i][1]);
      REP(j, i);
      if ((sig(outp(seg[i].dir(), seg[j].dir())) && (intersect(seg[i], seg[j]) == true)))
      {
        p.push_back(crosspoint(seg[i], seg[j]));
      }
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var s: dynamic = seg[i];
      var ps: dynamic = cpp_uninitialized();
      REP(j, n);
      if (s.online(p[j]))
      {
        ps.emplace_back(norm((p[j] - s[0])), j);
      }
      sort(ALL(ps));
      REP(j, (cpp_cast(ps.size()) - 1));
      {
        var u: dynamic = ps[j].second;
        var v: dynamic = ps[(j + 1)].second;
        g[u].emplace_back(u, v, 0, abs((p[u] - p[v])));
        g[v].emplace_back(v, u, 0, abs((p[u] - p[v])));
      }
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      sort(ALL(g[i]));
      UNIQUE(g[i]);
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var s: dynamic = msq.segment(j);
      if (intersect(s, l2))
      {
        var q: dynamic = (crosspoint(s, l2) - l2[0]);
        p.push_back(q);
        ll = min(ll, q);
        rr = max(rr, q);
        f = 1;
      }
    }

func RREP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var l2: dynamic = cpp_construct(vil[i], (vil[i] + l.dir()));
    var f: dynamic = 0;
    var rr: dynamic = cpp_construct((-INF), (-INF));
    u = min(rr, u);
    b = max(ll, b);
    if (((!f) || (u < b)))
    {
      return false;
    }
  }

func __cpp_lambda_2() -> dynamic
{
  cpp_statement("REP(i, n)");
  if ((!convex_contains(msq, gp, (vil[i] + (*q)))))
  {
    return 0;
  }
  return 1;
}

func FOR(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    if ((((*q) < b) || (u < (*q))))
    {
      continue;
    }
    if (__cpp_lambda_2())
    {
      return true;
    }
  }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if ((!check(temp, (best - EPS), i, j)))
      {
        continue;
      }
      var l: dynamic = 1.0;
      var r: dynamic = best;
      while (((r - l) > 1e-6))
      {
        var m: dynamic = (((l + r)) * cpp_cast(0.5));
        if (check(temp, m, i, j))
        {
          r = m;
        } else
        {
          l = m;
        }
      }
      best = r;
    }
