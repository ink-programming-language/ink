// Translated from solution.cpp.

func main()
{
  var x: dynamic;
  var c: dynamic;
  var ans = 0;
  read(x);
  {
    c = 1;
    while (((100 * c) <= x))
    {
      if (((105 * c) >= x))
      {
        ans = 1;
        break;
      }
      c += 1;
    }
  }
  write(ans);
}
