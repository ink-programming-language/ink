// Translated from solution.cpp.

func loop(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i<b;i++)");
}

func rep(i: dynamic, a: dynamic)
{
  return cpp_expression("#include<io");
}

var pb = cpp_expression("#include<");

var mp = cpp_expression("#include<");

func all(in_cpp: dynamic)
{
  return cpp_expression("#include<iostream>");
}

func shosu(x: dynamic)
{
  return cpp_expression("#include<iostream> #in");
}

var PI = acos(-1);

var EPS = 1e-8;

var inf = 1e8;

func main()
{
  var n: dynamic;
  var co = 0;
  while (cpp_comma((cin >> n), n))
  {
    co += 1;
    var x: dynamic;
    var y: dynamic;
    var x1: dynamic;
    var x2: dynamic;
    var y1: dynamic;
    var y2: dynamic;
    var in_cpp = cpp_construct(210, vi(210));
    sort(all(x));
    sort(all(y));
    x.erase(unique(all(x)), x.end());
    y.erase(unique(all(y)), y.end());
    var sum = 0;
    rep(i, 210);
    rep(j, 210);
    if (in_cpp[i][j])
    {
      sum += (((x[(i + 1)] - x[i])) * ((y[(j + 1)] - y[j])));
    }
    write(co, " ", shosu(2), sum, "\n");
  }
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var a: dynamic;
      var b: dynamic;
      var c: dynamic;
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

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var a = (find(all(x), x1[i]) - x.begin());
      var b = (find(all(x), x2[i]) - x.begin());
      var c = (find(all(y), y1[i]) - y.begin());
      var d = (find(all(y), y2[i]) - y.begin());
      loop(j, a, b);
      loop(k, c, d)[j][k] = true;
    }
