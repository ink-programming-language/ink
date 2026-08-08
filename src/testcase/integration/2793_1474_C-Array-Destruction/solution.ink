// Translated from solution.cpp.

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

func operator_shift_left(os: dynamic, p: dynamic)
{
  (((((os << cpp_char("(")) << p.first) << cpp_char(",")) << p.second) << cpp_char(")"));
  return os;
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  (os << cpp_char("{"));
  {
    var i = 0;
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

func debugg()
{
  write("\n");
}

func debugg(x: dynamic, args: dynamic...)
{
  write(" ", x);
  debugg(cpp_expand(args));
}

func debug()
{
  return cpp_expression("#line 1 \"/home/siro53/kyo-pro/compro_library/template/template.cpp\" #i");
}

func dump(x: dynamic)
{
  return cpp_expression("#line 1 \"/home/siro53/kyo-pro/compro_library/template");
}

func debug()
{
  return cpp_expression("#line 1 \"");
}

func dump(x: dynamic)
{
  return cpp_expression("#line 1 \"");
}

class Setup
{
  func Setup()
  {
      cin.tie(0);
      ios.sync_with_stdio(false);
      write(fixed, setprecision(15));
    }
}

var Setup: dynamic;

func ALL(v: dynamic)
{
  return cpp_expression("#line 1 \"/home/siro53/");
}

func RALL(v: dynamic)
{
  return cpp_expression("#line 1 \"/home/siro53/ky");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i = (a); i < int(b); i++)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#line 1 \"/ho");
}

var INF = (1 << 30);

var LLINF = (1 << 60);

var MOD = 1000000007;

var dx = [1, 0, -1, 0];

var dy = [0, 1, 0, -1];

func solve()
{
  var N: dynamic;
  read(N);
  var a = cpp_construct((2 * N));
  REP(i, (2 * N));
  read(a[i]);
  var max_id = (max_element(ALL(a)) - a.begin());
  debug(N);
  REP(other_id, (N * 2));
  {
    if ((other_id == max_id))
    {
      continue;
    }
    var x = a[max_id];
    var se: dynamic;
    REP(i, (N * 2));
    {
      if (cpp_binary((i == max_id), "or", (i == other_id)))
      {
        continue;
      }
      se.insert(a[i]);
    }
    var ok = true;
    var ans: dynamic;
    ans.emplace_back(a[max_id], a[other_id]);
    debug(a[max_id], a[other_id]);
    while ((!se.empty()))
    {
      var f = false;
      {
        var it = prev(se.end());
        while ((it != se.begin()))
        {
          var it2 = se.find((x - (*it)));
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
          var v1 = (*it);
          var v2 = (*it2);
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

func main()
{
  var t = 1;
  read(t);
  REP(please_give_me_ac, t);
  solve();
}
