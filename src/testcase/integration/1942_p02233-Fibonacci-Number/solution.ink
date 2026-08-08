// Translated from solution.cpp.

func main()
{
  var MAX = 46;
  var f = [1, 1];
  {
    var i = 2;
    while ((i < MAX))
    {
      f[i] = (f[(i - 1)] + f[(i - 2)]);
      i += 1;
    }
  }
  var n: dynamic;
  read(n);
  write(f[n], "\n");
  return 0;
}
