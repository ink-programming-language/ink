// Translated from solution.cpp.

var MAX_N: dynamic = cpp_expression("#includ");

var mp: dynamic = cpp_expression("#include<");

var pb: dynamic = cpp_expression("#include<");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

var INF: dynamic = cpp_expression("#include<");

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i = 0;i < n;i++)");
}

func gcm(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    swap(a, b);
  }
  while ((b > 0))
  {
    a %= b;
    swap(a, b);
  }
  return a;
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return ((a / gcm(a, b)) * b);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var LCM: dynamic = cpp_uninitialized();
  var GCM: dynamic = cpp_uninitialized();
  var flag: dynamic = cpp_array(MAX_N);
  var count: dynamic = cpp_array(MAX_N);
  while (1)
  {
    read(n, a, b);
    if ((n == 0))
    {
      break;
    }
    LCM = lcm(a, b);
    GCM = gcm(a, b);
    var ans: dynamic = cpp_uninitialized();
    var irange: dynamic = ((min((MAX_N - 1), LCM) / a) + 1);
    count[0] = 0;
    rep(i, (MAX_N - 1));
    {
      count[(i + 1)] = (count[i] + flag[(i + 1)]);
    }
    if ((n < LCM))
    {
      ans = count[n];
    } else
    {
      ans = (count[LCM] + (((n - LCM)) / GCM));
    }
    write((n - ans), "\n");
  }
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      flag[i] = 0;
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        flag[((a * i) + (b * j))] = true;
      }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var jrange: dynamic = ((((min((MAX_N - 1), LCM) - (a * i))) / b) + 1);
    }
