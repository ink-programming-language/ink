// Translated from solution.cpp.

var MOD = (1e9 + 7);

var MX = (2e5 + 5);

var INF = 1e18;

var PI = acos(((ld) - 1));

var xd = [1, 0, -1, 0];

var yd = [0, 1, 0, -1];

var rng = cpp_construct(cpp_cast(chrono.steady_clock.now().time_since_epoch().count()));

func pct(x: dynamic)
{
  return builtin_popcount(x);
}

func bits(x: dynamic)
{
  return (31 - builtin_clz(x));
}

func cdiv(a: dynamic, b: dynamic)
{
  return ((a / b) + (((((a ^ b)) > 0) && (a % b))));
}

func fdiv(a: dynamic, b: dynamic)
{
  return ((a / b) - (((((a ^ b)) < 0) && (a % b))));
}

func ckmin(a: dynamic, b: dynamic)
{
  return if ((b < a)) cpp_comma(cpp_assign(a, "=", b), 1) else 0;
}

func ckmax(a: dynamic, b: dynamic)
{
  return if ((a < b)) cpp_comma(cpp_assign(a, "=", b), 1) else 0;
}

func fstTrue(lo: dynamic, hi: dynamic, first: dynamic)
{
  hi += 1;
  assert((lo <= hi));
  while ((lo < hi))
  {
    var mid = (lo + (((hi - lo)) / 2));
    if (first(mid)) cpp_assign(hi, "=", mid) else cpp_assign(lo, "=", (mid + 1));
  }
  return lo;
}

func lstTrue(lo: dynamic, hi: dynamic, first: dynamic)
{
  lo -= 1;
  assert((lo <= hi));
  while ((lo < hi))
  {
    var mid = (lo + ((((hi - lo) + 1)) / 2));
    if (first(mid)) cpp_assign(lo, "=", mid) else cpp_assign(hi, "=", (mid - 1));
  }
  return lo;
}

func remDup(v: dynamic)
{
  sort(begin(v), end(v));
  v.erase(unique(begin(v), end(v)), end(v));
}

func erase(t: dynamic, u: dynamic)
{
  var it = t.find(u);
  assert((it != end(t)));
  t.erase(u);
}

func re(x: dynamic)
{
  read(x);
}

func re(d: dynamic)
{
  var t: dynamic;
  re(t);
  d = stod(t);
}

func re(d: dynamic)
{
  var t: dynamic;
  re(t);
  d = stold(t);
}

func re(t: dynamic, u: dynamic...)
{
  re(t);
  re(cpp_expand(u));
}

func re(c: dynamic)
{
  var a: dynamic;
  var b: dynamic;
  re(a, b);
  c = [a, b];
}

func re(p: dynamic)
{
  re(p.first, p.second);
}

func re(x: dynamic)
{
  for (var a in x)
  {
    re(a);
  }
}

func re(x: dynamic)
{
  for (var a in x)
  {
    re(a);
  }
}

func to_string(c: dynamic)
{
  return str(1, c);
}

func to_string(second: dynamic)
{
  return cpp_cast(second);
}

func to_string(second: dynamic)
{
  return second;
}

func to_string(b: dynamic)
{
  return to_string(cpp_cast(b));
}

func to_string(c: dynamic)
{
  var ss: dynamic;
  (ss << c);
  return ss.str();
}

func to_string(v: dynamic)
{
  var res = "{";
  {
    var i = (0);
    while ((i < (cpp_cast((v).size()))))
    {
      res += char((cpp_char("0") + v[i]));
      i += 1;
    }
  }
  res += "}";
  return res;
}

func to_string(b: dynamic)
{
  var res = "";
  {
    var i = (0);
    while ((i < (SZ)))
    {
      res += char((cpp_char("0") + b[i]));
      i += 1;
    }
  }
  return res;
}

func to_string(v: dynamic)
{
  var fst = 1;
  var res = "";
  for (var x in v)
  {
    if ((!fst))
    {
      res += " ";
    }
    fst = 0;
    res += to_string(x);
  }
  return res;
}

func to_string(p: dynamic)
{
  return ((to_string(p.first) + " ") + to_string(p.second));
}

func pr(x: dynamic)
{
  write(to_string(x));
}

func pr(t: dynamic, u: dynamic...)
{
  pr(t);
  pr(cpp_expand(u));
}

func ps()
{
  pr("\n");
}

func ps(t: dynamic, u: dynamic...)
{
  pr(t);
  if (cpp_sizeof(u))
  {
    pr(" ");
  }
  ps(cpp_expand(u));
}

func DBG()
{
  write("]", "\n");
}

func DBG(t: dynamic, u: dynamic...)
{
  write(to_string(t));
  if (cpp_sizeof(u))
  {
    write(", ");
  }
  DBG(cpp_expand(u));
}

func setIn(second: dynamic)
{
  freopen(second.c_str(), "r", stdin);
}

func setOut(second: dynamic)
{
  freopen(second.c_str(), "w", stdout);
}

func unsyncIO()
{
  cin.tie(0)->sync_with_stdio(0);
}

func setIO(second: dynamic = "")
{
  unsyncIO();
  if (cpp_cast((second).size()))
  {
    setIn((second + ".in"));
    setOut((second + ".out"));
  }
}

func sgn(a: dynamic)
{
  return (((a > 0)) - ((a < 0)));
}

func sq(a: dynamic)
{
  return (a * a);
}

func norm(p: dynamic)
{
  return (sq(p.first) + sq(p.second));
}

func abs(p: dynamic)
{
  return sqrt(norm(p));
}

func arg(p: dynamic)
{
  return atan2(p.second, p.first);
}

func conj(p: dynamic)
{
  return pair(p.first, (-p.second));
}

func perp(p: dynamic)
{
  return pair((-p.second), p.first);
}

func dir(ang: dynamic)
{
  return pair(cos(ang), sin(ang));
}

func operator_subtract(l: dynamic)
{
  return pair((-l.first), (-l.second));
}

func operator_add(l: dynamic, r: dynamic)
{
  return pair((l.first + r.first), (l.second + r.second));
}

func operator_subtract(l: dynamic, r: dynamic)
{
  return pair((l.first - r.first), (l.second - r.second));
}

func operator_multiply(l: dynamic, r: dynamic)
{
  return pair((l.first * r), (l.second * r));
}

func operator_multiply(l: dynamic, r: dynamic)
{
  return (r * l);
}

func operator_divide(l: dynamic, r: dynamic)
{
  return pair((l.first / r), (l.second / r));
}

func operator_multiply(l: dynamic, r: dynamic)
{
  return pair(((l.first * r.first) - (l.second * r.second)), ((l.second * r.first) + (l.first * r.second)));
}

func operator_divide(l: dynamic, r: dynamic)
{
  return ((l * conj(r)) / norm(r));
}

func operator_add_assign(l: dynamic, r: dynamic)
{
  return cpp_assign(l, "=", (l + r));
}

func operator_subtract_assign(l: dynamic, r: dynamic)
{
  return cpp_assign(l, "=", (l - r));
}

func operator(l: dynamic, r: dynamic)
{
  return cpp_assign(l, "=", (l * r));
}

func operator(l: dynamic, r: dynamic)
{
  return cpp_assign(l, "=", (l / r));
}

func operator(l: dynamic, r: dynamic)
{
  return cpp_assign(l, "=", (l * r));
}

func operator(l: dynamic, r: dynamic)
{
  return cpp_assign(l, "=", (l / r));
}

func unit(p: dynamic)
{
  return (p / abs(p));
}

func dot(a: dynamic, b: dynamic)
{
  return ((a.first * b.first) + (a.second * b.second));
}

func cross(a: dynamic, b: dynamic)
{
  return ((a.first * b.second) - (a.second * b.first));
}

func cross(p: dynamic, a: dynamic, b: dynamic)
{
  return cross((a - p), (b - p));
}

func reflect(p: dynamic, a: dynamic, b: dynamic)
{
  return (a + (conj((((p - a)) / ((b - a)))) * ((b - a))));
}

func foot(p: dynamic, a: dynamic, b: dynamic)
{
  return (((p + reflect(p, a, b))) / cpp_cast(2));
}

func onSeg(p: dynamic, a: dynamic, b: dynamic)
{
  return ((cross(a, b, p) == 0) && (dot((p - a), (p - b)) <= 0));
}

var N: dynamic;

var poly: dynamic;

var dists: dynamic;

var par: dynamic;

func genDists()
{
  dists = [0];
  {
    var i = (0);
    while ((i < (N)))
    {
      dists.push_back((dists.back() + abs((poly[i] - poly[(i + 1)]))));
      i += 1;
    }
  }
}

func rev()
{
  par ^= 1;
  var maxY = poly.back().second;
  reverse(begin(poly), end(poly));
  genDists();
  for (var t in poly)
  {
    t.second = (maxY - t.second);
  }
}

var ans: dynamic;

var vel: dynamic;

func ternary(lo: dynamic, hi: dynamic, eval: dynamic)
{
  {
    var cpp_name = (0);
    while ((cpp_name < (50)))
    {
      var m1 = ((((2 * lo) + hi)) / 3);
      var m2 = (((lo + (2 * hi))) / 3);
      if ((eval(m1) < eval(m2)))
      {
        hi = m2;
      } else
      {
        lo = m1;
      }
      cpp_name += 1;
    }
  }
  return [lo, eval(lo)];
}

var calced = cpp_array(2);

func tri(l: dynamic, r: dynamic)
{
  assert((l < r));
  var dirl = unit((poly[(l + 1)] - poly[l]));
  var dirr = unit((poly[(r + 1)] - poly[r]));
  var eval = __cpp_lambda_1;
  var getMin = __cpp_lambda_2;
  if ((!calced[par].count([l, r])))
  {
    calced[par][[l, r]] = ternary(0, (dists[(r + 1)] - dists[r]), getMin);
  }
  var mid = calced[par][[l, r]];
  if ((mid.second > (1 / vel)))
  {
    return;
  }
  var L: dynamic;
  var R: dynamic;
  {
    var lo = 0;
    var hi = mid.first;
    {
      var cpp_name = (0);
      while ((cpp_name < (50)))
      {
        var m = (((lo + hi)) / 2);
        if ((getMin(m) < (1 / vel)))
        {
          hi = m;
        } else
        {
          lo = m;
        }
        cpp_name += 1;
      }
    }
    L = lo;
  }
  {
    var lo = mid.first;
    var hi = (dists[(r + 1)] - dists[r]);
    {
      var cpp_name = (0);
      while ((cpp_name < (50)))
      {
        var m = (((lo + hi)) / 2);
        if ((getMin(m) < (1 / vel)))
        {
          lo = m;
        } else
        {
          hi = m;
        }
        cpp_name += 1;
      }
    }
    R = lo;
  }
  ans.push_back([(dists[r] + L), (dists[r] + R)]);
}

func ranges()
{
  ans.clear();
  {
    var i = (0);
    while ((i < (N)))
    {
      {
        var j = ((i + 1));
        while ((j < (N)))
        {
          tri(i, j);
          j += 1;
        }
      }
      i += 1;
    }
  }
  sort(begin(ans), end(ans));
  var ANS: dynamic;
  for (var t in ans)
  {
    if (((!cpp_cast((ANS).size())) || (ANS.back().second < t.first)))
    {
      ANS.push_back(t);
    } else
    {
      ckmax(ANS.back().second, t.second);
    }
  }
  return ANS;
}

func intervalIsect(a: dynamic, b: dynamic)
{
  if ((a.second < b.first))
  {
    return 0;
  }
  if ((b.second < a.first))
  {
    return 0;
  }
  return 1;
}

func escape(mid: dynamic)
{
  0;
  vel = mid;
  var lef = ranges();
  rev();
  var rig = ranges();
  rev();
  for (var t in rig)
  {
    t = [(dists.back() - t.second), (dists.back() - t.first)];
  }
  reverse(begin(rig), end(rig));
  var il = 0;
  var ir = 0;
  while (((il < cpp_cast((lef).size())) && (ir < cpp_cast((rig).size()))))
  {
    if (intervalIsect(lef[il], rig[ir]))
    {
      return 1;
    }
    if ((lef[il].first < rig[ir].first))
    {
      il += 1;
    } else
    {
      ir += 1;
    }
  }
  return 0;
}

func main()
{
  var beg = clock();
  setIO();
  re(N);
  poly.resize((N + 1));
  re(poly);
  genDists();
  var lo = 1;
  var hi = 1e4;
  while (((hi / lo) > (1 + 1e-8)))
  {
    var mid = sqrt((lo * hi));
    if (escape(mid))
    {
      lo = mid;
    } else
    {
      hi = mid;
    }
  }
  write(fixed, setprecision(9), lo);
  0;
}

func __cpp_lambda_1(t1: dynamic, t2: dynamic)
{
  var pos1 = (poly[l] + (t1 * dirl));
  var pos2 = (poly[r] + (t2 * dirr));
  return (abs((pos2 - pos1)) / ((((dists[r] + t2)) - ((dists[l] + t1)))));
}

func __cpp_lambda_3(x: dynamic)
{
  return eval(x, rig);
}

func __cpp_lambda_2(rig: dynamic)
{
  return ternary(0, (dists[(l + 1)] - dists[l]), __cpp_lambda_3).second;
}
