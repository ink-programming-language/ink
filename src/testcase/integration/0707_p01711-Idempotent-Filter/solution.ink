// Translated from solution.cpp.

func ALL(a: dynamic) -> dynamic
{
  return cpp_expression("//include //---------");
}

func RALL(a: dynamic) -> dynamic
{
  return cpp_expression("//include //------------");
}

var PB: dynamic = cpp_expression("//include");

var MP: dynamic = cpp_expression("//include");

func SZ(a: dynamic) -> dynamic
{
  return cpp_expression("//include //---");
}

func EACH(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(typeof((c).begin()) i=(c).begin(); i!=(c).end(); ++i)");
}

func EXIST(s: dynamic, e: dynamic) -> dynamic
{
  return cpp_expression("//include //------------");
}

func SORT(c: dynamic) -> dynamic
{
  return cpp_expression("//include //---------------");
}

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("//include");
}

var EPS: dynamic = 1e-10;

var PI: dynamic = acos(-1.0);

var filt: dynamic = cpp_array(128);

var crd: dynamic = [[13, 12, 14, 0, 1, 2, 3], [12, 11, 0, 1, 10, 3, 4], [14, 0, 15, 2, 3, 16, 5], [0, 1, 2, 3, 4, 5, 6], [1, 10, 3, 4, 9, 6, 8], [2, 3, 16, 5, 6, 17, 18], [3, 4, 5, 6, 8, 18, 7]];

func next(bit: dynamic) -> dynamic
{
  var ret: dynamic = bit;
  {
    var i: dynamic = 0;
    while ((i < 7))
    {
      var tmp: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((j < 7))
        {
          tmp |= (((((bit >> crd[i][j])) & 1)) << j);
          ret = (((ret & (~((1 << i))))) | ((filt[tmp] << i)));
          j += 1;
        }
      }
      i += 1;
    }
  }
  return ret;
}

func main() -> dynamic
{
  cin.tie(0);
  ios_base.sync_with_stdio(false);
  var s: dynamic = cpp_uninitialized();
  while (cpp_comma((cin >> s), (s != "#")))
  {
    {
      var i: dynamic = 0;
      while ((i < 128))
      {
        filt[i] = (s[i] - cpp_char("0"));
        i += 1;
      }
    }
    var ok: dynamic = true;
    {
      var b: dynamic = 0;
      while ((b < ((1 << 19))))
      {
        var n: dynamic = next(b);
        if (((((n >> 3) & 1)) != (((next(n) >> 3) & 1))))
        {
          ok = false;
          break;
        }
        b += 1;
      }
    }
    write(( (ok) ? "yes" : "no"), "\n");
  }
  return 0;
}
