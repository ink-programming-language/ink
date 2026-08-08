// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var n: dynamic;
  while ((((cin >> a) >> b) >> n))
  {
    var ans = 0;
    a = (10 * ((a % b)));
    while (cpp_update(n, "--"))
    {
      ans += (a / b);
      a = (10 * ((a % b)));
    }
    write(ans, "\n");
  }
  return 0;
}
