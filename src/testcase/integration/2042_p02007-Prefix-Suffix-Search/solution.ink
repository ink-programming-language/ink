// Translated from solution.cpp.

func lp(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(int)n;i++)");
}

class RollingHash
{
  var hashed: dynamic = cpp_uninitialized();
  var power: dynamic = cpp_uninitialized();
  func mul(a: dynamic, b: dynamic) -> dynamic
  {
      var x: dynamic = (cpp_cast(a) * b);
      var xh: dynamic = unsigned((x >> 32));
      var xl: dynamic = cpp_cast(x);
      var d: dynamic = cpp_uninitialized();
      var m: dynamic = cpp_uninitialized();
      cpp_expression("asm(\"divl %4; \\n\\t\" : \"=a\"(d),\"=d\"(m):\"d\"(xh),\"a\"(xl),\"r\"(mod))");
      return m;
    }
  func RollingHash(s: dynamic, base: dynamic = 10007) -> dynamic
  {
      var sz: dynamic = cpp_cast(s.size());
      hashed.assign((sz + 1), 0);
      power.assign((sz + 1), 0);
      power[0] = 1;
      {
        var i: dynamic = 0;
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
  func get(l: dynamic, r: dynamic) -> dynamic
  {
      var ret: dynamic = ((hashed[r] + mod) - mul(hashed[l], power[(r - l)]));
      if ((ret >= mod))
      {
        ret -= mod;
      }
      return ret;
    }
}

var int_cpp: dynamic = dynamic;

func calc(v: dynamic, s: dynamic) -> dynamic
{
  var l: dynamic = (lower_bound(v.begin(), v.end(), s) - v.begin());
  s[(s.size() - 1)] += 1;
  var r: dynamic = (lower_bound(v.begin(), v.end(), s) - v.begin());
  return abs((l - r));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, q);
  var m: dynamic = cpp_uninitialized();
  var ch: dynamic = cpp_uninitialized();
}

func lp(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var s: dynamic = cpp_uninitialized();
    read(s);
    lp(i, s.size());
    {
      m[[cpp_cast(rh.get(i, s.size())), cpp_cast(sh.get(i, s.size()))]].push_back(s);
    }
  }

func lp(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    read(a, b);
    var hs: dynamic = rh.get(0, b.size());
    var ss: dynamic = sh.get(0, b.size());
    var v: dynamic = m[[hs, ss]];
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
