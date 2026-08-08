// Translated from solution.cpp.

var ax: dynamic;

var ay: dynamic;

var bx: dynamic;

var by: dynamic;

var cx: dynamic;

var cy: dynamic;

func valid()
{
  return (((((ax * ((by - cy)))) + ((bx * ((cy - ay))))) + ((cx * ((ay - by))))) != 0);
}

func main()
{
  while ((scanf("%lld %lld %lld %lld %lld %lld", (&ax), (&ay), (&bx), (&by), (&cx), (&cy)) == 6))
  {
    var ab = (((((ax - bx)) * ((ax - bx)))) + ((((ay - by)) * ((ay - by)))));
    var bc = (((((bx - cx)) * ((bx - cx)))) + ((((by - cy)) * ((by - cy)))));
    puts(if ((valid() && (ab == bc))) "Yes" else "No");
  }
  return 0;
}
