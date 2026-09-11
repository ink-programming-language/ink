// Translated from solution.cpp.

var EPS: dynamic = cpp_expression("#includ");

func EQ(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream> #");
}

func EQV(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream> #include<cmath> #include<cstdio> #include<comp");
}

func abs(a: dynamic, b: dynamic) -> dynamic
{
  return sqrt(((((a.real() - b.real())) * ((a.real() - b.real()))) + (((a.imag() - b.imag())) * ((a.imag() - b.imag())))));
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return (((a.real() * b.imag()) - (a.imag() * b.real())));
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return (((a.real() * b.real()) + (a.imag() * b.imag())));
}

func is_parallel(a1: dynamic, a2: dynamic, b1: dynamic, b2: dynamic) -> dynamic
{
  return EQ(cross((a1 - a2), (b1 - b2)), 0.0);
}

func distance_ls_p(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  if ((dot((b - a), (c - a)) < EPS))
  {
    return abs((c - a));
  }
  if ((dot((a - b), (c - b)) < EPS))
  {
    return abs((c - b));
  }
  return (abs(cross((b - a), (c - a))) / abs((b - a)));
}

func main() -> dynamic
{
  var D: dynamic = cpp_uninitialized();
  while (true)
  {
    read(D);
    if ((D == 0))
    {
      break;
    }
    var vx: dynamic = cpp_uninitialized();
    var vy: dynamic = cpp_uninitialized();
    var px: dynamic = cpp_uninitialized();
    var py: dynamic = cpp_uninitialized();
    var st: dynamic = cpp_uninitialized();
    read(px, py, vx, vy);
    var as_cpp: dynamic = Point(px, py);
    var g: dynamic = Point(0, 0);
    var at: dynamic = Point((px + vx), (py + vy));
    if ((!is_parallel(as_cpp, g, at, as_cpp)))
    {
      cpp_goto("goto NO;");
    }
    st = distance_ls_p(as_cpp, at, g);
    if ((EPS < (abs(as_cpp, g) - st)))
    {
      if ((abs(as_cpp, g) > D))
      {
        cpp_goto("goto NO;");
      }
      printf("%.10lf\n", abs(as_cpp, g));
    } else if ((abs((abs(as_cpp, g) - st)) < EPS))
    {
      if (((2 - abs(as_cpp, g)) > D))
      {
        cpp_goto("goto NO;");
      }
      printf("%.10lf\n", (2 - abs(as_cpp, g)));
    }
    continue;
    printf("impossible\n");
  }
}
