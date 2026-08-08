// Translated from solution.cpp.

var MAX_N = cpp_expression("#includ");

var mp = cpp_expression("#include<");

var pb = cpp_expression("#include<");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

var INF = cpp_expression("#include<");

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = 0;i < n;i++)");
}

func gcm(a: dynamic, b: dynamic)
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

func lcm(a: dynamic, b: dynamic)
{
  return ((a / gcm(a, b)) * b);
}

func main()
{
  var n: dynamic;
  var a: dynamic;
  var b: dynamic;
  var LCM: dynamic;
  var GCM: dynamic;
  var flag = cpp_array(MAX_N);
  var count = cpp_array(MAX_N);
  while (1)
  {
    read(n, a, b);
    if ((n == 0))
    {
      break;
    }
    LCM = lcm(a, b);
    GCM = gcm(a, b);
    var ans: dynamic;
    var irange = ((min((MAX_N - 1), LCM) / a) + 1);
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

func rep(argument_0: dynamic, argument_1: dynamic)
{
      flag[i] = 0;
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
        flag[((a * i) + (b * j))] = true;
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var jrange = ((((min((MAX_N - 1), LCM) - (a * i))) / b) + 1);
    }
