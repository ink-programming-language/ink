// Translated from solution.cpp.

func main()
{
  var hh: dynamic;
  var mm: dynamic;
  var x: dynamic;
  var ans = 0;
  read(x, hh, mm);
  while ((((hh % 10) != 7) && ((mm % 10) != 7)))
  {
    mm -= x;
    if ((mm < 0))
    {
      mm = (mm + 60);
      hh = (hh - 1);
    }
    if ((hh < 0))
    {
      hh = 23;
    }
    ans += 1;
  }
  write(ans);
  return 0;
}
