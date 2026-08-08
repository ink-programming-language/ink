// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var ans = 0;
  read(n);
  {
    var i = 0;
    var a: dynamic;
    while ((i < n))
    {
      read(a);
      ans += (a - 1);
      i += 1;
    }
  }
  write(ans, "\n");
}
