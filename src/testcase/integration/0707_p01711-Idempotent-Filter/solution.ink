// Translated from solution.cpp.

func ALL(a: dynamic)
{
  return cpp_expression("//include //---------");
}

func RALL(a: dynamic)
{
  return cpp_expression("//include //------------");
}

var PB = cpp_expression("//include");

var MP = cpp_expression("//include");

func SZ(a: dynamic)
{
  return cpp_expression("//include //---");
}

func EACH(i: dynamic, c: dynamic)
{
  cpp_macro("for(typeof((c).begin()) i=(c).begin(); i!=(c).end(); ++i)");
}

func EXIST(s: dynamic, e: dynamic)
{
  return cpp_expression("//include //------------");
}

func SORT(c: dynamic)
{
  return cpp_expression("//include //---------------");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("//include");
}

var EPS = 1e-10;

var PI = acos(-1.0);

var filt = cpp_array(128);

var crd = [[13, 12, 14, 0, 1, 2, 3], [12, 11, 0, 1, 10, 3, 4], [14, 0, 15, 2, 3, 16, 5], [0, 1, 2, 3, 4, 5, 6], [1, 10, 3, 4, 9, 6, 8], [2, 3, 16, 5, 6, 17, 18], [3, 4, 5, 6, 8, 18, 7]];

func next(bit: dynamic)
{
  var ret = bit;
  {
    var i = 0;
    while ((i < 7))
    {
      var tmp = 0;
      {
        var j = 0;
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

func main()
{
  cin.tie(0);
  ios_base.sync_with_stdio(false);
  var s: dynamic;
  while (cpp_comma((cin >> s), (s != "#")))
  {
    {
      var i = 0;
      while ((i < 128))
      {
        filt[i] = (s[i] - cpp_char("0"));
        i += 1;
      }
    }
    var ok = true;
    {
      var b = 0;
      while ((b < ((1 << 19))))
      {
        var n = next(b);
        if (((((n >> 3) & 1)) != (((next(n) >> 3) & 1))))
        {
          ok = false;
          break;
        }
        b += 1;
      }
    }
    write((if (ok) "yes" else "no"), "\n");
  }
  return 0;
}
