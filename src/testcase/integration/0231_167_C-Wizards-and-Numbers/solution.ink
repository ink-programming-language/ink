// Translated from solution.cpp.

func check(a: dynamic, b: dynamic) -> dynamic
{
  if (((!a) || (!b)))
  {
    return false;
  }
  if ((a > b))
  {
    swap(a, b);
  }
  if ((!check(a, (b % a))))
  {
    return true;
  }
  return (!((((((b / a)) % ((a + 1)))) & 1)));
}

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
  read(T);
  {
    while (T)
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b);
      printf("%s\n",  ((check(a, b))) ? "First" : "Second");
      T -= 1;
    }
  }
}
