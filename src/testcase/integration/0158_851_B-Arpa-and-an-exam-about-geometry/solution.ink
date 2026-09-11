// Translated from solution.cpp.

var ax: dynamic = cpp_uninitialized();

var ay: dynamic = cpp_uninitialized();

var bx: dynamic = cpp_uninitialized();

var by: dynamic = cpp_uninitialized();

var cx: dynamic = cpp_uninitialized();

var cy: dynamic = cpp_uninitialized();

func valid() -> dynamic
{
  return (((((ax * ((by - cy)))) + ((bx * ((cy - ay))))) + ((cx * ((ay - by))))) != 0);
}

func main() -> dynamic
{
  while ((scanf("%lld %lld %lld %lld %lld %lld", (&ax), (&ay), (&bx), (&by), (&cx), (&cy)) == 6))
  {
    var ab: dynamic = (((((ax - bx)) * ((ax - bx)))) + ((((ay - by)) * ((ay - by)))));
    var bc: dynamic = (((((bx - cx)) * ((bx - cx)))) + ((((by - cy)) * ((by - cy)))));
    puts( ((valid() && (ab == bc))) ? "Yes" : "No");
  }
  return 0;
}
