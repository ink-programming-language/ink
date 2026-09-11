// Translated from solution.cpp.

func Cal(a: dynamic, b: dynamic) -> dynamic
{
  if ((a == 1))
  {
    return b;
  }
  if (((a % b) == 0))
  {
    return (a / b);
  }
  var res: dynamic = 0;
  if ((a > b))
  {
    var t: dynamic = (a / b);
    res += t;
    res += Cal((a - (b * t)), b);
  } else
  {
    res += Cal(b, a);
  }
  return res;
}

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(a, b);
  write(Cal(a, b), "\n");
}
