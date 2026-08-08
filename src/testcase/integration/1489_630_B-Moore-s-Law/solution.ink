// Translated from solution.cpp.

var n: dynamic;

var i: dynamic;

var j: dynamic;

var sum: dynamic;

var lsbl: dynamic;

var t: dynamic;

var lsbl1: dynamic;

func ksm(a: dynamic, b: dynamic)
{
  var sumend = 1;
  while (b)
  {
    if (((b % 2) == 1))
    {
      sumend *= a;
    }
    a *= a;
    b /= 2;
  }
  return sumend;
}

func main()
{
  read(n, t);
  lsbl1 = (n * ksm(1.000000011, t));
  write(fixed, setprecision(15), (n * ksm(1.000000011, t)), "\n");
  return 0;
}
