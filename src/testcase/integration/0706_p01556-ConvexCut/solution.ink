// Translated from solution.cpp.

var EPS: dynamic = 1e-8;

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(int)(n);++i)");
}

var F: dynamic = cpp_expression("#incl");

var S: dynamic = cpp_expression("#inclu");

func mp(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <bits");
}

func pb(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bi");
}

func SZ(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/st");
}

func ALL(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

func RALL(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc");
}

func FLL(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h");
}

func CLR(a: dynamic) -> dynamic
{
  return cpp_expression("#include");
}

func declare(a: dynamic, it: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/std");
}

func FOR(it: dynamic, a: dynamic) -> dynamic
{
  cpp_macro("for(declare(a.begin(),it);it!=a.end();++it)");
}

func FORR(it: dynamic, a: dynamic) -> dynamic
{
  cpp_macro("for(declare(a.rbegin(),it);it!=a.rend();++it)");
}

func operator_shift_left(o: dynamic, v: dynamic) -> dynamic
{
  return (((((o << "(") << v.F) << ", ") << v.S) << ")");
}

func operator_shift_left(o: dynamic, v: dynamic) -> dynamic
{
  (o << "{");
  ((rep(i, SZ(v)) << ( (i) ? ", " : "")) << v[i]);
  return (o << "}");
}

var dx: dynamic = [0, 1, 0, -1, 1, 1, -1, -1];

var dy: dynamic = [1, 0, -1, 0, -1, 1, 1, -1];

func s2i(a: dynamic) -> dynamic
{
  var r: dynamic = cpp_uninitialized();
  (ss >> r);
  return r;
}

func geti() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  return n;
}

var x: dynamic = cpp_array(100);

var y: dynamic = cpp_array(100);

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var sumx: dynamic = 0;
  var sumy: dynamic = 0;
  {
    var i: dynamic = 0;
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
  var ok: dynamic = true;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var ax: dynamic = (x[(((i + 1)) % n)] - x[i]);
      var ay: dynamic = (y[(((i + 1)) % n)] - y[i]);
      var bx: dynamic = (x[((((i + 1) + (n / 2))) % n)] - x[(((i + (n / 2))) % n)]);
      var by: dynamic = (y[((((i + 1) + (n / 2))) % n)] - y[(((i + (n / 2))) % n)]);
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
