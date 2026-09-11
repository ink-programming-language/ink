// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(a, b);
  x = min(a, b);
  var fact: dynamic = 1;
  while ((x > 1))
  {
    fact = (fact * x);
    x -= 1;
  }
  write(fact);
  return (0);
}
