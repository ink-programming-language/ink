// Translated from solution.cpp.

var EPS = 1e-8;

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(int)(n);++i)");
}

var F = cpp_expression("#incl");

var S = cpp_expression("#inclu");

func mp(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <bits");
}

func pb(a: dynamic)
{
  return cpp_expression("#include <bi");
}

func SZ(a: dynamic)
{
  return cpp_expression("#include <bits/st");
}

func ALL(a: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

func RALL(a: dynamic)
{
  return cpp_expression("#include <bits/stdc");
}

func FLL(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h");
}

func CLR(a: dynamic)
{
  return cpp_expression("#include");
}

func declare(a: dynamic, it: dynamic)
{
  return cpp_expression("#include <bits/std");
}

func FOR(it: dynamic, a: dynamic)
{
  cpp_macro("for(declare(a.begin(),it);it!=a.end();++it)");
}

func FORR(it: dynamic, a: dynamic)
{
  cpp_macro("for(declare(a.rbegin(),it);it!=a.rend();++it)");
}

func operator_shift_left(o: dynamic, v: dynamic)
{
  return (((((o << "(") << v.F) << ", ") << v.S) << ")");
}

func operator_shift_left(o: dynamic, v: dynamic)
{
  (o << "{");
  ((rep(i, SZ(v)) << (if (i) ", " else "")) << v[i]);
  return (o << "}");
}

var dx = [0, 1, 0, -1, 1, 1, -1, -1];

var dy = [1, 0, -1, 0, -1, 1, 1, -1];

func s2i(a: dynamic)
{
  var r: dynamic;
  (ss >> r);
  return r;
}

func geti()
{
  var n: dynamic;
  scanf("%d", (&n));
  return n;
}

var x = cpp_array(100);

var y = cpp_array(100);

func main(argc: dynamic, argv: dynamic)
{
  var n: dynamic;
  read(n);
  var sumx = 0;
  var sumy = 0;
  {
    var i = 0;
    while ((i < n))
    {
      read(x[i], y[i]);
      sumx += x[i];
      sumy += y[i];
      i += 1;
    }
  }
  sumx /= n;
  sumy /= n;
  var ok = true;
  {
    var i = 0;
    while ((i < n))
    {
      var ax = (x[(((i + 1)) % n)] - x[i]);
      var ay = (y[(((i + 1)) % n)] - y[i]);
      var bx = (x[((((i + 1) + (n / 2))) % n)] - x[(((i + (n / 2))) % n)]);
      var by = (y[((((i + 1) + (n / 2))) % n)] - y[(((i + (n / 2))) % n)]);
      ok &= (abs(((ax * by) - (bx * ay))) < EPS);
      ok &= (abs((((ax * ax) + (ay * ay)) - (((bx * bx) + (by * by))))) < EPS);
      i += 1;
    }
  }
  if ((ok && ((~n) % 2)))
  {
    printf("%.10f %.10f\n", sumx, sumy);
  } else
  {
    write("NA", "\n");
  }
  return 0;
}
