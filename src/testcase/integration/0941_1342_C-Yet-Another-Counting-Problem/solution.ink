// Translated from solution.cpp.

func MACRO_VAR_Scan(t: dynamic) -> dynamic
{
  read(t);
}

func MACRO_VAR_Scan(first: dynamic, rest: dynamic...) -> dynamic
{
  read(first);
  MACRO_VAR_Scan(cpp_expand(rest));
}

func MACRO_VEC_ROW_Init(n: dynamic, t: dynamic) -> dynamic
{
  t.resize(n);
}

func MACRO_VEC_ROW_Init(n: dynamic, first: dynamic, rest: dynamic...) -> dynamic
{
  first.resize(n);
  MACRO_VEC_ROW_Init(n, cpp_expand(rest));
}

func MACRO_VEC_ROW_Scan(p: dynamic, t: dynamic) -> dynamic
{
  read(t[p]);
}

func MACRO_VEC_ROW_Scan(p: dynamic, first: dynamic, rest: dynamic...) -> dynamic
{
  read(first[p]);
  MACRO_VEC_ROW_Scan(p, cpp_expand(rest));
}

func MACRO_OUT(t: dynamic) -> dynamic
{
  write(t);
}

func MACRO_OUT(first: dynamic, rest: dynamic...) -> dynamic
{
  write(first, " ");
  MACRO_OUT(cpp_expand(rest));
}

func IN(a: dynamic, x: dynamic, b: dynamic) -> dynamic
{
  return ((a <= x) && (x < b));
}

func CHMAX(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=",  (((a < b))) ? b : a);
}

func CHMIN(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=",  (((a > b))) ? b : a);
}

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  (((((os << "(") << p.first) << ", ") << p.second) << ")");
  return os;
}

var INFINT: dynamic = (((1 << 30)) - 1);

var INFINT_LIM: dynamic = (((1 << 31)) - 1);

var INFLL: dynamic = (1 << 60);

var INFLL_LIM: dynamic = ((((1 << 62)) - 1) + ((1 << 62)));

var eps: dynamic = 1e-6;

var MOD: dynamic = 1000000007;

var PI: dynamic = 3.141592653589793238462643383279;

func FILL(a: dynamic, val: dynamic) -> dynamic
{
  for (var x: dynamic in a)
  {
    x = val;
  }
}

func FILL(a: dynamic, val: dynamic) -> dynamic
{
  for (var b: dynamic in a)
  {
    FILL(b, val);
  }
}

func FILL(a: dynamic, val: dynamic) -> dynamic
{
  for (var x: dynamic in a)
  {
    x = val;
  }
}

func FILL(a: dynamic, val: dynamic) -> dynamic
{
  for (var b: dynamic in a)
  {
    FILL(b, val);
  }
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (b) ? gcd(b, (a % b)) : a;
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return ((a / gcd(a, b)) * b);
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var Q: dynamic = cpp_uninitialized();
  MACRO_VAR_Scan(Q);
  {
    var cpp_name: dynamic = (0);
    while ((cpp_name < (Q)))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      var q: dynamic = cpp_uninitialized();
      MACRO_VAR_Scan(a, b, q);
      if ((a > b))
      {
        swap(a, b);
      }
      var L: dynamic = lcm(a, b);
      var f: dynamic = __cpp_lambda_1;
      {
        var i: dynamic = (0);
        while ((i < (q)))
        {
          var l: dynamic = cpp_uninitialized();
          var r: dynamic = cpp_uninitialized();
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

func __cpp_lambda_1(x: dynamic) -> dynamic
{
  var res: dynamic = 0;
  var q: dynamic = (x / L);
  var r: dynamic = (x % L);
  res += (q * ((L - b)));
  res += max(cpp_cast(0), ((r - b) + 1));
  return res;
}
