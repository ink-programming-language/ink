// Translated from solution.cpp.

func lp(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(int)n;i++)");
}

class RollingHash
{
  var hashed: dynamic;
  var power: dynamic;
  func mul(a: dynamic, b: dynamic)
  {
      var x = (cpp_cast(a) * b);
      var xh = unsigned((x >> 32));
      var xl = cpp_cast(x);
      var d: dynamic;
      var m: dynamic;
      cpp_expression("asm(\"divl %4; \\n\\t\" : \"=a\"(d),\"=d\"(m):\"d\"(xh),\"a\"(xl),\"r\"(mod))");
      return m;
    }
  func RollingHash(s: dynamic, base: dynamic = 10007)
  {
      var sz = cpp_cast(s.size());
      hashed.assign((sz + 1), 0);
      power.assign((sz + 1), 0);
      power[0] = 1;
      {
        var i = 0;
        while ((i < sz))
        {
          power[(i + 1)] = mul(power[i], base);
          hashed[(i + 1)] = (mul(hashed[i], base) + s[i]);
          if ((hashed[(i + 1)] >= mod))
          {
            hashed[(i + 1)] -= mod;
          }
          i += 1;
        }
      }
    }
  func get(l: dynamic, r: dynamic)
  {
      var ret = ((hashed[r] + mod) - mul(hashed[l], power[(r - l)]));
      if ((ret >= mod))
      {
        ret -= mod;
      }
      return ret;
    }
}

var int_cpp = dynamic;

func calc(v: dynamic, s: dynamic)
{
  var l = (lower_bound(v.begin(), v.end(), s) - v.begin());
  s[(s.size() - 1)] += 1;
  var r = (lower_bound(v.begin(), v.end(), s) - v.begin());
  return abs((l - r));
}

func main()
{
  var n: dynamic;
  var q: dynamic;
  read(n, q);
  var m: dynamic;
  var ch: dynamic;
}

func lp(argument_0: dynamic, argument_1: dynamic)
{
    var s: dynamic;
    read(s);
    lp(i, s.size());
    {
      m[[cpp_cast(rh.get(i, s.size())), cpp_cast(sh.get(i, s.size()))]].push_back(s);
    }
  }

func lp(argument_0: dynamic, argument_1: dynamic)
{
    var a: dynamic;
    var b: dynamic;
    read(a, b);
    var hs = rh.get(0, b.size());
    var ss = sh.get(0, b.size());
    var v = m[[hs, ss]];
    if (v.empty())
    {
      write(0, "\n");
      continue;
    }
    if ((ch.find(hs) == ch.end()))
    {
      sort(v.begin(), v.end());
      ch.insert(hs);
    }
    write(calc(v, a), "\n");
  }
