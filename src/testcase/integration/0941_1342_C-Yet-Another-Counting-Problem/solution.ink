// Translated from solution.cpp.

func MACRO_VAR_Scan(t: dynamic)
{
  read(t);
}

func MACRO_VAR_Scan(first: dynamic, rest: dynamic...)
{
  read(first);
  MACRO_VAR_Scan(cpp_expand(rest));
}

func MACRO_VEC_ROW_Init(n: dynamic, t: dynamic)
{
  t.resize(n);
}

func MACRO_VEC_ROW_Init(n: dynamic, first: dynamic, rest: dynamic...)
{
  first.resize(n);
  MACRO_VEC_ROW_Init(n, cpp_expand(rest));
}

func MACRO_VEC_ROW_Scan(p: dynamic, t: dynamic)
{
  read(t[p]);
}

func MACRO_VEC_ROW_Scan(p: dynamic, first: dynamic, rest: dynamic...)
{
  read(first[p]);
  MACRO_VEC_ROW_Scan(p, cpp_expand(rest));
}

func MACRO_OUT(t: dynamic)
{
  write(t);
}

func MACRO_OUT(first: dynamic, rest: dynamic...)
{
  write(first, " ");
  MACRO_OUT(cpp_expand(rest));
}

func IN(a: dynamic, x: dynamic, b: dynamic)
{
  return ((a <= x) && (x < b));
}

func CHMAX(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "=", if (((a < b))) b else a);
}

func CHMIN(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "=", if (((a > b))) b else a);
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  (((((os << "(") << p.first) << ", ") << p.second) << ")");
  return os;
}

var INFINT = (((1 << 30)) - 1);

var INFINT_LIM = (((1 << 31)) - 1);

var INFLL = (1 << 60);

var INFLL_LIM = ((((1 << 62)) - 1) + ((1 << 62)));

var eps = 1e-6;

var MOD = 1000000007;

var PI = 3.141592653589793238462643383279;

func FILL(a: dynamic, val: dynamic)
{
  for (var x in a)
  {
    x = val;
  }
}

func FILL(a: dynamic, val: dynamic)
{
  for (var b in a)
  {
    FILL(b, val);
  }
}

func FILL(a: dynamic, val: dynamic)
{
  for (var x in a)
  {
    x = val;
  }
}

func FILL(a: dynamic, val: dynamic)
{
  for (var b in a)
  {
    FILL(b, val);
  }
}

func gcd(a: dynamic, b: dynamic)
{
  return if (b) gcd(b, (a % b)) else a;
}

func lcm(a: dynamic, b: dynamic)
{
  return ((a / gcd(a, b)) * b);
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var Q: dynamic;
  MACRO_VAR_Scan(Q);
  {
    var cpp_name = (0);
    while ((cpp_name < (Q)))
    {
      var a: dynamic;
      var b: dynamic;
      var q: dynamic;
      MACRO_VAR_Scan(a, b, q);
      if ((a > b))
      {
        swap(a, b);
      }
      var L = lcm(a, b);
      var f = __cpp_lambda_1;
      {
        var i = (0);
        while ((i < (q)))
        {
          var l: dynamic;
          var r: dynamic;
          MACRO_VAR_Scan(l, r);
          MACRO_OUT((f(r) - f((l - 1))));
          write(" ");
          i += 1;
        }
      }
      write("\n");
      cpp_name += 1;
    }
  }
  return 0;
}

func __cpp_lambda_1(x: dynamic)
{
  var res = 0;
  var q = (x / L);
  var r = (x % L);
  res += (q * ((L - b)));
  res += max(cpp_cast(0), ((r - b) + 1));
  return res;
}
