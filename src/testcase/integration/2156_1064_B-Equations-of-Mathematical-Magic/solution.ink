// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic;
    read(a);
    var ans = 0;
    while ((a > 0))
    {
      ans += (a % 2);
      a /= 2;
    }
    ans = pow(2, ans);
    write(ans, "\n");
  }
  return 0;
}
