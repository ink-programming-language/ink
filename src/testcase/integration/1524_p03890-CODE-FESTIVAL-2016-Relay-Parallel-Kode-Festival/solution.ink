// Translated from solution.cpp.

var GLIBCXX_DEBUG = cpp_expression("#ifd");

var NDEBUG = cpp_expression("#ifd");

var INF = 1e9;

func operator_shift_left(os: dynamic, p: dynamic)
{
  write(cpp_char("("), p.first, cpp_char(" "), p.second, cpp_char(")"));
  return os;
}

var endl = cpp_expression("#ifd");

func ALL(a: dynamic)
{
  return cpp_expression("#ifdef LOCAL111 #def");
}

func SZ(a: dynamic)
{
  return cpp_expression("#ifdef LOCAL111");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func RFOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for (int i=(b)-1;i>=(a);i--)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#ifdef LOC");
}

func RREP(i: dynamic, n: dynamic)
{
  cpp_macro("for (int i=(n)-1;i>=0;i--)");
}

func DEBUG(x: dynamic)
{
  return cpp_expression("#ifdef LOCAL111 #define");
}

func dpite(a: dynamic, b: dynamic)
{
  {
    var ite = a;
    while ((ite != b))
    {
      write((if ((ite == a)) "" else " "), (*ite));
      ite += 1;
    }
  }
  write("\n");
}

func DEBUG(x: dynamic)
{
  return cpp_expression("#ifd");
}

func dpite(a: dynamic, b: dynamic)
{
  return;
}

var F = cpp_expression("#ifde");

var S = cpp_expression("#ifdef");

var SNP = cpp_expression("#ifdef LOCAL");

func WRC(hoge: dynamic)
{
  return cpp_expression("#ifdef LOCAL111 #define _GLIBCXX_DE");
}

func pite(a: dynamic, b: dynamic)
{
  {
    var ite = a;
    while ((ite != b))
    {
      write((if ((ite == a)) "" else " "), (*ite));
      ite += 1;
    }
  }
  write("\n");
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return true;
  }
  return false;
}

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
    return true;
  }
  return false;
}

func ios_init()
{
  return;
  ios.sync_with_stdio(false);
  cin.tie(0);
}

func main()
{
  ios_init();
  var n: dynamic;
  while ((cin >> n))
  {
    var a = cpp_construct((1 << n));
    REP(i, (1 << n));
    {
      read(a[i]);
    }
    var v: dynamic;
    while ((SZ(a) > 1))
    {
      dpite(ALL(a));
      v.clear();
      REP(i, (SZ(a) / 2));
      {
        v.push_back(abs((a[(i * 2)] - a[((i * 2) + 1)])));
      }
      swap(a, v);
    }
    write(a[0], "\n");
  }
  return 0;
}
