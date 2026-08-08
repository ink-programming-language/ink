// Translated from solution.cpp.

func REP(i: dynamic, x: dynamic)
{
  cpp_macro("for(int i=0;i<(int)(x);i++)");
}

func REPS(i: dynamic, x: dynamic)
{
  cpp_macro("for(int i=1;i<=(int)(x);i++)");
}

func RREP(i: dynamic, x: dynamic)
{
  cpp_macro("for(int i=((int)(x)-1);i>=0;i--)");
}

func RREPS(i: dynamic, x: dynamic)
{
  cpp_macro("for(int i=((int)(x));i>0;i--)");
}

func FOR(i: dynamic, c: dynamic)
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();i++)");
}

func RFOR(i: dynamic, c: dynamic)
{
  cpp_macro("for(__typeof((c).rbegin())i=(c).rbegin();i!=(c).rend();i++)");
}

func ALL(container: dynamic)
{
  return cpp_expression("#include <cstdio> #include <cmath> #in");
}

func RALL(container: dynamic)
{
  return cpp_expression("#include <cstdio> #include <cmath> #incl");
}

func SZ(container: dynamic)
{
  return cpp_expression("#include <cstdio> #incl");
}

func mp(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <cstdi");
}

func UNIQUE(v: dynamic)
{
  cpp_macro("v.erase( unique(v.begin(), v.end()), v.end() );");
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  (os << "[");
  (os << "]");
  return os;
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  (os << "{");
  (os << "}");
  return os;
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  return (((((os << "(") << t.first) << ",") << t.second) << ")");
}

func operator_add(s: dynamic, t: dynamic)
{
  return pair((s.first + t.first), (s.second + t.second));
}

func operator_subtract(s: dynamic, t: dynamic)
{
  return pair((s.first - t.first), (s.second - t.second));
}

var X = cpp_expression("#inclu");

var Y = cpp_expression("#inclu");

func at(i: dynamic)
{
  return cpp_expression("#include <cs");
}

var SELF = cpp_expression("#includ");

enum cpp_enum_1
{
  TRUE = 1,
  FALSE = 0,
  BORDER = -1
}

var INF = 1e8;

var EPS = 1e-6;

var PI = 3.1415926535897932384626;

func sig(x: dynamic)
{
  return (if ((abs(x) < EPS)) 0 else if ((x > 0)) 1 else -1);
}

func less(x: dynamic, y: dynamic)
{
  return if (sig((x - y))) (x < y) else BORDER;
}

func norm(p: dynamic)
{
  return ((p.X * p.X) + (p.Y * p.Y));
}

func inp(a: dynamic, b: dynamic)
{
  return ((conj(a) * b)).X;
}

func outp(a: dynamic, b: dynamic)
{
  return ((conj(a) * b)).Y;
}

func unit(p: dynamic)
{
  return (p / abs(p));
}

func proj(s: dynamic, t: dynamic)
{
  return ((t * inp(s, t)) / norm(t));
}

func ccw(s: dynamic, t: dynamic, p: dynamic, adv: dynamic = 0)
{
  var res = sig(outp((t - s), (p - s)));
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
  func L(p1: dynamic, p2: dynamic)
  {
      this->push_back(p1);
      this->push_back(p2);
    }
  func L()
  {
    }
  func dir()
  {
      return (at(1) - at(0));
    }
  func online(p: dynamic)
  {
      return (!sig(outp((p - at(0)), dir())));
    }
}

class S
{
  func S(p1: dynamic, p2: dynamic)
  {
      this->L = cpp_construct(p1, p2);
    }
  func S()
  {
    }
  func online(p: dynamic)
  {
      if (((!sig(norm((p - at(0))))) || (!sig(norm((p - at(1)))))))
      {
        return BORDER;
      }
      return if ((((!sig(outp((p - at(0)), dir()))) && (inp((p - at(0)), dir()) > (-EPS))) && (inp((p - at(1)), (-dir())) > (-EPS)))) true else false;
      return (!sig(((abs((at(0) - p)) + abs((at(1) - p))) - abs((at(0) - at(1))))));
    }
}

class G
{
  func G(size: dynamic = 0)
  {
      this->vector = cpp_construct(size);
    }
  func edge(i: dynamic)
  {
      return S(at(i), at(if (((i + 1) == size())) 0 else (i + 1)));
    }
}

func intersect(s: dynamic, l: dynamic)
{
  return (((sig(outp(l.dir(), (s[0] - l[0]))) * sig(outp(l.dir(), (s[1] - l[0])))) <= 0));
}

func crosspoint(l: dynamic, m: dynamic)
{
  var A = outp(l.dir(), m.dir());
  var B = outp(l.dir(), (l[1] - m[0]));
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
  var p: dynamic;
  var g: dynamic;
  func Arrangement()
  {
    }
  func Arrangement(seg: dynamic)
  {
      var m = seg.size();
      sort(ALL(p));
      UNIQUE(p);
      var n = p.size();
      g.resize(n);
    }
  func getIdx(q: dynamic)
  {
      var it = lower_bound(ALL(p), q);
      if (((it == p.end()) || ((*it) != q)))
      {
        return -1;
      }
      return (it - p.begin());
    }
}

class DualGraph
{
  var n: dynamic;
  var p: dynamic;
  var g: dynamic;
  func DualGraph(p: dynamic)
  {
      this->p = cpp_construct(p);
      this->g = cpp_construct(p.size());
      this->n = cpp_construct(p.size());
    }
  func add_edge(s: dynamic, t: dynamic)
  {
      var a = arg((p[t] - p[s]));
      g[s].emplace_back(s, t, a);
      g[t].emplace_back(t, s, if ((a > 0)) (a - PI) else (a + PI));
    }
  func add_polygon(s: dynamic, t: dynamic, a: dynamic)
  {
      var e = lower_bound(ALL(g[s]), (a - EPS));
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
      add_polygon(e->v, t, if ((e->a > 0)) (e->a - PI) else (e->a + PI));
    }
  func dual()
  {
      var s = (min_element(ALL(p)) - p.begin());
      var poly: dynamic;
      add_polygon(s, poly, ((-PI) * cpp_cast(0.5)));
      return poly;
    }
}

func operator_less(a: dynamic, b: dynamic)
{
  return if (sig((a.X - b.X))) (a.X < b.X) else ((a.Y + EPS) < b.Y);
}

func operator_equal(a: dynamic, b: dynamic)
{
  return (abs((a - b)) < EPS);
}

func operator_shift_right(is: dynamic, p: dynamic)
{
  var x: dynamic;
  var y: dynamic;
  ((is >> x) >> y);
  p = P(x, y);
  return is;
}

var n: dynamic;

var m: dynamic;

var k: dynamic;

var vil: dynamic;

class MSQ
{
  func MSQ()
  {
    }
  var p: dynamic;
  var s: dynamic;
  var m: dynamic;
  var k: dynamic;
  func MSQ(m: dynamic, k: dynamic)
  {
      this->m = cpp_construct(m);
      this->k = cpp_construct(k);
      REP(i, m).push_back(polar(cpp_cast(1), ((((2 * PI) * i) / m) + (PI * cpp_cast(0.5)))));
      REP(i, m).emplace_back(p[i], p[(((i + k)) % m)]);
      var dg = cpp_construct(a.p);
      REP(i, a.g.size());
      REP(j, a.g[i].size());
      {
        var u = a.g[i][j].u;
        var v = a.g[i][j].v;
        if ((u < v))
        {
          dg.add_edge(u, v);
        }
      }
      cpp_cast(cpp_assign(((*this)), "=", dg.dual()));
      reverse(this->begin(), this->end());
    }
  func copy(r: dynamic, c: dynamic, msq: dynamic)
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
  func segment(i: dynamic)
  {
      return s[i];
    }
}

func convex_contains(msq: dynamic, g: dynamic, p: dynamic)
{
  var n = msq.size();
  var a = 0;
  var b = n;
  var pg = (p - g);
  while (((a + 1) < b))
  {
    var c = (((a + b)) / 2);
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

func check(temp: dynamic, r: dynamic, i: dynamic, j: dynamic)
{
  var msq: dynamic;
  var gp = (vil[i] - (temp.segment(j)[0] * r));
  temp.copy(r, gp, msq);
  var l = msq.segment(j);
  var p: dynamic;
  var b = cpp_construct(0);
  var u = l.dir();
  if ((u < b))
  {
    swap(b, u);
  }
  return false;
}

func main()
{
  ios.sync_with_stdio(false);
  while (cpp_comma((((cin >> n) >> m) >> k), n))
  {
    vil = vector(n);
    REP(i, n);
    read(vil[i]);
    var best = 2000;
    REP(i, n);
    printf("%.10f\n", cpp_cast(best));
  }
  return 0;
}

func FOR(argument_0: dynamic, argument_1: dynamic)
{
    if ((it != t.begin()))
    {
      (os << ",");
    }
    (os << (*it));
  }

func FOR(argument_0: dynamic, argument_1: dynamic)
{
    if ((it != t.begin()))
    {
      (os << ",");
    }
    (os << (*it));
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      p.push_back(seg[i][0]);
      p.push_back(seg[i][1]);
      REP(j, i);
      if ((sig(outp(seg[i].dir(), seg[j].dir())) && (intersect(seg[i], seg[j]) == true)))
      {
        p.push_back(crosspoint(seg[i], seg[j]));
      }
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      var s = seg[i];
      var ps: dynamic;
      REP(j, n);
      if (s.online(p[j]))
      {
        ps.emplace_back(norm((p[j] - s[0])), j);
      }
      sort(ALL(ps));
      REP(j, (cpp_cast(ps.size()) - 1));
      {
        var u = ps[j].second;
        var v = ps[(j + 1)].second;
        g[u].emplace_back(u, v, 0, abs((p[u] - p[v])));
        g[v].emplace_back(v, u, 0, abs((p[u] - p[v])));
      }
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      sort(ALL(g[i]));
      UNIQUE(g[i]);
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      var s = msq.segment(j);
      if (intersect(s, l2))
      {
        var q = (crosspoint(s, l2) - l2[0]);
        p.push_back(q);
        ll = min(ll, q);
        rr = max(rr, q);
        f = 1;
      }
    }

func RREP(argument_0: dynamic, argument_1: dynamic)
{
    var l2 = cpp_construct(vil[i], (vil[i] + l.dir()));
    var f = 0;
    var rr = cpp_construct((-INF), (-INF));
    u = min(rr, u);
    b = max(ll, b);
    if (((!f) || (u < b)))
    {
      return false;
    }
  }

func __cpp_lambda_2()
{
  cpp_statement("REP(i, n)");
  if ((!convex_contains(msq, gp, (vil[i] + (*q)))))
  {
    return 0;
  }
  return 1;
}

func FOR(argument_0: dynamic, argument_1: dynamic)
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

func REP(argument_0: dynamic, argument_1: dynamic)
{
      if ((!check(temp, (best - EPS), i, j)))
      {
        continue;
      }
      var l = 1.0;
      var r = best;
      while (((r - l) > 1e-6))
      {
        var m = (((l + r)) * cpp_cast(0.5));
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
