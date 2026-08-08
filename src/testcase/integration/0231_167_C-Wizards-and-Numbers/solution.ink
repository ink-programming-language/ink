// Translated from solution.cpp.

func check(a: dynamic, b: dynamic)
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

func main()
{
  var T: dynamic;
  read(T);
  {
    while (T)
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      printf("%s\n", if ((check(a, b))) "First" else "Second");
      T -= 1;
    }
  }
}
