// Translated from solution.cpp.

func Cal(a: dynamic, b: dynamic)
{
  if ((a == 1))
  {
    return b;
  }
  if (((a % b) == 0))
  {
    return (a / b);
  }
  var res = 0;
  if ((a > b))
  {
    var t = (a / b);
    res += t;
    res += Cal((a - (b * t)), b);
  } else
  {
    res += Cal(b, a);
  }
  return res;
}

func main()
{
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  write(Cal(a, b), "\n");
}
