// Translated from solution.cpp.

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

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  (((((os << cpp_char("(")) << p.first) << cpp_char(",")) << p.second) << cpp_char(")"));
  return os;
}

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  (os << cpp_char("{"));
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(v.size())))
    {
      if (i)
      {
        (os << cpp_char(","));
      }
      (os << v[i]);
      i += 1;
    }
  }
  (os << cpp_char("}"));
  return os;
}

func debugg() -> dynamic
{
  write("\n");
}

func debugg(x: dynamic, args: dynamic...) -> dynamic
{
  write(" ", x);
  debugg(cpp_expand(args));
}

func debug() -> dynamic
{
  return cpp_expression("#line 1 \"/home/siro53/kyo-pro/compro_library/template/template.cpp\" #i");
}

func dump(x: dynamic) -> dynamic
{
  return cpp_expression("#line 1 \"/home/siro53/kyo-pro/compro_library/template");
}

func debug() -> dynamic
{
  return cpp_expression("#line 1 \"");
}

func dump(x: dynamic) -> dynamic
{
  return cpp_expression("#line 1 \"");
}

class Setup
{
  func Setup() -> dynamic
  {
      cin.tie(0);
      ios.sync_with_stdio(false);
      write(fixed, setprecision(15));
    }
}

var Setup: dynamic = cpp_uninitialized();

func ALL(v: dynamic) -> dynamic
{
  return cpp_expression("#line 1 \"/home/siro53/");
}

func RALL(v: dynamic) -> dynamic
{
  return cpp_expression("#line 1 \"/home/siro53/ky");
}

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i = (a); i < int(b); i++)");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#line 1 \"/ho");
}

var INF: dynamic = (1 << 30);

var LLINF: dynamic = (1 << 60);

var MOD: dynamic = 1000000007;

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, 1, 0, -1];

func solve() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var a: dynamic = cpp_construct((2 * N));
  REP(i, (2 * N));
  read(a[i]);
  var max_id: dynamic = (max_element(ALL(a)) - a.begin());
  debug(N);
  REP(other_id, (N * 2));
  {
    if ((other_id == max_id))
    {
      continue;
    }
    var x: dynamic = a[max_id];
    var se: dynamic = cpp_uninitialized();
    REP(i, (N * 2));
    {
      if (cpp_binary((i == max_id), "or", (i == other_id)))
      {
        continue;
      }
      se.insert(a[i]);
    }
    var ok: dynamic = true;
    var ans: dynamic = cpp_uninitialized();
    ans.emplace_back(a[max_id], a[other_id]);
    debug(a[max_id], a[other_id]);
    while ((!se.empty()))
    {
      var f: dynamic = false;
      {
        var it: dynamic = prev(se.end());
        while ((it != se.begin()))
        {
          var it2: dynamic = se.find((x - (*it)));
          if ((it2 == se.end()))
          {
            it -= 1;
            continue;
          }
          if ((it == it2))
          {
            it -= 1;
            continue;
          }
          var v1: dynamic = (*it);
          var v2: dynamic = (*it2);
          debug(v1, v2);
          ans.emplace_back(v1, v2);
          x = max(v1, v2);
          se.erase(it);
          se.erase(it2);
          f = true;
          break;
          it -= 1;
        }
      }
      if ((!f))
      {
        ok = false;
        break;
      }
    }
    if (ok)
    {
      write("YES\n");
      write((a[max_id] + a[other_id]), "\n");
      REP(i, ans.size());
      {
        write(v1, " ", v2, "\n");
      }
      return;
    }
  }
  write("NO\n");
}

func main() -> dynamic
{
  var t: dynamic = 1;
  read(t);
  REP(please_give_me_ac, t);
  solve();
}
