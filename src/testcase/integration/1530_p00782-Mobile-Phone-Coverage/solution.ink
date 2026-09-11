// Translated from solution.cpp.

func loop(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i<b;i++)");
}

func rep(i: dynamic, a: dynamic) -> dynamic
{
  return cpp_expression("#include<io");
}

var pb: dynamic = cpp_expression("#include<");

var mp: dynamic = cpp_expression("#include<");

func all(in_cpp: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream>");
}

func shosu(x: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream> #in");
}

var PI: dynamic = acos(-1);

var EPS: dynamic = 1e-8;

var inf: dynamic = 1e8;

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var co: dynamic = 0;
  while (cpp_comma((cin >> n), n))
  {
    co += 1;
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    var x1: dynamic = cpp_uninitialized();
    var x2: dynamic = cpp_uninitialized();
    var y1: dynamic = cpp_uninitialized();
    var y2: dynamic = cpp_uninitialized();
    var in_cpp: dynamic = cpp_construct(210, vi(210));
    sort(all(x));
    sort(all(y));
    x.erase(unique(all(x)), x.end());
    y.erase(unique(all(y)), y.end());
    var sum: dynamic = 0;
    rep(i, 210);
    rep(j, 210);
    if (in_cpp[i][j])
    {
      sum += (((x[(i + 1)] - x[i])) * ((y[(j + 1)] - y[j])));
    }
    write(co, " ", shosu(2), sum, "\n");
  }
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      var c: dynamic = cpp_uninitialized();
      read(a, b, c);
      x.pb((a - c));
      x.pb((a + c));
      y.pb((b - c));
      y.pb((b + c));
      x1.pb((a - c));
      x2.pb((a + c));
      y1.pb((b - c));
      y2.pb((b + c));
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var a: dynamic = (find(all(x), x1[i]) - x.begin());
      var b: dynamic = (find(all(x), x2[i]) - x.begin());
      var c: dynamic = (find(all(y), y1[i]) - y.begin());
      var d: dynamic = (find(all(y), y2[i]) - y.begin());
      loop(j, a, b);
      loop(k, c, d)[j][k] = true;
    }
