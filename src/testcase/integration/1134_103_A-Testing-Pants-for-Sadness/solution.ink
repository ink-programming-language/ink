// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var t: dynamic;
  var res = 0;
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(t);
      res += (((t * i)) - ((i - 1)));
      i += 1;
    }
  }
  write(res, "\n");
}
